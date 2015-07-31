/**
 * @file tree.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Manipulation with libyang data structures
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

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#include "common.h"
#include "context.h"
#include "parser.h"
#include "xml.h"
#include "tree_internal.h"

void ly_submodule_free(struct ly_submodule *submodule);

struct ly_mnode_leaf *
find_leaf(struct ly_mnode *parent, const char *name, int len)
{
    struct ly_mnode *child;
    struct ly_mnode_leaf *result;

    if (!len) {
        len = strlen(name);
    }

    LY_TREE_FOR(parent->child, child) {
        switch (child->nodetype) {
        case LY_NODE_LEAF:
            /* direct check */
            if (child->name == name || (!strncmp(child->name, name, len) && !child->name[len])) {
                return (struct ly_mnode_leaf *)child;
            }
            break;
        case LY_NODE_USES:
            /* search recursively */
            result = find_leaf(child, name, len);
            if (result) {
                return result;
            }
            break;
        default:
            /* ignore */
            break;
        }
    }

    return NULL;
}

void
ly_mnode_unlink(struct ly_mnode *node)
{
    struct ly_mnode *parent, *first;

    if (!node) {
        return;
    }

    /* unlink from data model if necessary */
    if (node->module) {
        if (node->module->data == node) {
            node->module->data = node->next;
        } else if (node->module->rpc == node) {
            node->module->rpc = node->next;
        } else if (node->module->notif == node) {
            node->module->notif = node->next;
        }
    }

    /* store pointers to important nodes */
    parent = node->parent;
    if (parent && !parent->nodetype) {
        /* handle augments - first, unlink it from the augment parent ... */
        if (parent->child == node) {
            parent->child = node->next;
        }
        /* and then continue with the target parent */
        parent = ((struct ly_augment *)parent)->target;
    }

    /* unlink from parent */
    if (parent) {
        if (parent->child == node) {
            parent->child = node->next;
        }
        node->parent = NULL;
    }

    /* unlink from siblings */
    if (node->prev == node) {
        /* there are no more siblings */
        return;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        /* unlinking the last element */
        if (parent) {
            first = parent->child;
        } else {
            first = node;
            while (node->prev->next) {
                first = node->prev;
            }
        }
        first->prev = node->prev;
    }
    if (node->prev->next) {
        node->prev->next = node->next;
    }

    /* clean up the unlinked element */
    node->next = NULL;
    node->prev = node;
}

/*
 * Add child model node at the end of the parent's child list.
 * If the child is connected somewhere (has a parent), it is completely
 * unlinked and none of the following conditions applies.
 * If the child has prev sibling(s), they are ignored (child is added at the
 * end of the child list).
 * If the child has next sibling(s), all of them are connected with the parent.
 */
int
ly_mnode_addchild(struct ly_mnode *parent, struct ly_mnode *child)
{
    struct ly_mnode *last;

    assert(parent);
    assert(child);

    /* checks */
    switch (parent->nodetype) {
    case LY_NODE_CONTAINER:
    case LY_NODE_LIST:
    case LY_NODE_GROUPING:
    case LY_NODE_USES:
    case LY_NODE_INPUT:
    case LY_NODE_OUTPUT:
    case LY_NODE_NOTIF:
        if (!(child->nodetype &
                (LY_NODE_ANYXML | LY_NODE_CHOICE | LY_NODE_CONTAINER | LY_NODE_GROUPING | LY_NODE_LEAF |
                 LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_USES))) {
            LOGVAL(VE_SPEC, 0, "Unexpected substatement \"%s\" in \"%s\" (%s).",
                   strnodetype(child->nodetype), strnodetype(parent->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LY_NODE_CHOICE:
        if (!(child->nodetype &
                (LY_NODE_ANYXML | LY_NODE_CASE | LY_NODE_CONTAINER | LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST))) {
            LOGVAL(VE_SPEC, 0, "Unexpected substatement \"%s\" in \"choice\" %s.",
                   strnodetype(child->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LY_NODE_CASE:
        if (!(child->nodetype &
                (LY_NODE_ANYXML | LY_NODE_CHOICE | LY_NODE_CONTAINER | LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_LIST | LY_NODE_USES))) {
            LOGVAL(VE_SPEC, 0, "Unexpected substatement \"%s\" in \"case\" %s.",
                   strnodetype(child->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LY_NODE_RPC:
        if (!(child->nodetype & (LY_NODE_INPUT | LY_NODE_OUTPUT | LY_NODE_GROUPING))) {
            LOGVAL(VE_SPEC, 0, "Unexpected substatement \"%s\" in \"rpc\" %s.",
                   strnodetype(child->nodetype), parent->name);
            return EXIT_FAILURE;
        }
        break;
    case LY_NODE_LEAF:
    case LY_NODE_LEAFLIST:
    case LY_NODE_ANYXML:
        LOGVAL(VE_SPEC, 0, "The \"%s\" statement (%s) cannot have any substatement.",
               strnodetype(parent->nodetype), parent->name);
        return EXIT_FAILURE;
    case LY_NODE_AUGMENT:
        LOGVAL(VE_SPEC, 0, "Internal error (%s:%d)", __FILE__, __LINE__);
        return EXIT_FAILURE;
    }

    if (child->parent) {
        ly_mnode_unlink(child);
    }

    if (!parent->child) {
        /* the only/first child of the parent */
        parent->child = child;
        child->parent = parent;
        last = child;
    } else {
        /* add a new child at the end of parent's child list */
        last = parent->child->prev;
        last->next = child;
        child->prev = last;
    }
    while (last->next) {
        last = last->next;
        last->parent = parent;
    }
    parent->child->prev = last;

    return EXIT_SUCCESS;
}

static struct ly_module *
find_import_in_includes_recursive(struct ly_module *mod, const char *prefix, uint32_t pref_len)
{
    int i, j;
    struct ly_submodule *sub_mod;
    struct ly_module *ret;

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
        ret = find_import_in_includes_recursive((struct ly_module *)mod->inc[i].submodule, prefix, pref_len);
        if (ret) {
            return ret;
        }
    }

    return NULL;
}

static struct ly_module *
find_prefixed_module(struct ly_module *mod, const char *prefix, uint32_t pref_len)
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
    return find_import_in_includes_recursive(mod, prefix, pref_len);
}

/*
 * id - schema-nodeid
 *
 * node_type - LY_NODE_AUGMENT (searches also RPCs and notifications)
 *           - LY_NODE_USES    (only descendant-schema-nodeid allowed, ".." not allowed)
 *           - LY_NODE_CHOICE  (search only start->child, only descendant-schema-nodeid allowed)
 */
struct ly_mnode *
resolve_schema_nodeid(const char *id, struct ly_mnode *start, struct ly_module *mod, LY_NODE_TYPE node_type)
{
    const char *name, *prefix, *ptr;
    struct ly_mnode *sibling;
    uint32_t nam_len, pref_len;
    struct ly_module *prefix_mod, *start_mod;
    /* 0 - in module, 1 - in 1st submodule, 2 - in 2nd submodule, ... */
    uint8_t in_submod = 0;
    /* 0 - in data, 1 - in RPCs, 2 - in notifications (relevant only with LY_NODE_AUGMENT) */
    uint8_t in_mod_part = 0;

    assert(mod);
    assert(id);

    if (id[strlen(id)-1] == '/') {
        LOGERR(LY_EINVAL, "%s: Path ending with '/' is not valid.", __func__);
        return NULL;
    }

    if (id[0] == '/') {
        if (node_type & (LY_NODE_USES | LY_NODE_CHOICE)) {
            return NULL;
        }
        ptr = strchr(id+1, '/');
        prefix = id+1;
    } else {
        ptr = strchr(id, '/');
        prefix = id;
    }
    pref_len = (ptr ? (unsigned)(ptr-prefix) : strlen(prefix));

    ptr = strnchr(prefix, ':', pref_len);
    /* there is a prefix */
    if (ptr) {
        nam_len = (pref_len-(ptr-prefix))-1;
        pref_len = ptr-prefix;
        name = ptr+1;

    /* no prefix used */
    } else {
        name = prefix;
        nam_len = pref_len;
        prefix = NULL;
    }

    /* absolute-schema-nodeid */
    if (id[0] == '/') {
        if (prefix) {
            start_mod = find_prefixed_module(mod, prefix, pref_len);
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
        if (!strcmp(name, ".")) {
            /* this node - start does not change */
        } else if (!strcmp(name, "..")) {
            /* ".." is not allowed in refines and augments in uses, there is no need for it there */
            if (!start || (node_type == LY_NODE_USES)) {
                return NULL;
            }
            start = start->parent;
        } else {
            sibling = NULL;
            LY_TREE_FOR(start, sibling) {
                /* name match */
                if ((sibling->name && !strncmp(name, sibling->name, nam_len) && (strlen(sibling->name) == nam_len))
                        || (!strncmp(name, "input", 5) && (nam_len == 5) && (sibling->nodetype == LY_NODE_INPUT))
                        || (!strncmp(name, "output", 6) && (nam_len == 6) && (sibling->nodetype == LY_NODE_OUTPUT))) {

                    /* prefix match check */
                    if (prefix) {

                        prefix_mod = find_prefixed_module(mod, prefix, pref_len);
                        if (!prefix_mod) {
                            return NULL;
                        }

                        if (!sibling->module->type) {
                            if (prefix_mod != sibling->module) {
                                continue;
                            }
                        } else {
                            if (prefix_mod != ((struct ly_submodule *)sibling->module)->belongsto) {
                                continue;
                            }
                        }
                    }

                    /* the result node? */
                    ptr = name+nam_len;
                    if (!ptr[0]) {
                        return sibling;
                    }
                    assert(ptr[0] == '/');

                    /* check for shorthand cases - then 'start' does not change */
                    if (!sibling->parent || (sibling->parent->nodetype != LY_NODE_CHOICE)
                            || (sibling->nodetype == LY_NODE_CASE)) {
                        start = sibling->child;
                    }
                    break;
                }
            }

            /* we did not find the case in direct siblings */
            if (node_type == LY_NODE_CHOICE) {
                return NULL;
            }

            /* no match */
            if (!sibling) {
                /* on augment search also RPCs and notifications, if we are in top-level */
                if ((node_type == LY_NODE_AUGMENT) && (!start || !start->parent)) {
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
        }

        /* we found our submodule */
        if (in_submod) {
            start_mod = (struct ly_module *)start_mod->inc[in_submod-1].submodule;
            in_submod = 0;
        }

        assert((*(name+nam_len) == '/') || (*(name+nam_len) == '\0'));

        /* make prefix point to the next node name */
        prefix = name+nam_len;
        ++prefix;
        assert(prefix[0]);

        /* parse prefix and node name */
        ptr = strchr(prefix, '/');
        pref_len = (ptr ? (unsigned)(ptr-prefix) : strlen(prefix));
        ptr = strnchr(prefix, ':', pref_len);

        /* there is prefix */
        if (ptr) {
            nam_len = (pref_len-(ptr-prefix))-1;
            pref_len = ptr-prefix;
            name = ptr+1;

        /* no prefix used */
        } else {
            name = prefix;
            nam_len = pref_len;
            prefix = NULL;
        }
    }

    /* cannot get here */
    return NULL;
}

static int
is_identifier(char start, char c)
{
    if (start == c) {
        /* first letter */
        if (isalpha(start) || c == '_') {
            return 1;
        }
    } else {
        /* checking inner letter */
        if (isalnum(c) || c == '_' || c == '-' || c == '.') {
            return 1;
        }
    }
    return 0;
}

static int
match_data_nodeid(const char *id, struct lyd_node *data, struct lyd_node *parent, struct lyd_node **result)
{
    int i = 0, j;
    const char *name;
    struct ly_module *mod;
    struct lyd_node *node, *start;

    *result = NULL;

    /* [prefix:]identifier */
    name = id;
    while(is_identifier(name[0], name[i])) {
        i++;
    }
    if (name[i] == ':') {
        /* we have prefix, find appropriate module */
        if (!i) {
            /* syntax error */
            return 0;
        }

        mod = find_prefixed_module(data->schema->module, name, i);
        if (!mod) {
            /* invalid prefix */
            return 0;
        }

        /* now get the identifier */
        name = &id[i + 1];
        j = 0;
        while(is_identifier(name[0], name[j])) {
            j++;
        }
        if (!j) {
            i = 0;
        }
    } else {
        /* no prefix, module is the same as of current node */
        mod = data->schema->module;
        j = i;
    }
    if (!i) {
        /* syntax error */
        return 0;
    }

    if (parent) {
        start = parent->child;
    } else {
        /* get data root */
        for (start = data; start->parent; start = start->parent);
    }
    LY_TREE_FOR(start, node) {
        if (node->schema->module == mod && !strncmp(node->schema->name, name, j)) {
            /* matching target */
            *result = node;
            break;
        }
    }

    if (!*result) {
        return 0;
    }

    return i;
}

/* return number of processed bytes in id */
static int
resolve_data_nodeid(const char *id, struct lyd_node *data, struct leafref_instid **parents)
{
    int i = 0, j, flag;
    struct ly_module *mod;
    const char *name;
    struct leafref_instid *item, *par_iter;
    struct lyd_node *node, *root = NULL;

    /* [prefix:]identifier */
    name = id;
    while(is_identifier(name[0], name[i])) {
        i++;
    }
    if (name[i] == ':') {
        /* we have prefix, find appropriate module */
        if (!i) {
            /* syntax error */
            return 0;
        }

        mod = find_prefixed_module(data->schema->module, name, i);
        if (!mod) {
            /* invalid prefix */
            return 0;
        }

        /* now get the identifier */
        name = &id[++i];
        j = 0;
        while(is_identifier(name[0], name[j])) {
            j++;
        }
        if (!j) {
            i = 0;
        } else {
            i += j;
        }
    } else {
        /* no prefix, module is the same as of current node */
        mod = data->schema->module;
        j = i;
    }
    if (!i) {
        /* syntax error */
        return 0;
    }

    if (!*parents) {
        *parents = malloc(sizeof **parents);
        (*parents)->dnode = NULL;
        (*parents)->next = NULL;

        /* find root data element */
        for (root = data; root->parent; root = root->parent);
        /* make sure it's first */
        for (; root->prev->next; root = root->prev);
    }
    for (par_iter = *parents; par_iter; par_iter = par_iter->next) {
        if (par_iter->dnode && (par_iter->dnode->schema->nodetype & (LY_NODE_LEAF | LY_NODE_LEAFLIST))) {
            /* skip */
            continue;
        }
        flag = 0;
        LY_TREE_FOR(par_iter->dnode ? par_iter->dnode->child : root, node) {
            if (node->schema->module == mod && !strncmp(node->schema->name, name, j)) {
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

    return i;
}

int
resolve_path(struct lyd_node *data, const char *path, struct leafref_instid **ret)
{
    struct leafref_instid *riter = NULL, *raux;
    struct ly_mnode_leaf *schema = (struct ly_mnode_leaf *)data->schema;
    struct lyd_node *pathnode = NULL;
    int i, j;
    char *p = strdup(path);
    char *name;
    struct lyd_node *pred_source, *pred_target;
    *ret = NULL;

    i = 0;
    if (p[0] == '/') {
        /* absolute path, start with '/' */
        i = 0;

    } else {
        *ret = calloc(1, sizeof **ret);
        while(!strncmp(&p[i], "../", 3)) {
            /* relative path */
            i += 3;
            if (!*ret) {
                /* error, too many .. */
                LOGVAL(DE_INVAL, 0, p, schema->name);
                goto error;
            } else if (!(*ret)->dnode) {
                /* first .. */
                (*ret)->dnode = data->parent;
            } else if (!(*ret)->dnode->parent) {
                /* we are in root */
                free(*ret);
                *ret = NULL;
            } else {
                /* multiple .. */
                (*ret)->dnode = (*ret)->dnode->parent;
            }
        }
        if (!i) {
            /* neither absolute or relative p */
            LOGVAL(DE_INVAL, 0, p, schema->name);
            goto error;
        }
        /* start with '/' */
        i--;
    }

    /* searching for nodeset */
    for (; p[i]; ) {
        if (p[i] == '/') {
            i++;

            /* node identifier */
            j = resolve_data_nodeid(&p[i], data, ret);
            if (!j || !*ret) {
                goto error;
            }

            i += j;
            if (p[i] == '[') {
                /* we have predicate, so the current results must be lists */
                for (raux = NULL, riter = *ret; riter; ) {
                    if (riter->dnode->schema->nodetype == LY_NODE_LIST &&
                            ((struct ly_mnode_list *)riter->dnode->schema)->keys) {
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
                if (!*ret) {
                    /* no matching node */
                }
            }
        } else if (p[i] == '[') {
            /* predicate */
            i++;
            while(isspace(p[i])) {
                i++;
            }

            /* [prefix:]identifier */
            name = &p[i]; /* use name later */
            while (isalnum(p[i]) || p[i] == '_' || p[i] == '-' || p[i] == '.' || p[i] == ':') {
                i++;
            }

            /* *WSP "=" *WSP */
            while (isspace(p[i])) {
                p[i] = '\0';
                i++;
            }
            if (p[i] != '=') {
                /* error */
            }
            p[i] = '\0';
            i++;
            while (isspace(p[i])) {
                i++;
            }

            /* path-key-expr (pred_source) */
            /* current-function-invocation */
            if (strncmp(&p[i], "current()", 9)) {
                /* error */
            }
            pred_source = data;
            i += 9;
            /* *WSP */
            while (isspace(p[i])) {
                i++;
            }
            /* rel-path-keyexpr */
            while(p[i] != ']') {
                /* "/" *WSP */
                if (p[i] != '/') {
                    /* error */
                }
                i++;
                while (isspace(p[i])) {
                    i++;
                }

                if (!strncmp("..", &p[i], 2)) {
                    /* 1*(".." *WSP */
                    i += 2;
                    while (isspace(p[i])) {
                        i++;
                    }
                    pred_source = pred_source->parent;
                } else {
                    /* node-identifier *WSP */
                    j = match_data_nodeid(&p[i], data, pred_source, &pred_source);
                    if (!j) {
                        /* error */
                    }
                    i += j;
                    while (isspace(p[i])) {
                        i++;
                    }
                }
            }
            i++;

            if (pred_source == data) {
                /* something is wrong */
            }

            /* find match between target and source nodes */
            for (raux = NULL, riter = *ret; riter; ) {
                pathnode = riter->dnode;

                /* get target */
                if (!match_data_nodeid(name, data, pathnode, &pred_target)) {
                    /* error */
                }

                if (pred_target->schema->nodetype != pred_source->schema->nodetype) {
                    goto remove_leafref;
                }

                if (((struct ly_mnode_leaf *)pred_target->schema)->type.base != ((struct ly_mnode_leaf *)pred_source->schema)->type.base) {
                    goto remove_leafref;
                }

                if (((struct lyd_node_leaf *)pred_target)->value_str != ((struct lyd_node_leaf *)pred_source)->value_str) {
                    goto remove_leafref;
                }

                /* leafref is ok, continue check with next leafref */
                raux = riter;
                riter = riter->next;
                continue;

remove_leafref:

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
        } else {
            /* syntax error */
            goto error;
        }
    }

    free(p);
    return 0;

error:

    free(p);
    while (*ret) {
        raux = (*ret)->next;
        free(*ret);
        *ret = raux;
    }

    return 1;
}

int
resolve_instid(struct lyd_node *data, const char *path, int path_len, struct leafref_instid **ret)
{
    struct leafref_instid *riter = NULL, *raux;
    struct lyd_node *pathnode = NULL;
    int i, j, cur_idx, idx = -1;
    char *p = strndup(path, path_len);
    char *name, *value = NULL;
    struct lyd_node *pred_target;

    i = 0;
    if (p[0] != '/') {
        /* error */
    }

    /* searching for nodeset */
    while (p[i]) {
        if (p[i] == '/') {
            i++;

            /* node identifier */
            j = resolve_data_nodeid(&p[i], data, ret);
            if (!j || !*ret) {
                goto error;
            }

            i += j;
            if (p[i] == '[') {
                /* we have predicate, so the current results must be list or leaf-list */
                for (raux = NULL, riter = *ret; riter; ) {
                    if ((riter->dnode->schema->nodetype == LY_NODE_LIST &&
                            ((struct ly_mnode_list *)riter->dnode->schema)->keys)
                            || (riter->dnode->schema->nodetype == LY_NODE_LEAFLIST)) {
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
                if (!*ret) {
                    /* no matching node */
                }
            }
        } else if (p[i] == '[') {
            /* predicate */
            i++;
            while(isspace(p[i])) {
                i++;
            }

            /* [prefix:]identifier */
            name = &p[i]; /* use name later */
            while (isalnum(p[i]) || p[i] == '_' || p[i] == '-' || p[i] == '.' || p[i] == ':') {
                i++;
            }

            /* *WSP */
            while (isspace(p[i])) {
                p[i] = '\0';
                i++;
            }

            /* name is position */
            if (isdigit(name[0])) {
                if (p[i] != ']') {
                    /* error */
                }
                p[i] = '\0';
                ++i;
                idx = atoi(name);
                goto resolve_predicate;
            }

            /* "=" *WSP */
            if (p[i] != '=') {
                /* error */
            }
            p[i] = '\0';
            i++;
            while (isspace(p[i])) {
                i++;
            }

            /* (DQUOTE/SQUOTE) string (DQUOTE/SQUOTE) */
            if ((p[i] != '\"') && (p[i] != '\'')) {
                /* error */
            }

            value = &p[i+1];
            if (!value[0]) {
                /* error */
            }

            j = i;
            i++;

            while (p[j] != p[i]) {
                if (p[i] == '\0') {
                    /* error */
                }
                i++;
            }

            p[i] = '\0';
            i++;

            /* *WSP */
            while (isspace(p[i])) {
                i++;
            }

            if (p[i] != ']') {
                /* error */
            }
            i++;

resolve_predicate:

            /* find match between target and source nodes */
            for (cur_idx = 0, raux = NULL, riter = *ret; riter; ++cur_idx) {
                pathnode = riter->dnode;

                /* get target */
                if (!strcmp(name, ".") || !value) {
                    pred_target = pathnode;
                } else if (!match_data_nodeid(name, data, pathnode, &pred_target)) {
                    /* error */
                }

                /* check that we have the correct type */
                if (!strcmp(name, ".")) {
                    if (pathnode->schema->nodetype != LY_NODE_LEAFLIST) {
                        goto remove_leafref;
                    }
                } else if (value) {
                    if (pathnode->schema->nodetype != LY_NODE_LIST) {
                        goto remove_leafref;
                    }
                }

                if ((value && strcmp(((struct lyd_node_leaf *)pred_target)->value_str, value)) || (!value && (idx != cur_idx))) {
                    goto remove_leafref;
                }

                /* leafref is ok, continue check with next leafref */
                raux = riter;
                riter = riter->next;
                continue;

remove_leafref:

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
        } else {
            /* syntax error */
            goto error;
        }
    }

    free(p);
    return 0;

error:

    free(p);
    while (*ret) {
        raux = (*ret)->next;
        free(*ret);
        *ret = raux;
    }

    return 1;
}

API struct ly_module *
lys_parse(struct ly_ctx *ctx, const char *data, LY_MINFORMAT format)
{
    if (!ctx || !data) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    switch (format) {
    case LY_IN_YIN:
        return yin_read_module(ctx, data, 1);
    case LY_IN_YANG:
    default:
        /* TODO */
        return NULL;
    }

    return NULL;
}

struct ly_submodule *
ly_submodule_read(struct ly_module *module, const char *data, LY_MINFORMAT format, int implement)
{
    assert(module);
    assert(data);

    switch (format) {
    case LY_IN_YIN:
        return yin_read_submodule(module, data, implement);
    case LY_IN_YANG:
    default:
        /* TODO */
        return NULL;
    }

    return NULL;
}

struct ly_module *
lys_read_import(struct ly_ctx *ctx, int fd, LY_MINFORMAT format)
{
    struct ly_module *module;
    struct stat sb;
    char *addr;

    if (!ctx || fd < 0) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    /*
     * TODO
     * This is just a temporary solution to make working automatic search for
     * imported modules. This doesn't work e.g. for streams (stdin)
     */
    fstat(fd, &sb);
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    switch (format) {
    case LY_IN_YIN:
        module = yin_read_module(ctx, addr, 0);
        break;
    case LY_IN_YANG:
    default:
        /* TODO */
        munmap(addr, sb.st_size);
        return NULL;
    }
    munmap(addr, sb.st_size);

    return module;
}

API struct ly_module *
lys_read(struct ly_ctx *ctx, int fd, LY_MINFORMAT format)
{
    struct ly_module *module;
    struct stat sb;
    char *addr;

    if (!ctx || fd < 0) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    /*
     * TODO
     * This is just a temporary solution to make working automatic search for
     * imported modules. This doesn't work e.g. for streams (stdin)
     */
    fstat(fd, &sb);
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    module = lys_parse(ctx, addr, format);
    munmap(addr, sb.st_size);

    return module;
}

struct ly_submodule *
ly_submodule_read_fd(struct ly_module *module, int fd, LY_MINFORMAT format, int implement)
{
    struct ly_submodule *submodule;
    struct stat sb;
    char *addr;

    assert(module);
    assert(fd >= 0);

    /*
     * TODO
     * This is just a temporary solution to make working automatic search for
     * imported modules. This doesn't work e.g. for streams (stdin)
     */
    fstat(fd, &sb);
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    /* TODO addr error check */
    submodule = ly_submodule_read(module, addr, format, implement);
    munmap(addr, sb.st_size);

    return submodule;

}

struct ly_restr *
ly_restr_dup(struct ly_ctx *ctx, struct ly_restr *old, int size)
{
    struct ly_restr *result;
    int i;

    if (!size) {
        return NULL;
    }

    result = calloc(size, sizeof *result);
    for (i = 0; i < size; i++) {
        result[i].expr = lydict_insert(ctx, old[i].expr, 0);
        result[i].dsc = lydict_insert(ctx, old[i].dsc, 0);
        result[i].ref = lydict_insert(ctx, old[i].ref, 0);
        result[i].eapptag = lydict_insert(ctx, old[i].eapptag, 0);
        result[i].emsg = lydict_insert(ctx, old[i].emsg, 0);
    }

    return result;
}

void
ly_restr_free(struct ly_ctx *ctx, struct ly_restr *restr)
{
    assert(ctx);
    if (!restr) {
        return;
    }

    lydict_remove(ctx, restr->expr);
    lydict_remove(ctx, restr->dsc);
    lydict_remove(ctx, restr->ref);
    lydict_remove(ctx, restr->eapptag);
    lydict_remove(ctx, restr->emsg);
}

void
ly_type_dup(struct ly_ctx *ctx, struct ly_type *new, struct ly_type *old)
{
    int i;

    new->prefix = lydict_insert(ctx, old->prefix, 0);
    new->base = old->base;
    new->der = old->der;

    switch (new->base) {
    case LY_TYPE_BINARY:
        if (old->info.binary.length) {
            new->info.binary.length = ly_restr_dup(ctx, old->info.binary.length, 1);
        }
        break;

    case LY_TYPE_BITS:
        new->info.bits.count = old->info.bits.count;
        if (new->info.bits.count) {
            new->info.bits.bit = calloc(new->info.bits.count, sizeof *new->info.bits.bit);
            for (i = 0; i < new->info.bits.count; i++) {
                new->info.bits.bit[i].name = lydict_insert(ctx, old->info.bits.bit[i].name, 0);
                new->info.bits.bit[i].dsc = lydict_insert(ctx, old->info.bits.bit[i].dsc, 0);
                new->info.bits.bit[i].ref = lydict_insert(ctx, old->info.bits.bit[i].ref, 0);
                new->info.bits.bit[i].status = old->info.bits.bit[i].status;
                new->info.bits.bit[i].pos = old->info.bits.bit[i].pos;
            }
        }
        break;

    case LY_TYPE_DEC64:
        new->info.dec64.dig = old->info.dec64.dig;
        if (old->info.dec64.range) {
            new->info.dec64.range = ly_restr_dup(ctx, old->info.dec64.range, 1);
        }
        break;

    case LY_TYPE_ENUM:
        new->info.enums.count = old->info.enums.count;
        if (new->info.enums.count) {
            new->info.enums.list = calloc(new->info.enums.count, sizeof *new->info.enums.list);
            for (i = 0; i < new->info.enums.count; i++) {
                new->info.enums.list[i].name = lydict_insert(ctx, old->info.enums.list[i].name, 0);
                new->info.enums.list[i].dsc = lydict_insert(ctx, old->info.enums.list[i].dsc, 0);
                new->info.enums.list[i].ref = lydict_insert(ctx, old->info.enums.list[i].ref, 0);
                new->info.enums.list[i].status = old->info.enums.list[i].status;
                new->info.enums.list[i].value = old->info.enums.list[i].value;
            }
        }
        break;

    case LY_TYPE_IDENT:
        new->info.ident.ref = old->info.ident.ref;
        break;

    case LY_TYPE_INST:
        new->info.inst.req = old->info.inst.req;
        break;

    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        if (old->info.num.range) {
            new->info.num.range = ly_restr_dup(ctx, old->info.num.range, 1);
        }
        break;

    case LY_TYPE_LEAFREF:
        new->info.lref.path = lydict_insert(ctx, old->info.lref.path, 0);
        break;

    case LY_TYPE_STRING:
        if (old->info.str.length) {
            new->info.str.length = ly_restr_dup(ctx, old->info.str.length, 1);
        }
        new->info.str.patterns = ly_restr_dup(ctx, old->info.str.patterns, old->info.str.pat_count);
        break;

    case LY_TYPE_UNION:
        new->info.uni.count = old->info.uni.count;
        if (new->info.uni.count) {
            new->info.uni.type = calloc(new->info.uni.count, sizeof *new->info.uni.type);
            for (i = 0; i < new->info.uni.count; i++) {
                ly_type_dup(ctx, &(new->info.uni.type[i]), &(old->info.uni.type[i]));
            }
        }
        break;

    default:
        /* nothing to do for LY_TYPE_BOOL, LY_TYPE_EMPTY */
        break;
    }
}

void
ly_type_free(struct ly_ctx *ctx, struct ly_type *type)
{
    int i;

    assert(ctx);
    if (!type) {
        return;
    }

    lydict_remove(ctx, type->prefix);

    switch (type->base) {
    case LY_TYPE_BINARY:
        ly_restr_free(ctx, type->info.binary.length);
        free(type->info.binary.length);
        break;
    case LY_TYPE_BITS:
        for (i = 0; i < type->info.bits.count; i++) {
            lydict_remove(ctx, type->info.bits.bit[i].name);
            lydict_remove(ctx, type->info.bits.bit[i].dsc);
            lydict_remove(ctx, type->info.bits.bit[i].ref);
        }
        free(type->info.bits.bit);
        break;

    case LY_TYPE_DEC64:
        ly_restr_free(ctx, type->info.dec64.range);
        free(type->info.dec64.range);
        break;

    case LY_TYPE_ENUM:
        for (i = 0; i < type->info.enums.count; i++) {
            lydict_remove(ctx, type->info.enums.list[i].name);
            lydict_remove(ctx, type->info.enums.list[i].dsc);
            lydict_remove(ctx, type->info.enums.list[i].ref);
        }
        free(type->info.enums.list);
        break;

    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        ly_restr_free(ctx, type->info.num.range);
        free(type->info.num.range);
        break;

    case LY_TYPE_LEAFREF:
        lydict_remove(ctx, type->info.lref.path);
        break;

    case LY_TYPE_STRING:
        ly_restr_free(ctx, type->info.str.length);
        free(type->info.str.length);
        for (i = 0; i < type->info.str.pat_count; i++) {
            ly_restr_free(ctx, &type->info.str.patterns[i]);
        }
        free(type->info.str.patterns);
        break;

    case LY_TYPE_UNION:
        for (i = 0; i < type->info.uni.count; i++) {
            ly_type_free(ctx, &type->info.uni.type[i]);
        }
        free(type->info.uni.type);
        break;

    default:
        /* nothing to do for LY_TYPE_IDENT, LY_TYPE_INST, LY_TYPE_BOOL, LY_TYPE_EMPTY */
        break;
    }
}

struct ly_tpdf *
ly_tpdf_dup(struct ly_ctx *ctx, struct ly_tpdf *old, int size)
{
    struct ly_tpdf *result;
    int i;

    if (!size) {
        return NULL;
    }

    result = calloc(size, sizeof *result);
    for (i = 0; i < size; i++) {
        result[i].name = lydict_insert(ctx, old[i].name, 0);
        result[i].dsc = lydict_insert(ctx, old[i].dsc, 0);
        result[i].ref = lydict_insert(ctx, old[i].ref, 0);
        result[i].flags = old[i].flags;
        result[i].module = old[i].module;

        ly_type_dup(ctx, &(result[i].type), &(old[i].type));

        result[i].dflt = lydict_insert(ctx, old[i].dflt, 0);
        result[i].units = lydict_insert(ctx, old[i].units, 0);
    }

    return result;
}

void
ly_tpdf_free(struct ly_ctx *ctx, struct ly_tpdf *tpdf)
{
    assert(ctx);
    if (!tpdf) {
        return;
    }

    lydict_remove(ctx, tpdf->name);
    lydict_remove(ctx, tpdf->dsc);
    lydict_remove(ctx, tpdf->ref);

    ly_type_free(ctx, &tpdf->type);

    lydict_remove(ctx, tpdf->units);
    lydict_remove(ctx, tpdf->dflt);
}

struct ly_when *
ly_when_dup(struct ly_ctx *ctx, struct ly_when *old)
{
    struct ly_when *new;

    if (!old) {
        return NULL;
    }

    new = calloc(1, sizeof *new);
    new->cond = lydict_insert(ctx, old->cond, 0);
    new->dsc = lydict_insert(ctx, old->dsc, 0);
    new->ref = lydict_insert(ctx, old->ref, 0);

    return new;
}

void
ly_when_free(struct ly_ctx *ctx, struct ly_when *w)
{
    if (!w) {
        return;
    }

    lydict_remove(ctx, w->cond);
    lydict_remove(ctx, w->dsc);
    lydict_remove(ctx, w->ref);

    free(w);
}

struct ly_augment *
ly_augment_dup(struct ly_module *module, struct ly_mnode *parent, struct ly_augment *old, int size)
{
    struct ly_augment *new = NULL;
    int i = -1;

    if (!size) {
        return NULL;
    }

    new = calloc(size, sizeof *new);
    for (i = 0; i < size; i++) {
        new[i].target_name = lydict_insert(module->ctx, old[i].target_name, 0);
        new[i].dsc = lydict_insert(module->ctx, old[i].dsc, 0);
        new[i].ref = lydict_insert(module->ctx, old[i].ref, 0);
        new[i].flags = old[i].flags;
        /* .target = NULL; .nodetype = 0 */

        new[i].parent = parent;

        /* copy the definition of augment nodes */
        if (old[i].child) {
            new[i].child = (struct ly_mnode *)lyxml_dup_elem(module->ctx, (struct lyxml_elem *)old[i].child, NULL, 1);
        }
    }

    return new;
}

struct ly_refine *
ly_refine_dup(struct ly_ctx *ctx, struct ly_refine *old, int size)
{
    struct ly_refine *result;
    int i;

    if (!size) {
        return NULL;
    }

    result = calloc(size, sizeof *result);
    for (i = 0; i < size; i++) {
        result[i].target = lydict_insert(ctx, old[i].target, 0);
        result[i].dsc = lydict_insert(ctx, old[i].dsc, 0);
        result[i].ref = lydict_insert(ctx, old[i].ref, 0);
        result[i].flags = old[i].flags;
        result[i].target_type = old[i].target_type;
        result[i].must_size = old[i].must_size;
        result[i].must = ly_restr_dup(ctx, old[i].must, old[i].must_size);
        if (result[i].target_type & (LY_NODE_LEAF | LY_NODE_CHOICE)) {
            result[i].mod.dflt = lydict_insert(ctx, old[i].mod.dflt, 0);
        } else if (result[i].target_type == LY_NODE_CONTAINER) {
            result[i].mod.presence = lydict_insert(ctx, old[i].mod.presence, 0);
        } else if (result[i].target_type & (LY_NODE_LIST | LY_NODE_LEAFLIST)) {
            result[i].mod.list = old[i].mod.list;
        }
    }

    return result;
}

void
ly_ident_free(struct ly_ctx *ctx, struct ly_ident *ident)
{
    struct ly_ident_der *der;

    assert(ctx);
    if (!ident) {
        return;
    }

    /*
     * if caller free only a single data model which is used (its identity is
     * reference from identity in another module), this silly freeing can lead
     * to segmentation fault. But without noting if the module is used by some
     * other, it cannot be solved.
     *
     * Possible solution is to not allow caller to remove particular schema
     * from the context.
     */
    while (ident->der) {
        der = ident->der;
        ident->der = der->next;
        free(der);
    }

    lydict_remove(ctx, ident->name);
    lydict_remove(ctx, ident->dsc);
    lydict_remove(ctx, ident->ref);

}

void
ly_grp_free(struct ly_ctx *ctx, struct ly_mnode_grp *grp)
{
    int i;

    /* handle only specific parts for LY_NODE_GROUPING */
    for (i = 0; i < grp->tpdf_size; i++) {
        ly_tpdf_free(ctx, &grp->tpdf[i]);
    }
    free(grp->tpdf);
}

void
ly_anyxml_free(struct ly_ctx *ctx, struct ly_mnode_anyxml *anyxml)
{
    int i;

    for (i = 0; i < anyxml->must_size; i++) {
        ly_restr_free(ctx, &anyxml->must[i]);
    }
    free(anyxml->must);

    ly_when_free(ctx, anyxml->when);
}

void
ly_leaf_free(struct ly_ctx *ctx, struct ly_mnode_leaf *leaf)
{
    int i;

    for (i = 0; i < leaf->must_size; i++) {
        ly_restr_free(ctx, &leaf->must[i]);
    }
    free(leaf->must);

    ly_when_free(ctx, leaf->when);

    ly_type_free(ctx, &leaf->type);
    lydict_remove(ctx, leaf->units);
    lydict_remove(ctx, leaf->dflt);
}

void
ly_leaflist_free(struct ly_ctx *ctx, struct ly_mnode_leaflist *llist)
{
    int i;

    for (i = 0; i < llist->must_size; i++) {
        ly_restr_free(ctx, &llist->must[i]);
    }
    free(llist->must);

    ly_when_free(ctx, llist->when);

    ly_type_free(ctx, &llist->type);
    lydict_remove(ctx, llist->units);
}

void
ly_list_free(struct ly_ctx *ctx, struct ly_mnode_list *list)
{
    int i;

    /* handle only specific parts for LY_NODE_LIST */
    for (i = 0; i < list->tpdf_size; i++) {
        ly_tpdf_free(ctx, &list->tpdf[i]);
    }
    free(list->tpdf);

    for (i = 0; i < list->must_size; i++) {
        ly_restr_free(ctx, &list->must[i]);
    }
    free(list->must);

    ly_when_free(ctx, list->when);

    for (i = 0; i < list->unique_size; i++) {
        free(list->unique[i].leafs);
    }
    free(list->unique);

    free(list->keys);
}

void
ly_container_free(struct ly_ctx *ctx, struct ly_mnode_container *cont)
{
    int i;

    /* handle only specific parts for LY_NODE_CONTAINER */
    lydict_remove(ctx, cont->presence);

    for (i = 0; i < cont->tpdf_size; i++) {
        ly_tpdf_free(ctx, &cont->tpdf[i]);
    }
    free(cont->tpdf);

    for (i = 0; i < cont->must_size; i++) {
        ly_restr_free(ctx, &cont->must[i]);
    }
    free(cont->must);

    ly_when_free(ctx, cont->when);
}

void
ly_feature_free(struct ly_ctx *ctx, struct ly_feature *f)
{
    lydict_remove(ctx, f->name);
    lydict_remove(ctx, f->dsc);
    lydict_remove(ctx, f->ref);
    free(f->features);
}

void
ly_deviation_free(struct ly_ctx *ctx, struct ly_deviation *dev)
{
    int i, j;

    lydict_remove(ctx, dev->target_name);
    lydict_remove(ctx, dev->dsc);
    lydict_remove(ctx, dev->ref);

    for (i = 0; i < dev->deviate_size; i++) {
        lydict_remove(ctx, dev->deviate[i].dflt);
        lydict_remove(ctx, dev->deviate[i].units);

        if (dev->deviate[i].mod == LY_DEVIATE_DEL) {
            for (j = 0; j < dev->deviate[i].must_size; j++) {
                ly_restr_free(ctx, &dev->deviate[i].must[j]);
            }
            free(dev->deviate[i].must);

            for (j = 0; j < dev->deviate[i].unique_size; j++) {
                free(dev->deviate[j].unique[j].leafs);
            }
            free(dev->deviate[i].unique);
        }
    }
    free(dev->deviate);
}

void
ly_augment_free(struct ly_ctx *ctx, struct ly_augment *aug)
{
    lydict_remove(ctx, aug->target_name);
    lydict_remove(ctx, aug->dsc);
    lydict_remove(ctx, aug->ref);

    free(aug->features);

    ly_when_free(ctx, aug->when);

    lyxml_free_elem(ctx, (struct lyxml_elem *)aug->child);
}

void
ly_uses_free(struct ly_ctx *ctx, struct ly_mnode_uses *uses)
{
    int i, j;

    for (i = 0; i < uses->refine_size; i++) {
        lydict_remove(ctx, uses->refine[i].target);
        lydict_remove(ctx, uses->refine[i].dsc);
        lydict_remove(ctx, uses->refine[i].ref);

        for (j = 0; j < uses->refine[j].must_size; j++) {
            ly_restr_free(ctx, &uses->refine[i].must[j]);
        }
        free(uses->refine[i].must);

        if (uses->refine[i].target_type & (LY_NODE_LEAF | LY_NODE_CHOICE)) {
            lydict_remove(ctx, uses->refine[i].mod.dflt);
        } else if (uses->refine[i].target_type & LY_NODE_CONTAINER) {
            lydict_remove(ctx, uses->refine[i].mod.presence);
        }
    }
    free(uses->refine);

    for (i = 0; i < uses->augment_size; i++) {
        ly_augment_free(ctx, &uses->augment[i]);
    }
    free(uses->augment);

    ly_when_free(ctx, uses->when);
}

void
ly_mnode_free(struct ly_mnode *node)
{
    struct ly_ctx *ctx;
    struct ly_mnode *sub, *next;

    if (!node) {
        return;
    }

    assert(node->module);
    assert(node->module->ctx);

    ctx = node->module->ctx;

    /* common part */
    LY_TREE_FOR_SAFE(node->child, next, sub) {
        ly_mnode_free(sub);
    }
    free(node->features);

    lydict_remove(ctx, node->name);
    lydict_remove(ctx, node->dsc);
    lydict_remove(ctx, node->ref);

    /* specific part */
    switch (node->nodetype) {
    case LY_NODE_CONTAINER:
        ly_container_free(ctx, (struct ly_mnode_container *)node);
        break;
    case LY_NODE_CHOICE:
        ly_when_free(ctx, ((struct ly_mnode_choice *)node)->when);
        break;
    case LY_NODE_LEAF:
        ly_leaf_free(ctx, (struct ly_mnode_leaf *)node);
        break;
    case LY_NODE_LEAFLIST:
        ly_leaflist_free(ctx, (struct ly_mnode_leaflist *)node);
        break;
    case LY_NODE_LIST:
        ly_list_free(ctx, (struct ly_mnode_list *)node);
        break;
    case LY_NODE_ANYXML:
        ly_anyxml_free(ctx, (struct ly_mnode_anyxml *)node);
        break;
    case LY_NODE_USES:
        ly_uses_free(ctx, (struct ly_mnode_uses *)node);
        break;
    case LY_NODE_CASE:
        ly_when_free(ctx, ((struct ly_mnode_case *)node)->when);
        break;
    case LY_NODE_AUGMENT:
        /* do nothing */
        break;
    case LY_NODE_GROUPING:
    case LY_NODE_RPC:
    case LY_NODE_INPUT:
    case LY_NODE_OUTPUT:
    case LY_NODE_NOTIF:
        ly_grp_free(ctx, (struct ly_mnode_grp *)node);
        break;
    }

    /* again common part */
    ly_mnode_unlink(node);
    free(node);
}

static void
module_free_common(struct ly_module *module)
{
    struct ly_ctx *ctx;
    unsigned int i;
    int j, l;

    assert(module->ctx);
    ctx = module->ctx;

    /* as first step, free the imported modules */
    for (i = 0; i < module->imp_size; i++) {
        /* get the imported module from the context and then free,
         * this check is necessary because the imported module can
         * be already removed
         */
        l = ctx->models.used;
        for (j = 0; j < l; j++) {
            if (ctx->models.list[j] == module->imp[i].module) {
                lys_free(module->imp[i].module);
                break;
            }
        }
    }
    free(module->imp);

    while (module->data) {
        ly_mnode_free(module->data);
    }
    while (module->rpc) {
        ly_mnode_free(module->rpc);
    }
    while (module->notif) {
        ly_mnode_free(module->notif);
    }

    lydict_remove(ctx, module->dsc);
    lydict_remove(ctx, module->ref);
    lydict_remove(ctx, module->org);
    lydict_remove(ctx, module->contact);

    /* revisions */
    for (i = 0; i < module->rev_size; i++) {
        lydict_remove(ctx, module->rev[i].dsc);
        lydict_remove(ctx, module->rev[i].ref);
    }
    free(module->rev);

    /* identities */
    for (i = 0; i < module->ident_size; i++) {
        ly_ident_free(ctx, &module->ident[i]);
    }
    module->ident_size = 0;
    free(module->ident);

    /* typedefs */
    for (i = 0; i < module->tpdf_size; i++) {
        ly_tpdf_free(ctx, &module->tpdf[i]);
    }
    free(module->tpdf);

    /* include */
    for (i = 0; i < module->inc_size; i++) {
        ly_submodule_free(module->inc[i].submodule);
    }
    free(module->inc);

    /* augment */
    for (i = 0; i < module->augment_size; i++) {
        ly_augment_free(ctx, &module->augment[i]);
    }
    free(module->augment);

    /* features */
    for (i = 0; i < module->features_size; i++) {
        ly_feature_free(ctx, &module->features[i]);
    }
    free(module->features);

    /* deviations */
    for (i = 0; i < module->deviation_size; i++) {
        ly_deviation_free(ctx, &module->deviation[i]);
    }
    free(module->deviation);

    lydict_remove(ctx, module->name);
}

void
ly_submodule_free(struct ly_submodule *submodule)
{
    if (!submodule) {
        return;
    }

    submodule->inc_size = 0;
    free(submodule->inc);
    submodule->inc = NULL;

    /* common part with struct ly_module */
    module_free_common((struct ly_module *)submodule);

    /* no specific items to free */

    free(submodule);
}

struct ly_mnode *
ly_mnode_dup(struct ly_module *module, struct ly_mnode *mnode, uint8_t flags, int recursive, unsigned int line)
{
    struct ly_mnode *retval = NULL, *aux, *child;
    struct ly_ctx *ctx = module->ctx;
    int i, j;

    struct ly_mnode_container *cont;
    struct ly_mnode_container *cont_orig = (struct ly_mnode_container *)mnode;
    struct ly_mnode_choice *choice;
    struct ly_mnode_choice *choice_orig = (struct ly_mnode_choice *)mnode;
    struct ly_mnode_leaf *leaf;
    struct ly_mnode_leaf *leaf_orig = (struct ly_mnode_leaf *)mnode;
    struct ly_mnode_leaflist *llist;
    struct ly_mnode_leaflist *llist_orig = (struct ly_mnode_leaflist *)mnode;
    struct ly_mnode_list *list;
    struct ly_mnode_list *list_orig = (struct ly_mnode_list *)mnode;
    struct ly_mnode_anyxml *anyxml;
    struct ly_mnode_anyxml *anyxml_orig = (struct ly_mnode_anyxml *)mnode;
    struct ly_mnode_uses *uses;
    struct ly_mnode_uses *uses_orig = (struct ly_mnode_uses *)mnode;
    struct ly_mnode_grp *mix;
    struct ly_mnode_grp *mix_orig = (struct ly_mnode_grp *)mnode;
    struct ly_mnode_case *cs;
    struct ly_mnode_case *cs_orig = (struct ly_mnode_case *)mnode;

    /* we cannot just duplicate memory since the strings are stored in
     * dictionary and we need to update dictionary counters.
     */

    switch (mnode->nodetype) {
    case LY_NODE_CONTAINER:
        cont = calloc(1, sizeof *cont);
        retval = (struct ly_mnode *)cont;
        break;

    case LY_NODE_CHOICE:
        choice = calloc(1, sizeof *choice);
        retval = (struct ly_mnode *)choice;
        break;

    case LY_NODE_LEAF:
        leaf = calloc(1, sizeof *leaf);
        retval = (struct ly_mnode *)leaf;
        break;

    case LY_NODE_LEAFLIST:
        llist = calloc(1, sizeof *llist);
        retval = (struct ly_mnode *)llist;
        break;

    case LY_NODE_LIST:
        list = calloc(1, sizeof *list);
        retval = (struct ly_mnode *)list;
        break;

    case LY_NODE_ANYXML:
        anyxml = calloc(1, sizeof *anyxml);
        retval = (struct ly_mnode *)anyxml;
        break;

    case LY_NODE_USES:
        uses = calloc(1, sizeof *uses);
        retval = (struct ly_mnode *)uses;
        break;

    case LY_NODE_CASE:
        cs = calloc(1, sizeof *cs);
        retval = (struct ly_mnode *)cs;
        break;

        /* exact same structure */
    case LY_NODE_GROUPING:
    case LY_NODE_RPC:
    case LY_NODE_INPUT:
    case LY_NODE_OUTPUT:
    case LY_NODE_NOTIF:
        mix = calloc(1, sizeof *mix);
        retval = (struct ly_mnode *)mix;
        break;

    default:
        goto error;
    }

    /*
     * duplicate generic part of the structure
     */
    retval->name = lydict_insert(ctx, mnode->name, 0);
    retval->dsc = lydict_insert(ctx, mnode->dsc, 0);
    retval->ref = lydict_insert(ctx, mnode->ref, 0);
    retval->flags = mnode->flags;
    if (!(retval->flags & LY_NODE_CONFIG_MASK)) {
        /* set parent's config flag */
        retval->flags |= flags & LY_NODE_CONFIG_MASK;
    }

    retval->module = module;
    retval->nodetype = mnode->nodetype;

    retval->prev = retval;

    retval->features_size = mnode->features_size;
    retval->features = calloc(retval->features_size, sizeof *retval->features);
    memcpy(retval->features, mnode->features, retval->features_size * sizeof *retval->features);

    if (recursive) {
        /* go recursively */
        LY_TREE_FOR(mnode->child, child) {
            aux = ly_mnode_dup(module, child, retval->flags, 1, line);
            if (!aux || ly_mnode_addchild(retval, aux)) {
                goto error;
            }
        }
    }

    /*
     * duplicate specific part of the structure
     */
    switch (mnode->nodetype) {
    case LY_NODE_CONTAINER:
        cont->when = ly_when_dup(ctx, cont_orig->when);
        cont->presence = lydict_insert(ctx, cont_orig->presence, 0);

        cont->must_size = cont_orig->must_size;
        cont->tpdf_size = cont_orig->tpdf_size;

        cont->must = ly_restr_dup(ctx, cont_orig->must, cont->must_size);
        cont->tpdf = ly_tpdf_dup(ctx, cont_orig->tpdf, cont->tpdf_size);
        break;

    case LY_NODE_CHOICE:
        choice->when = ly_when_dup(ctx, choice_orig->when);
        if (choice->dflt) {
            LY_TREE_FOR(choice->child, child) {
                if (child->name == choice_orig->dflt->name) {
                    choice->dflt = child;
                    break;
                }
            }
        }
        break;

    case LY_NODE_LEAF:
        ly_type_dup(ctx, &(leaf->type), &(leaf_orig->type));
        leaf->units = lydict_insert(ctx, leaf_orig->units, 0);
        leaf->dflt = lydict_insert(ctx, leaf_orig->dflt, 0);

        leaf->must_size = leaf_orig->must_size;
        leaf->must = ly_restr_dup(ctx, leaf_orig->must, leaf->must_size);

        leaf->when = ly_when_dup(ctx, leaf_orig->when);
        break;

    case LY_NODE_LEAFLIST:

        ly_type_dup(ctx, &(llist->type), &(llist_orig->type));
        llist->units = lydict_insert(ctx, llist_orig->units, 0);

        llist->min = llist_orig->min;
        llist->max = llist_orig->max;

        llist->must_size = llist_orig->must_size;
        llist->must = ly_restr_dup(ctx, llist_orig->must, llist->must_size);

        llist->when = ly_when_dup(ctx, llist_orig->when);
        break;

    case LY_NODE_LIST:
        list->min = list_orig->min;
        list->max = list_orig->max;

        list->must_size = list_orig->must_size;
        list->tpdf_size = list_orig->tpdf_size;
        list->keys_size = list_orig->keys_size;
        list->unique_size = list_orig->unique_size;

        list->must = ly_restr_dup(ctx, list_orig->must, list->must_size);
        list->tpdf = ly_tpdf_dup(ctx, list_orig->tpdf, list->tpdf_size);

        if (list->keys_size) {
            list->keys = calloc(list->keys_size, sizeof *list->keys);
            for (i = 0; i < list->keys_size; i++) {
                list->keys[i] = find_leaf(retval, list_orig->keys[i]->name, 0);
            }
        }
        if (list->unique_size) {
            list->unique = calloc(list->unique_size, sizeof *list->unique);
            for (i = 0; i < list->unique_size; i++) {
                list->unique[i].leafs = calloc(list->unique[i].leafs_size, sizeof *list->unique[i].leafs);
                for (j = 0; j < list->unique[i].leafs_size; j++) {
                    list->unique[i].leafs[j] = find_leaf(retval, list_orig->unique[i].leafs[j]->name, 0);
                }
            }
        }
        list->when = ly_when_dup(ctx, list_orig->when);
        break;

    case LY_NODE_ANYXML:
        anyxml->must_size = anyxml_orig->must_size;
        anyxml->must = ly_restr_dup(ctx, anyxml_orig->must, anyxml->must_size);
        anyxml->when = ly_when_dup(ctx, anyxml_orig->when);
        break;

    case LY_NODE_USES:
        uses->grp = uses_orig->grp;
        uses->when = ly_when_dup(ctx, uses_orig->when);
        uses->refine_size = uses_orig->refine_size;
        uses->refine = ly_refine_dup(ctx, uses_orig->refine, uses_orig->refine_size);
        uses->augment_size = uses_orig->augment_size;
        uses->augment = ly_augment_dup(module, (struct ly_mnode *)uses, uses_orig->augment, uses_orig->augment_size);
        if (resolve_uses(uses, line)) {
            goto error;
        }
        break;

    case LY_NODE_CASE:
        cs->when = ly_when_dup(ctx, cs_orig->when);
        break;

    case LY_NODE_GROUPING:
    case LY_NODE_RPC:
    case LY_NODE_INPUT:
    case LY_NODE_OUTPUT:
    case LY_NODE_NOTIF:
        mix->tpdf_size = mix_orig->tpdf_size;
        mix->tpdf = ly_tpdf_dup(ctx, mix_orig->tpdf, mix->tpdf_size);
        break;

    default:
        /* LY_NODE_AUGMENT */
        goto error;
    }

    return retval;

error:
    LOGDBG("error: %s:%d", __FILE__, __LINE__);

    ly_mnode_free(retval);
    return NULL;
}

API void
lys_free(struct ly_module *module)
{
    struct ly_ctx *ctx;
    int i;

    if (!module) {
        return;
    }

    /* remove schema from the context */
    ctx = module->ctx;
    if (ctx->models.used) {
        for (i = 0; i < ctx->models.used; i++) {
            if (ctx->models.list[i] == module) {
                /* replace the position in the list by the last module in the list */
                ctx->models.used--;
                ctx->models.list[i] = ctx->models.list[ctx->models.used];
                ctx->models.list[ctx->models.used] = NULL;
                /* we are done */
                break;
            }
        }
    }

    /* common part with struct ly_submodule */
    module_free_common(module);

    /* specific items to free */
    lydict_remove(module->ctx, module->ns);
    lydict_remove(module->ctx, module->prefix);

    free(module);
}

/*
 * op: 1 - enable, 0 - disable
 */
static int
ly_features_change(struct ly_module *module, const char *name, int op)
{
    int all = 0;
    int i, j, k;

    if (!module || !name || !strlen(name)) {
        return EXIT_FAILURE;
    }

    if (!strcmp(name, "*")) {
        /* enable all */
        all = 1;
    }

    /* module itself */
    for (i = 0; i < module->features_size; i++) {
        if (all || !strcmp(module->features[i].name, name)) {
            if (op) {
                module->features[i].flags |= LY_NODE_FENABLED;
                /* enable referenced features (recursion) */
                for (k = 0; k < module->features[i].features_size; k++) {
                    ly_features_change(module->features[i].features[k]->module,
                                       module->features[i].features[k]->name, op);
                }
            } else {
                module->features[i].flags &= ~LY_NODE_FENABLED;
            }
            if (!all) {
                return EXIT_SUCCESS;
            }
        }
    }

    /* submodules */
    for (j = 0; j < module->inc_size; j++) {
        for (i = 0; i < module->inc[j].submodule->features_size; i++) {
            if (all || !strcmp(module->inc[j].submodule->features[i].name, name)) {
                if (op) {
                    module->inc[j].submodule->features[i].flags |= LY_NODE_FENABLED;
                } else {
                    module->inc[j].submodule->features[i].flags &= ~LY_NODE_FENABLED;
                }
                if (!all) {
                    return EXIT_SUCCESS;
                }
            }
        }
    }

    /* TODO submodules of submodules ... */

    if (all) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

API int
lys_features_enable(struct ly_module *module, const char *feature)
{
    return ly_features_change(module, feature, 1);
}

API int
lys_features_disable(struct ly_module *module, const char *feature)
{
    return ly_features_change(module, feature, 0);
}

API int
lys_features_state(struct ly_module *module, const char *feature)
{
    int i, j;

    if (!module || !feature) {
        return -1;
    }

    /* search for the specified feature */
    /* module itself */
    for (i = 0; i < module->features_size; i++) {
        if (!strcmp(feature, module->features[i].name)) {
            if (module->features[i].flags & LY_NODE_FENABLED) {
                return 1;
            } else {
                return 0;
            }
        }
    }

    /* submodules */
    for (j = 0; j < module->inc_size; j++) {
        for (i = 0; i < module->inc[j].submodule->features_size; i++) {
            if (!strcmp(feature, module->inc[j].submodule->features[i].name)) {
                if (module->inc[j].submodule->features[i].flags & LY_NODE_FENABLED) {
                    return 1;
                } else {
                    return 0;
                }
            }
        }
    }

    /* TODO submodules of submodules ... */

    /* feature definition not found */
    return -1;
}

API const char **
lys_features_list(struct ly_module *module, uint8_t **states)
{
    const char **result = NULL;
    int i, j;
    unsigned int count;

    if (!module) {
        return NULL;
    }

    count = module->features_size;
    for (i = 0; i < module->inc_size; i++) {
        count += module->inc[i].submodule->features_size;
    }
    result = malloc((count + 1) * sizeof *result);
    if (states) {
        *states = malloc((count + 1) * sizeof **states);
    }
    count = 0;

    /* module itself */
    for (i = 0; i < module->features_size; i++) {
        result[count] = module->features[i].name;
        if (states) {
            if (module->features[i].flags & LY_NODE_FENABLED) {
                (*states)[count] = 1;
            } else {
                (*states)[count] = 0;
            }
        }
        count++;
    }

    /* submodules */
    for (j = 0; j < module->inc_size; j++) {
        for (i = 0; i < module->inc[j].submodule->features_size; i++) {
            result[count] = module->inc[j].submodule->features[i].name;
            if (module->inc[j].submodule->features[i].flags & LY_NODE_FENABLED) {
                (*states)[count] = 1;
            } else {
                (*states)[count] = 0;
            }
            count++;
        }
    }

    /* TODO submodules of submodules ... */

    /* terminating NULL byte */
    result[count] = NULL;

    return result;
}

static struct ly_ident *
find_base_ident_sub(struct ly_module *module, struct ly_ident *ident, const char *basename)
{
    unsigned int i, j;
    struct ly_ident *base_iter = NULL;
    struct ly_ident_der *der;

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

struct ly_ident *
find_base_ident(struct ly_module *module, struct ly_ident *ident, const char *basename, int line, const char* parent)
{
    const char *name;
    int prefix_len = 0;
    int i, found = 0;
    struct ly_ident *result;

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
            LOGVAL(VE_INPREFIX, line, basename);
            return NULL;
        }
    } else {
        /* search in submodules */
        for (i = 0; i < module->inc_size; i++) {
            result = find_base_ident_sub((struct ly_module *)module->inc[i].submodule, ident, name);
            if (result) {
                return result;
            }
        }
    }

    /* search in the identified module */
    result = find_base_ident_sub(module, ident, name);
    if (!result) {
        LOGVAL(VE_INARG, line, basename, parent);
    }

    return result;
}

struct ly_ident *
find_identityref(struct ly_ident *base, const char *name, const char *ns)
{
    struct ly_ident_der *der;

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

API struct lyd_node *
lyd_parse(struct ly_ctx *ctx, const char *data, LY_DFORMAT format)
{
    if (!ctx || !data) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    switch (format) {
    case LY_DATA_XML:
        return xml_read_data(ctx, data);
    case LY_DATA_JSON:
    default:
        /* TODO */
        return NULL;
    }

    return NULL;
}

API void
lyd_free(struct lyd_node *node)
{
    struct lyd_node *next, *child;

    if (!node) {
        return;
    }

    if (!(node->schema->nodetype & (LY_NODE_LEAF | LY_NODE_LEAFLIST | LY_NODE_ANYXML))) {
        /* free children */
        LY_TREE_FOR_SAFE(node->child, next, child) {
            lyd_free(child);
        }
    } else if (node->schema->nodetype == LY_NODE_ANYXML) {
        lyxml_free_elem(((struct lyd_node_anyxml *)node)->ctx, ((struct lyd_node_anyxml *)node)->value);
    } else {
        /* free value */
        switch(((struct lyd_node_leaf *)node)->value_type) {
        case LY_TYPE_BINARY:
        case LY_TYPE_STRING:
            lydict_remove(node->schema->module->ctx, ((struct lyd_node_leaf *)node)->value.string);
            break;
        case LY_TYPE_BITS:
            if (((struct lyd_node_leaf *)node)->value.bit) {
                free(((struct lyd_node_leaf *)node)->value.bit);
            }
            break;
        default:
            /* TODO nothing needed : LY_TYPE_BOOL, LY_TYPE_DEC64*/
            break;
        }
    }

    if (node->prev->next) {
        node->prev->next = node->next;
    } else if (node->parent) {
        /* first node */
        node->parent->child = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }

    free(node);
}

API int
lyd_islast(struct lyd_node *node)
{
    struct lyd_node *n;

    if (!node->next) {
        return 1;
    }

    for (n = node->next; n; n = n->next) {
        switch (n->schema->nodetype) {
        case LY_NODE_LIST:
            if (!((struct lyd_node_list *)n)->lprev) {
                return 0;
            }
            break;
        case LY_NODE_LEAFLIST:
            if (!((struct lyd_node_leaflist *)n)->lprev) {
                return 0;
            }
            break;
        default:
            return 0;
        }
    }
    return 1;
}
