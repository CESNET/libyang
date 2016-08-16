/**
 * @file tree_data.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Manipulation with libyang data structures
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

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "tree_data.h"
#include "parser.h"
#include "resolve.h"
#include "xml_internal.h"
#include "tree_internal.h"
#include "validation.h"
#include "xpath.h"

static int lyd_unlink_internal(struct lyd_node *node, int permanent);

int
lyd_check_topmandatory(struct lyd_node *data, struct ly_ctx *ctx, int options)
{
    int i;

    if ((options & LYD_OPT_NOSIBLINGS) || (options & (LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG))) {
        return EXIT_SUCCESS;
    }

    assert(ctx);

    if (!(options & LYD_OPT_TYPEMASK) || (options & (LYD_OPT_DATA | LYD_OPT_CONFIG))) {
        /* check for missing mandatory elements (from the top level) according to schemas in context */
        for (i = 0; i < ctx->models.used; i++) {
            if (!ctx->models.list[i]->data) {
                continue;
            }
            if (ly_check_mandatory(data, ctx->models.list[i]->data, (options & LYD_OPT_TYPEMASK) ? 0 : 1, 0)) {
                return EXIT_FAILURE;
            }
        }
    } else if (options & LYD_OPT_NOTIF) {
        if (!data || data->parent || (data->prev != data) || (data->schema->nodetype != LYS_NOTIF)) {
            LOGERR(LY_EINVAL, "Subtree is not a single notification.");
            return EXIT_FAILURE;
        }
        if (ly_check_mandatory(data, NULL, 1, 0)) {
            return EXIT_FAILURE;
        }
    } else if (options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY)) {
        if (!data || data->parent || (data->prev != data) || !(data->schema->nodetype & (LYS_RPC | LYS_ACTION))) {
            LOGERR(LY_EINVAL, "Subtree is not a single RPC/reply.");
            return EXIT_FAILURE;
        }
        if (ly_check_mandatory(data, NULL, 1, (options & LYD_OPT_RPCREPLY) ? 1 : 0)) {
            return EXIT_FAILURE;
        }
    } else if (options & LYD_OPT_ACTION) {
        if (!data || data->parent || (data->prev != data) || !(data->schema->nodetype & (LYS_CONTAINER | LYS_LIST))) {
            LOGERR(LY_EINVAL, "Subtree is not a single action.");
            return EXIT_FAILURE;
        }
    } else {
        LOGINT;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static struct lyd_node *
lyd_parse_(struct ly_ctx *ctx, const struct lys_node *parent, const char *data, LYD_FORMAT format, int options)
{
    struct lyxml_elem *xml;
    struct lyd_node *result = NULL;
    int xmlopt = LYXML_PARSE_MULTIROOT;

    if (!ctx || !data) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    if (options & LYD_OPT_NOSIBLINGS) {
        xmlopt = 0;
    }

    switch (format) {
    case LYD_XML:
        xml = lyxml_parse_mem(ctx, data, xmlopt);
        if (ly_errno) {
            return NULL;
        }
        result = lyd_parse_xml(ctx, &xml, options, parent);
        lyxml_free_withsiblings(ctx, xml);
        break;
    case LYD_JSON:
        result = lyd_parse_json(ctx, parent, data, options);
        break;
    default:
        /* error */
        return NULL;
    }

    if (ly_errno) {
        lyd_free_withsiblings(result);
        return NULL;
    } else {
        return result;
    }
}

static struct lyd_node *
lyd_parse_data_(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options, va_list ap)
{
    const struct lys_node *rpc_act = NULL;

    if (lyp_check_options(options)) {
        LOGERR(LY_EINVAL, "%s: Invalid options (multiple data type flags set).", __func__);
        return NULL;
    }

    if (options & LYD_OPT_RPCREPLY) {
        rpc_act = va_arg(ap,  struct lys_node *);
        if (!rpc_act || !(rpc_act->nodetype & (LYS_RPC | LYS_ACTION))) {
            LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
            return NULL;
        }
    }

    return lyd_parse_(ctx, rpc_act, data, format, options);
}

API struct lyd_node *
lyd_parse_mem(struct ly_ctx *ctx, const char *data, LYD_FORMAT format, int options, ...)
{
    va_list ap;
    struct lyd_node *result;

    va_start(ap, options);
    result = lyd_parse_data_(ctx, data, format, options, ap);
    va_end(ap);

    return result;
}

static struct lyd_node *
lyd_parse_fd_(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options, va_list ap)
{
    struct lyd_node *ret;
    struct stat sb;
    char *data;

    if (!ctx || (fd == -1)) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    if (fstat(fd, &sb) == -1) {
        LOGERR(LY_ESYS, "Failed to stat the file descriptor (%s).", strerror(errno));
        return NULL;
    }

    if (!sb.st_size) {
        ly_errno = LY_SUCCESS;
        return NULL;
    }

    data = mmap(NULL, sb.st_size + 1, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        LOGERR(LY_ESYS, "Mapping file descriptor into memory failed.");
        return NULL;
    }

    ret = lyd_parse_data_(ctx, data, format, options, ap);

    munmap(data, sb.st_size + 1);

    return ret;
}

API struct lyd_node *
lyd_parse_fd(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options, ...)
{
    struct lyd_node *ret;
    va_list ap;

    va_start(ap, options);
    ret = lyd_parse_fd_(ctx, fd, format, options, ap);
    va_end(ap);

    return ret;
}

API struct lyd_node *
lyd_parse_path(struct ly_ctx *ctx, const char *path, LYD_FORMAT format, int options, ...)
{
    int fd;
    struct lyd_node *ret;
    va_list ap;

    if (!ctx || !path) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        LOGERR(LY_ESYS, "Failed to open data file \"%s\" (%s).", path, strerror(errno));
        return NULL;
    }

    va_start(ap, options);
    ret = lyd_parse_fd_(ctx, fd, format, options, ap);

    va_end(ap);
    close(fd);

    return ret;
}

static struct lys_node *
lyd_new_find_schema(struct lyd_node *parent, const struct lys_module *module, int rpc_output)
{
    struct lys_node *siblings;

    if (!parent) {
        siblings = module->data;
    } else {
        if (!parent->schema) {
            return NULL;
        }
        siblings = parent->schema->child;
        if (siblings && (siblings->nodetype == (rpc_output ? LYS_INPUT : LYS_OUTPUT))) {
            siblings = siblings->next;
        }
        if (siblings && (siblings->nodetype == (rpc_output ? LYS_OUTPUT : LYS_INPUT))) {
            siblings = siblings->child;
        }
    }

    return siblings;
}

struct lyd_node *
_lyd_new(struct lyd_node *parent, const struct lys_node *schema)
{
    struct lyd_node *ret;

    ret = calloc(1, sizeof *ret);
    if (!ret) {
        LOGMEM;
        return NULL;
    }
    ret->schema = (struct lys_node *)schema;
    ret->validity = LYD_VAL_NOT;
    if (resolve_applies_when(ret)) {
        ret->when_status = LYD_WHEN;
    }
    ret->prev = ret;
    if (parent) {
        if (lyd_insert(parent, ret)) {
            lyd_free(ret);
            return NULL;
        }
    }

    return ret;
}

API struct lyd_node *
lyd_new(struct lyd_node *parent, const struct lys_module *module, const char *name)
{
    const struct lys_node *snode = NULL, *siblings;

    if ((!parent && !module) || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    siblings = lyd_new_find_schema(parent, module, 0);
    if (!siblings) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (lys_get_data_sibling(module, siblings, name, LYS_CONTAINER | LYS_LIST | LYS_NOTIF | LYS_RPC | LYS_ACTION, &snode)
            || !snode) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    return _lyd_new(parent, snode);
}

static struct lyd_node *
lyd_create_leaf(const struct lys_node *schema, const char *val_str)
{
    struct lyd_node_leaf_list *ret;

    ret = calloc(1, sizeof *ret);
    if (!ret) {
        LOGMEM;
        return NULL;
    }
    ret->schema = (struct lys_node *)schema;
    ret->validity = LYD_VAL_NOT;
    if (resolve_applies_when((struct lyd_node *)ret)) {
        ret->when_status = LYD_WHEN;
    }
    ret->prev = (struct lyd_node *)ret;
    ret->value_type = ((struct lys_node_leaf *)schema)->type.base;
    ret->value_str = lydict_insert(schema->module->ctx, val_str ? val_str : "", 0);

    return (struct lyd_node *)ret;
}

static struct lyd_node *
_lyd_new_leaf(struct lyd_node *parent, const struct lys_node *schema, const char *val_str)
{
    struct lyd_node *ret;

    ret = lyd_create_leaf(schema, val_str);
    if (!ret) {
        return NULL;
    }

    /* connect to parent */
    if (parent) {
        if (lyd_insert(parent, ret)) {
            lyd_free(ret);
            return NULL;
        }
    }

    /* resolve the type correctly (after it was connected to parent cause of log) */
    if (lyp_parse_value((struct lyd_node_leaf_list *)ret, NULL, 1)) {
        lyd_free((struct lyd_node *)ret);
        return NULL;
    }

    if (ret->schema->flags & LYS_UNIQUE) {
        /* locate the first parent list */
        for (parent = ret->parent; parent && parent->schema->nodetype != LYS_LIST; parent = parent->parent);

        /* set flag for future validation */
        if (parent) {
            parent->validity |= LYD_VAL_UNIQUE;
        }
    }

    return ret;
}

API struct lyd_node *
lyd_new_leaf(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *val_str)
{
    const struct lys_node *snode = NULL, *siblings;

    if ((!parent && !module) || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    siblings = lyd_new_find_schema(parent, module, 0);
    if (!siblings) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (lys_get_data_sibling(module, siblings, name, LYS_LEAFLIST | LYS_LEAF, &snode) || !snode) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    return _lyd_new_leaf(parent, snode, val_str);
}

API int
lyd_change_leaf(struct lyd_node_leaf_list *leaf, const char *val_str)
{
    const char *backup;
    lyd_val backup_val;
    struct lyd_node *parent;
    struct lys_node_list *slist;
    uint32_t i;

    if (!leaf) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    /* key value cannot be changed */
    if (leaf->parent && (leaf->parent->schema->nodetype == LYS_LIST)) {
        slist = (struct lys_node_list *)leaf->parent->schema;
        for (i = 0; i < slist->keys_size; ++i) {
            if (ly_strequal(slist->keys[i]->name, leaf->schema->name, 1)) {
                LOGVAL(LYE_SPEC, LY_VLOG_LYD, leaf, "List key value cannot be changed.");
                return EXIT_FAILURE;
            }
        }
    }

    if (!strcmp(leaf->value_str, val_str ? val_str : "")) {
        /* the value remains the same */
        return EXIT_SUCCESS;
    }

    backup = leaf->value_str;
    memcpy(&backup_val, &leaf->value, sizeof backup);
    leaf->value_str = lydict_insert(leaf->schema->module->ctx, val_str ? val_str : "", 0);
    /* leaf->value is erased by lyp_parse_value() */

    /* resolve the type correctly */
    if (lyp_parse_value(leaf, NULL, 1)) {
        lydict_remove(leaf->schema->module->ctx, leaf->value_str);
        leaf->value_str = backup;
        memcpy(&leaf->value, &backup_val, sizeof backup);
        return EXIT_FAILURE;
    }

    /* value is correct, remove backup */
    lydict_remove(leaf->schema->module->ctx, backup);

    /* clear the default flag, the value is different */
    leaf->dflt = 0;

    if (leaf->schema->flags & LYS_UNIQUE) {
        /* locate the first parent list */
        for (parent = leaf->parent; parent && parent->schema->nodetype != LYS_LIST; parent = parent->parent);

        /* set flag for future validation */
        if (parent) {
            parent->validity |= LYD_VAL_UNIQUE;
        }
    }

    return EXIT_SUCCESS;
}

static struct lyd_node *
lyd_create_anyxml(const struct lys_node *schema, char *val_str, struct lyxml_elem *val_xml)
{
    struct lyd_node_anyxml *ret;

    assert(!val_str || !val_xml);

    ret = calloc(1, sizeof *ret);
    if (!ret) {
        LOGMEM;
        return NULL;
    }
    ret->schema = (struct lys_node *)schema;
    ret->validity = LYD_VAL_NOT;
    if (resolve_applies_when((struct lyd_node *)ret)) {
        ret->when_status = LYD_WHEN;
    }
    ret->prev = (struct lyd_node *)ret;

    if (val_str) {
        ret->xml_struct = 0;
        ret->value.str = lydict_insert_zc(schema->module->ctx, val_str);
    } else {
        ret->xml_struct = 1;
        ret->value.xml = val_xml;
    }

    return (struct lyd_node *)ret;
}

static struct lyd_node *
_lyd_new_anyxml(struct lyd_node *parent, const struct lys_node *schema, char *val_str, struct lyxml_elem *val_xml)
{
    struct lyd_node *ret;

    ret = lyd_create_anyxml(schema, val_str, val_xml);
    if (!ret) {
        return NULL;
    }

    /* connect to parent */
    if (parent) {
        if (lyd_insert(parent, ret)) {
            lyd_free(ret);
            return NULL;
        }
    }

    return ret;
}

API struct lyd_node *
lyd_new_anyxml_str(struct lyd_node *parent, const struct lys_module *module, const char *name, char *val_str)
{
    const struct lys_node *siblings, *snode;

    if ((!parent && !module) || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    siblings = lyd_new_find_schema(parent, module, 0);
    if (!siblings) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (lys_get_data_sibling(module, siblings, name, LYS_ANYXML, &snode) || !snode) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    return _lyd_new_anyxml(parent, snode, val_str, NULL);
}

API struct lyd_node *
lyd_new_anyxml_xml(struct lyd_node *parent, const struct lys_module *module, const char *name,
                   struct lyxml_elem *val_xml)
{
    const struct lys_node *siblings, *snode;

    if ((!parent && !module) || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    siblings = lyd_new_find_schema(parent, module, 0);
    if (!siblings) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (lys_get_data_sibling(module, siblings, name, LYS_ANYXML, &snode) || !snode) {
        return NULL;
    }

    return _lyd_new_anyxml(parent, snode, NULL, val_xml);
}

API struct lyd_node *
lyd_new_output(struct lyd_node *parent, const struct lys_module *module, const char *name)
{
    const struct lys_node *snode = NULL, *siblings;

    if ((!parent && !module) || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    siblings = lyd_new_find_schema(parent, module, 1);
    if (!siblings) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (lys_get_data_sibling(module, siblings, name, LYS_CONTAINER | LYS_LIST | LYS_NOTIF | LYS_RPC | LYS_ACTION, &snode)
            || !snode) {
        return NULL;
    }

    return _lyd_new(parent, snode);
}

API struct lyd_node *
lyd_new_output_leaf(struct lyd_node *parent, const struct lys_module *module, const char *name, const char *val_str)
{
    const struct lys_node *snode = NULL, *siblings;

    if ((!parent && !module) || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    siblings = lyd_new_find_schema(parent, module, 1);
    if (!siblings) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (lys_get_data_sibling(module, siblings, name, LYS_LEAFLIST | LYS_LEAF, &snode) || !snode) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    return _lyd_new_leaf(parent, snode, val_str);
}

API struct lyd_node *
lyd_new_output_anyxml_str(struct lyd_node *parent, const struct lys_module *module, const char *name, char *val_str)
{
    const struct lys_node *siblings, *snode;

    if ((!parent && !module) || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    siblings = lyd_new_find_schema(parent, module, 1);
    if (!siblings) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (lys_get_data_sibling(module, siblings, name, LYS_ANYXML, &snode) || !snode) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    return _lyd_new_anyxml(parent, snode, val_str, NULL);
}

API struct lyd_node *
lyd_new_output_anyxml_xml(struct lyd_node *parent, const struct lys_module *module, const char *name,
                          struct lyxml_elem *val_xml)
{
    const struct lys_node *siblings, *snode;

    if ((!parent && !module) || !name) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    siblings = lyd_new_find_schema(parent, module, 1);
    if (!siblings) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (lys_get_data_sibling(module, siblings, name, LYS_ANYXML, &snode) || !snode) {
        return NULL;
    }

    return _lyd_new_anyxml(parent, snode, NULL, val_xml);
}

static int
lyd_new_path_list_keys(struct lyd_node *list, const char *list_name, const char *predicate, int *parsed)
{
    const char *name, *value;
    char *key_val;
    int r, i, nam_len, val_len, has_predicate = 1;
    struct lys_node_list *slist;

    slist = (struct lys_node_list *)list->schema;

    for (i = 0; i < slist->keys_size; ++i) {
        if (!has_predicate) {
            LOGVAL(LYE_PATH_MISSKEY, LY_VLOG_NONE, NULL, list_name);
            return -1;
        }

        if (((r = parse_schema_json_predicate(predicate, &name, &nam_len, &value, &val_len, &has_predicate)) < 1)
                || !strncmp(name, ".", nam_len)) {
            LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, predicate[-r], &predicate[-r]);
            return -1;
        }
        *parsed += r;
        predicate += r;

        if (strncmp(slist->keys[i]->name, name, nam_len) || slist->keys[i]->name[nam_len]) {
            LOGVAL(LYE_PATH_INKEY, LY_VLOG_NONE, NULL, name[0], name);
            return -1;
        }

        key_val = malloc((val_len + 1) * sizeof(char));
        if (!key_val) {
            LOGMEM;
            return -1;
        }
        strncpy(key_val, value, val_len);
        key_val[val_len] = '\0';

        if (!_lyd_new_leaf(list, (const struct lys_node *)slist->keys[i], key_val)) {
            free(key_val);
            return -1;
        }
        free(key_val);
    }

    return 0;
}

API struct lyd_node *
lyd_new_path(struct lyd_node *data_tree, struct ly_ctx *ctx, const char *path, const char *value, int options)
{
    char *module_name = ly_buf(), *buf_backup = NULL, *str;
    const char *mod_name, *name, *val_name, *val, *node_mod_name, *id;
    struct lyd_node *ret = NULL, *node, *parent = NULL;
    const struct lys_node *schild, *sparent;
    const struct lys_node_list *slist;
    const struct lys_module *module, *prev_mod;
    int r, i, parsed = 0, mod_name_len, nam_len, val_name_len, val_len;
    int is_relative = -1, has_predicate, first_iter = 1;

    if (!path || (!data_tree && !ctx)
            || (!data_tree && (path[0] != '/'))) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    if (!ctx) {
        ctx = data_tree->schema->module->ctx;
    }

    id = path;

    if (data_tree) {
        parent = resolve_partial_json_data_nodeid(id, value, data_tree, options, &parsed);
        if (parsed == -1) {
            return NULL;
        }
        if (parsed) {
            assert(parent);
            /* if we parsed something we have a relative path now for sure, otherwise we don't know */
            is_relative = 1;

            id += parsed;

            if (!id[0]) {
                /* the node exists */
                if (!(options & LYD_PATH_OPT_UPDATE)) {
                    LOGVAL(LYE_PATH_EXISTS, LY_VLOG_STR, path);
                    return NULL;
                }

                /* update leaf value if needed */
                if ((parent->schema->nodetype == LYS_LEAF)
                        && (!value || strcmp(((struct lyd_node_leaf_list *)parent)->value_str, value))) {
                    r = lyd_change_leaf((struct lyd_node_leaf_list *)parent, value);
                    if (r) {
                        return NULL;
                    }

                    return parent;
                }

                return NULL;
            }
        }
    }

    if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, &has_predicate)) < 1) {
        LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
        return NULL;
    }

    id += r;

    /* prepare everything for the schema search loop */
    if (is_relative) {
        /* we are relative to data_tree or parent if some part of the path already exists */
        if (!data_tree) {
            LOGERR(LY_EINVAL, "%s: provided relative path (%s) without context node.", path);
            return NULL;
        } else if (!parent) {
            parent = data_tree;
        }
        sparent = parent->schema;
        module = lys_node_module(sparent);
        prev_mod = module;
    } else {
        /* we are starting from scratch, absolute path */
        assert(!parent);
        if (!mod_name) {
            str = strndup(path, (name + nam_len) - path);
            LOGVAL(LYE_PATH_MISSMOD, LY_VLOG_STR, str);
            free(str);
            return NULL;
        } else if (mod_name_len > LY_BUF_SIZE - 1) {
            LOGINT;
            return NULL;
        }

        if (ly_buf_used && module_name[0]) {
            buf_backup = strndup(module_name, LY_BUF_SIZE - 1);
        }
        ly_buf_used++;

        memmove(module_name, mod_name, mod_name_len);
        module_name[mod_name_len] = '\0';
        module = ly_ctx_get_module(ctx, module_name, NULL);

        if (buf_backup) {
            /* return previous internal buffer content */
            strcpy(module_name, buf_backup);
            free(buf_backup);
        }
        ly_buf_used--;

        if (!module) {
            str = strndup(path, (mod_name + mod_name_len) - path);
            LOGVAL(LYE_PATH_INMOD, LY_VLOG_STR, str);
            free(str);
            return NULL;
        }
        mod_name = NULL;
        mod_name_len = 0;
        prev_mod = module;

        sparent = NULL;
    }

    /* create nodes in a loop */
    while (1) {
        /* find the schema node */
        schild = NULL;
        while ((schild = lys_getnext(schild, sparent, module, 0))) {
            if (schild->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_NOTIF | LYS_RPC | LYS_ACTION)) {
                /* module comparison */
                if (mod_name) {
                    node_mod_name = lys_node_module(schild)->name;
                    if (strncmp(node_mod_name, mod_name, mod_name_len) || node_mod_name[mod_name_len]) {
                        continue;
                    }
                } else if (lys_node_module(schild) != prev_mod) {
                    continue;
                }

                /* name check */
                if (strncmp(schild->name, name, nam_len) || schild->name[nam_len]) {
                    continue;
                }

                /* RPC in/out check */
                if (lys_parent(schild)) {
                    if (options & LYD_PATH_OPT_OUTPUT) {
                        if (lys_parent(schild)->nodetype == LYS_INPUT) {
                            continue;
                        }
                    } else {
                        if (lys_parent(schild)->nodetype == LYS_OUTPUT) {
                            continue;
                        }
                    }
                }

                break;
            }
        }

        if (!schild) {
            str = strndup(path, (name + nam_len) - path);
            LOGVAL(LYE_PATH_INNODE, LY_VLOG_STR, str);
            free(str);
            lyd_free(ret);
            return NULL;
        }

        /* we have the right schema node */
        switch (schild->nodetype) {
        case LYS_CONTAINER:
        case LYS_LIST:
        case LYS_NOTIF:
        case LYS_RPC:
        case LYS_ACTION:
            node = _lyd_new(is_relative ? parent : NULL, schild);
            break;
        case LYS_LEAF:
        case LYS_LEAFLIST:
            str = NULL;
            if (has_predicate) {
                if ((r = parse_schema_json_predicate(id, &val_name, &val_name_len, &val, &val_len, &has_predicate)) < 1) {
                    LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
                    lyd_free(ret);
                    return NULL;
                }
                id += r;

                if ((val_name[0] != '.') || (val_name_len != 1)) {
                    LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, val_name[0], val_name);
                    lyd_free(ret);
                    return NULL;
                }

                str = strndup(val, val_len);
                if (!str) {
                    LOGMEM;
                    lyd_free(ret);
                    return NULL;
                }
            }
            if (id[0]) {
                LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
                free(str);
                lyd_free(ret);
                return NULL;
            }
            node = _lyd_new_leaf(is_relative ? parent : NULL, schild, (str ? str : value));
            free(str);
            break;
        case LYS_ANYXML:
            if (id[0]) {
                LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
                lyd_free(ret);
                return NULL;
            }
            str = strdup(value ? value : "");
            if (!str) {
                LOGMEM;
                lyd_free(ret);
                return NULL;
            }
            node = _lyd_new_anyxml(is_relative ? parent : NULL, schild, str, NULL);
            break;
        default:
            LOGINT;
            node = NULL;
            break;
        }

        if (!node) {
            str = strndup(path, id - path);
            if (is_relative) {
                LOGVAL(LYE_SPEC, LY_VLOG_STR, str, "Failed to create node \"%s\" as a child of \"%s\".",
                       schild->name, parent->schema->name);
            } else {
                LOGVAL(LYE_SPEC, LY_VLOG_STR, str, "Failed to create node \"%s\".", schild->name);
            }
            free(str);
            lyd_free(ret);
            return NULL;
        }
        /* special case when we are creating a sibling of a top-level data node */
        if (!is_relative) {
            if (data_tree) {
                for (; data_tree->next; data_tree = data_tree->next);
                if (lyd_insert_after(data_tree, node)) {
                    lyd_free(ret);
                    return NULL;
                }
            }
            is_relative = 1;
        }

        if (first_iter) {
            /* sort if needed, but only when inserted somewhere */
            sparent = node->schema;
            do {
                sparent = lys_parent(sparent);
            } while (sparent && (sparent->nodetype != ((options & LYD_PATH_OPT_OUTPUT) ? LYS_OUTPUT : LYS_INPUT)));
            if (sparent && lyd_schema_sort(node, 0)) {
                lyd_free(ret);
                return NULL;
            }
            ret = node;
            first_iter = 0;
        }

        parsed = 0;
        if ((schild->nodetype == LYS_LIST) && (!has_predicate || lyd_new_path_list_keys(node, name, id, &parsed))) {
            lyd_free(ret);
            return NULL;
        }
        id += parsed;

        if (!id[0]) {
            /* we are done */
            return ret;
        } else if (options & LYD_PATH_OPT_NOPARENT) {
            /* we were supposed to be done */
            str = strndup(path, (name + nam_len) - path);
            LOGVAL(LYE_PATH_MISSPAR, LY_VLOG_STR, str);
            free(str);
            return NULL;
        }

        /* prepare for another iteration */
        parent = node;
        sparent = schild;
        prev_mod = lys_node_module(schild);

        /* parse another node */
        if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, &has_predicate)) < 1) {
            LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
            lyd_free(ret);
            return NULL;
        }
        id += r;

        /* if a key of a list was supposed to be created, it is created as a part of the list instance creation */
        if ((schild->nodetype == LYS_LIST) && !mod_name) {
            slist = (const struct lys_node_list *)schild;
            for (i = 0; i < slist->keys_size; ++i) {
                if (!strncmp(slist->keys[i]->name, name, nam_len) && !slist->keys[i]->name[nam_len]) {
                    /* the path continues? there cannot be anything after a key (leaf) */
                    if (id[0]) {
                        LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
                        lyd_free(ret);
                        return NULL;
                    }
                    return ret;
                }
            }
        }
    }

    LOGINT;
    return NULL;
}

/* both target and source were validated */
static void
lyd_merge_node_update(struct lyd_node *target, struct lyd_node *source)
{
    struct ly_ctx *ctx;
    struct lyd_node_leaf_list *trg_leaf, *src_leaf;
    struct lyd_node_anyxml *trg_axml, *src_axml;

    assert((target->schema == source->schema) && (target->schema->nodetype & (LYS_LEAF | LYS_ANYXML)));
    ctx = target->schema->module->ctx;

    if (target->schema->nodetype == LYS_LEAF) {
        trg_leaf = (struct lyd_node_leaf_list *)target;
        src_leaf = (struct lyd_node_leaf_list *)source;

        lydict_remove(ctx, trg_leaf->value_str);
        trg_leaf->value_str = src_leaf->value_str;
        src_leaf->value_str = NULL;

        trg_leaf->value = src_leaf->value;
        src_leaf->value = (lyd_val)0;
        if ((trg_leaf->value_type == LY_TYPE_INST) || (trg_leaf->value_type == LY_TYPE_LEAFREF)) {
            /* these are, for instance, cases when the resulting data tree will definitely not be valid */
            trg_leaf->value = (lyd_val)0;
        }
    } else {
        trg_axml = (struct lyd_node_anyxml *)target;
        src_axml = (struct lyd_node_anyxml *)source;

        if (trg_axml->xml_struct) {
            lyxml_free(ctx, trg_axml->value.xml);
        } else {
            lydict_remove(ctx, trg_axml->value.str);
        }

        trg_axml->xml_struct = src_axml->xml_struct;
        trg_axml->value = src_axml->value;

        src_axml->xml_struct = 1;
        src_axml->value.xml = NULL;
    }
}

static int
lyd_merge_node_equal(struct lyd_node *node1, struct lyd_node *node2)
{
    int i;
    struct lyd_node *child1, *child2;

    if (node1->schema != node2->schema) {
        return 0;
    }

    switch (node1->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LEAF:
    case LYS_ANYXML:
        return 1;
    case LYS_LEAFLIST:
        if (!strcmp(((struct lyd_node_leaf_list *)node1)->value_str, ((struct lyd_node_leaf_list *)node2)->value_str)) {
            return 1;
        }
        break;
    case LYS_LIST:
        child1 = node1->child;
        child2 = node2->child;
        /* the exact data order is guaranteed */
        for (i = 0; i < ((struct lys_node_list *)node1->schema)->keys_size; ++i) {
            if (!child1 || !child2 || (child1->schema != child2->schema)
                    || strcmp(((struct lyd_node_leaf_list *)child1)->value_str, ((struct lyd_node_leaf_list *)child2)->value_str)) {
                break;
            }
            child1 = child1->next;
            child2 = child2->next;
        }
        if (i == ((struct lys_node_list *)node1->schema)->keys_size) {
            return 1;
        }
        break;
    default:
        LOGINT;
        break;
    }

    return 0;
}

/* spends source */
static int
lyd_merge_parent_children(struct lyd_node *target, struct lyd_node *source)
{
    struct lyd_node *trg_parent, *src, *src_backup, *src_elem, *src_elem_backup, *src_next, *trg_child, *trg_parent_backup;

    LY_TREE_FOR_SAFE(source, src_backup, src) {
        for (src_elem = src_next = src, trg_parent = target;
            src_elem;
            src_elem = src_next) {

            LY_TREE_FOR(trg_parent->child, trg_child) {
                /* schema match, data match? */
                if (lyd_merge_node_equal(trg_child, src_elem)) {
                    if (trg_child->schema->nodetype & (LYS_LEAF | LYS_ANYXML)) {
                        lyd_merge_node_update(trg_child, src_elem);
                    }
                    break;
                }
            }

            /* first prepare for the next iteration */
            src_elem_backup = src_elem;
            trg_parent_backup = trg_parent;
            if ((src_elem->schema->nodetype & (LYS_CONTAINER | LYS_LIST)) && trg_child) {
                /* go into children */
                src_next = src_elem->child;
                trg_parent = trg_child;
            } else {
                /* no children (or the whole subtree will be inserted), try siblings */
                if (src_elem == src) {
                    /* we are done, src has no children, but we still need to insert it */
                    src_next = NULL;
                    src_elem = src_elem->child;
                } else {
                    src_next = src_elem->next;
                    /* trg_parent does not change */
                }
            }
            while (!src_next) {
                src_elem = src_elem->parent;
                if (src_elem->parent == src->parent) {
                    /* we are done, no next element to process */
                    break;
                }

                /* parent is already processed, go to its sibling */
                src_next = src_elem->next;
                trg_parent = trg_parent->parent;
            }

            if (!trg_child) {
                /* we need to insert the whole subtree */
                if (lyd_insert(trg_parent_backup, src_elem_backup)) {
                    LOGINT;
                    lyd_free_withsiblings(source);
                    return -1;
                }
                if (src_elem == src) {
                    /* we are finished for this src, we spent it, so forget the pointer if available */
                    if (source == src) {
                        source = source->next;
                    }
                    break;
                }
            }
        }
    }

    lyd_free_withsiblings(source);
    return 0;
}

/* spends source */
static int
lyd_merge_siblings(struct lyd_node *target, struct lyd_node *source)
{
    struct lyd_node *trg, *src, *src_backup;

    while (target->prev->next) {
        target = target->prev;
    }

    LY_TREE_FOR_SAFE(source, src_backup, src) {
        LY_TREE_FOR(target, trg) {
            /* sibling found, merge it */
            if (lyd_merge_node_equal(trg, src)) {
                if (trg->schema->nodetype & (LYS_LEAF | LYS_ANYXML)) {
                    lyd_merge_node_update(trg, src);
                } else {
                    if (lyd_merge_parent_children(trg, src->child)) {
                        lyd_free_withsiblings(source);
                        return -1;
                    }
                }
                break;
            }
        }

        /* sibling not found, insert it */
        if (!trg) {
            lyd_insert_after(target->prev, src);
            if (src == source) {
                /* just so source is not freed, we inserted it and need it further */
                source = src_backup;
            }
        }
    }

    lyd_free_withsiblings(source);
    return 0;
}

API int
lyd_merge(struct lyd_node *target, const struct lyd_node *source, int options)
{
    struct lyd_node *node = NULL, *node2, *trg_merge_start, *src_merge_start = NULL;
    struct lys_node *src_snode;
    int i, src_depth, depth, first_iter, ret;

    if (!target || !source || (target->schema->module->ctx != source->schema->module->ctx)) {
        ly_errno = LY_EINVAL;
        return -1;
    }

    if (lys_parent(target->schema)) {
        LOGERR(LY_EINVAL, "Target not a top-level data tree.");
        return -1;
    }

    /* find source top-level schema node */
    for (src_snode = source->schema, src_depth = 0;
         lys_parent(src_snode);
         src_snode = lys_parent(src_snode), ++src_depth);

    /* check for forbidden data merges */
    if (target->schema->nodetype & (LYS_RPC | LYS_NOTIF)) {
        if (target->schema != src_snode) {
            LOGERR(LY_EINVAL, "Target RPC or notification, source not (or a different schema node).");
            return -1;
        }
    } else {
        if (src_snode->nodetype & (LYS_RPC | LYS_NOTIF)) {
            LOGERR(LY_EINVAL, "Target data, source RPC output, RPC input, or a notification.");
            return -1;
        }
    }

    /* find first shared missing schema parent of the subtrees */
    trg_merge_start = target;
    depth = 0;
    first_iter = 1;
    while (1) {
        do {
            for (src_snode = source->schema, i = 0; i < src_depth - depth; src_snode = lys_parent(src_snode), ++i);
            ++depth;
        } while (src_snode != source->schema && (src_snode->nodetype & (LYS_CHOICE | LYS_CASE | LYS_USES)));

        if (src_snode == source->schema) {
            break;
        }

        if (src_snode->nodetype != LYS_CONTAINER) {
            /* we would have to create a list (the only data node with children except container), impossible */
            LOGERR(LY_EINVAL, "Cannot create %s \"%s\" for the merge.", strnodetype(src_snode->nodetype), src_snode->name);
            goto error;
        }

        /* have we created any missing containers already? if we did,
         * it is totally useless to search for match, there won't ever be */
        if (!src_merge_start) {
            if (first_iter) {
                node = trg_merge_start;
                first_iter = 0;
            } else {
                node = trg_merge_start->child;
            }

            /* find it in target data nodes */
            LY_TREE_FOR(node, node) {
                if (node->schema == src_snode) {
                    trg_merge_start = node;
                    break;
                }
            }
        }

        if (!node) {
            /* it is not there, create it */
            src_merge_start = _lyd_new(src_merge_start, src_snode);
        }
    }

    /* process source according to options */
    if (options & LYD_OPT_DESTRUCT) {
        node = (struct lyd_node *)source;
        if ((node->prev != node) && (options & LYD_OPT_NOSIBLINGS)) {
            node2 = node->prev;
            lyd_unlink(node);
            lyd_free_withsiblings(node2);
        }
    } else {
        node = NULL;
        for (; source; source = source->next) {
            node2 = lyd_dup(source, 1);
            if (!node2) {
                goto error;
            }
            if (node) {
                if (lyd_insert_after(node->prev, node2)) {
                    goto error;
                }
            } else {
                node = node2;
            }

            if (options & LYD_OPT_NOSIBLINGS) {
                break;
            }
        }
    }

    if (src_merge_start) {
        src_merge_start->child = node;
        LY_TREE_FOR(node, node) {
            node->parent = src_merge_start;
        }
    } else {
        src_merge_start = node;
    }

    if (!first_iter) {
        /* !! src_merge start is a child(ren) of trg_merge_start */
        ret = lyd_merge_parent_children(trg_merge_start, src_merge_start);
    } else {
        /* !! src_merge start is a (top-level) sibling(s) of trg_merge_start */
        ret = lyd_merge_siblings(trg_merge_start, src_merge_start);
    }

    if (target->schema->nodetype == LYS_RPC) {
        lyd_schema_sort(target, 1);
    }

    return ret;

error:
    lyd_free_withsiblings(node);
    lyd_free_withsiblings(src_merge_start);
    return -1;
}

API void
lyd_free_diff(struct lyd_difflist *diff)
{
    if (diff) {
        free(diff->type);
        free(diff->first);
        free(diff->second);
        free(diff);
    }
}

static int
lyd_difflist_add(struct lyd_difflist *diff, unsigned int *size, unsigned int index,
                 LYD_DIFFTYPE type, struct lyd_node *first, struct lyd_node *second)
{
    void *new;

    assert(diff);
    assert(size && *size);

    if (index + 1 == *size) {
        /* it's time to enlarge */
        *size = *size + 16;
        new = realloc(diff->type, *size * sizeof *diff->type);
        if (!new) {
            LOGMEM;
            return EXIT_FAILURE;
        }
        diff->type = new;

        new = realloc(diff->first, *size * sizeof *diff->first);
        if (!new) {
            LOGMEM;
            return EXIT_FAILURE;
        }
        diff->first = new;

        new = realloc(diff->second, *size * sizeof *diff->second);
        if (!new) {
            LOGMEM;
            return EXIT_FAILURE;
        }
        diff->second = new;
    }

    /* insert the item */
    diff->type[index] = type;
    diff->first[index] = first;
    diff->second[index] = second;

    /* terminate the arrays */
    index++;
    diff->type[index] = LYD_DIFF_END;
    diff->first[index] = NULL;
    diff->second[index] = NULL;

    return EXIT_SUCCESS;
}

struct diff_ordered_dist {
    struct diff_ordered_dist *next;
    int dist;
};
struct diff_ordered_item {
    struct lyd_node *first;
    struct lyd_node *second;
    struct diff_ordered_dist *dist;
};
struct diff_ordered {
    struct lys_node *schema;
    unsigned int count;
    struct diff_ordered_item *items; /* array */
    struct diff_ordered_dist *dist;  /* linked list (1-way, ring) */
    struct diff_ordered_dist *dist_last;  /* aux pointer for faster insertion sort */
};

static void
diff_ordset_insert(struct lyd_node *node, struct ly_set *ordset_keys, struct ly_set *ordset)
{
    unsigned int i;
    struct diff_ordered *new_ordered;

    i = ly_set_add(ordset_keys, node->schema, 0);
    if (i == ordset->number) {
        /* not seen user-ordered list */
        new_ordered = malloc(sizeof *new_ordered);
        new_ordered->schema = node->schema;
        new_ordered->count = 0;
        new_ordered->items = NULL;
        new_ordered->dist = NULL;
        ly_set_add(ordset, new_ordered, LY_SET_OPT_USEASLIST);
    }
    ((struct diff_ordered *)ordset->set.g[i])->count++;
}

static void
diff_ordset_free(struct ly_set *set)
{
    unsigned int i, j;
    struct diff_ordered *ord;

    if (!set) {
        return;
    }

    for (i = 0; i < set->number; i++) {
        ord = (struct diff_ordered *)set->set.g[i];
        for (j = 0; j < ord->count; j++) {
            free(ord->items[j].dist);
        }
        free(ord->items);
        free(ord);
    }

    ly_set_free(set);
}

/*
 * -1 - error
 *  0 - ok
 *  1 - first and second not the same
 */
static int
lyd_diff_compare(struct lyd_node *first, struct lyd_node *second,
                 struct lyd_difflist *diff, unsigned int *size, unsigned int *i, struct ly_set *matchset,
                 struct ly_set *ordset_keys, struct ly_set *ordset)
{
    int rc;
    char *str1, *str2;
    struct lyd_node_anyxml *axml;

    switch (first->schema->nodetype) {
    case LYS_LEAFLIST:
    case LYS_LIST:
        rc = lyd_list_equal(first, second, 0, 0);
        if (rc == -1) {
            return -1;
        } else if (!rc) {
            /* list instances differs */
            return 1;
        } /* else matches */

        /* additional work for future move matching in case of user ordered lists */
        if (first->schema->flags & LYS_USERORDERED) {
            diff_ordset_insert(first, ordset_keys, ordset);
        }

        /* no break, fall through */
    case LYS_CONTAINER:
        second->validity |= LYD_VAL_INUSE;
        /* remember the matching node in first for keeping correct pointer in first
         * for comparing when passing through the second tree in lyd_diff().
         * Duplicities are not allowed actually, but they cannot happen since single
         * node can match only one node in the other tree */
        ly_set_add(matchset, first, LY_SET_OPT_USEASLIST);
        break;
    case LYS_LEAF:
        /* check for leaf's modification */
        if (!ly_strequal(((struct lyd_node_leaf_list * )first)->value_str,
                         ((struct lyd_node_leaf_list * )second)->value_str, 1)) {
            if (lyd_difflist_add(diff, size, (*i)++, LYD_DIFF_CHANGED, first, second)) {
               return -1;
            }
        }
        break;
    case LYS_ANYXML:
        /* check for anyxml's modification */
        axml = (struct lyd_node_anyxml *)first;
        if (!axml->value.str) {
            lyxml_print_mem(&str1, axml->value.xml, LYXML_PRINT_SIBLINGS);
            axml->value.str = lydict_insert_zc(axml->schema->module->ctx, str1);
        }
        str1 = (char *)axml->value.str;

        axml = (struct lyd_node_anyxml *)second;
        if (!axml->value.str) {
            lyxml_print_mem(&str2, axml->value.xml, LYXML_PRINT_SIBLINGS);
            axml->value.str = lydict_insert_zc(axml->schema->module->ctx, str2);
        }
        str2 = (char *)axml->value.str;

        if (!ly_strequal(str1, str2, 1)) {
            if (lyd_difflist_add(diff, size, (*i)++, LYD_DIFF_CHANGED, first, second)) {
                return -1;
            }
        }
        break;
    default:
        LOGINT;
        return -1;
    }

    /* mark both that they have matching instance in the other tree */
    first->validity |= LYD_VAL_INUSE;

    return 0;
}

static int
lyd_diff_move_preprocess(struct diff_ordered *ordered, struct lyd_node *first, struct lyd_node *second)
{
    struct lyd_node *iter;
    unsigned int pos = 0;
    int abs_dist;
    struct diff_ordered_dist *dist_aux;
    struct diff_ordered_dist *dist_iter, *dist_last;
    char *str = NULL;

    /* ordered->count was zeroed and now it is incremented with each added
     * item's information, so it is actually position of the second node
     */

    /* get the position of the first node */
    for (iter = first->prev; iter->next; iter = iter->prev) {
        if (!(iter->validity & LYD_VAL_INUSE)) {
            /* skip deleted nodes */
            continue;
        }
        if (iter->schema == first->schema) {
            pos++;
        }
    }
    if (pos != ordered->count) {
        LOGDBG("DIFF: Detected moved element \"%s\" from %d to %d (distance %d)",
               str = lyd_path(first), pos, ordered->count, ordered->count - pos);
        free(str);
    }

    /* store information, count distance */
    ordered->items[pos].dist = dist_aux = calloc(1, sizeof *dist_aux);
    ordered->items[pos].dist->dist = ordered->count - pos;
    abs_dist = abs(ordered->items[pos].dist->dist);
    ordered->items[pos].first = first;
    ordered->items[pos].second = second;
    ordered->count++;

    /* insert sort of distances, higher first */
    for (dist_iter = ordered->dist, dist_last = NULL;
            dist_iter;
            dist_last = dist_iter, dist_iter = dist_iter->next) {
        if (abs_dist >= abs(dist_iter->dist)) {
            /* found correct place */
            dist_aux->next = dist_iter;
            if (dist_last) {
                dist_last->next = dist_aux;
            }
            break;
        } else if (dist_iter->next == ordered->dist) {
            /* last item */
            dist_aux->next = ordered->dist; /* ring list */
            ordered->dist_last = dist_aux;
            break;
        }
    }
    if (dist_aux->next == ordered->dist) {
        /* first item */
        ordered->dist = dist_aux;
        if (dist_aux->next) {
            /* more than one item, update the last one's next */
            ordered->dist_last->next = dist_aux;
        } else {
            /* the only item */
            ordered->dist_last = dist_aux;
            dist_aux->next = dist_aux; /* ring list */
        }
    }

    return 0;
}

static struct lyd_difflist *
lyd_diff_init_difflist(unsigned int *size)
{
    struct lyd_difflist *result;

    result = malloc(sizeof *result);
    *size = 1;
    result->type = calloc(*size, sizeof *result->type);
    result->first = calloc(*size, sizeof *result->first);
    result->second = calloc(*size, sizeof *result->second);

    return result;
}

API struct lyd_difflist *
lyd_diff(struct lyd_node *first, struct lyd_node *second, int options)
{
    int rc;
    struct lyd_node *elem1, *elem2, *iter, *aux, *parent = NULL, *next1, *next2;
    struct lyd_difflist *result, *result2 = NULL;
    void *new;
    unsigned int size, size2, index = 0, index2 = 0, i, j, k;
    struct matchlist_s {
        struct matchlist_s *prev;
        struct ly_set *match;
    } *matchlist = NULL, *mlaux;
    struct ly_set *ordset_keys = NULL, *ordset = NULL;
    struct diff_ordered *ordered;
    struct diff_ordered_dist *dist_aux, *dist_iter;
    struct diff_ordered_item item_aux;

    if (!first) {
        /* all nodes in second were created,
         * but the second must be top level */
        if (second && second->parent) {
            LOGERR(LY_EINVAL, "%s: \"first\" parameter is NULL and \"second\" is not top level.", __func__);
            return NULL;
        }
        result = lyd_diff_init_difflist(&size);
        LY_TREE_FOR(second, iter) {
            if (lyd_difflist_add(result, &size, index++, LYD_DIFF_CREATED, NULL, iter)) {
                goto error;
            }
            if (options & LYD_OPT_NOSIBLINGS) {
                break;
            }
        }
        return result;
    } else if (!second) {
        /* all nodes from first were deleted */
        result = lyd_diff_init_difflist(&size);
        LY_TREE_FOR(first, iter) {
            if (lyd_difflist_add(result, &size, index++, LYD_DIFF_DELETED, iter, NULL)) {
                goto error;
            }
            if (options & LYD_OPT_NOSIBLINGS) {
                break;
            }
        }
        return result;
    }

    if (options & LYD_OPT_NOSIBLINGS) {
        /* both trees must start at the same (schema) node */
        if (first->schema != second->schema) {
            LOGERR(LY_EINVAL, "%s: incompatible trees to compare with LYD_OPT_NOSIBLINGS option.", __func__);
            return NULL;
        }
        /* use first's and second's child to make comparison the same as without LYD_OPT_NOSIBLINGS */
        first = first->child;
        second = second->child;
    } else {
        /* go to the first sibling in both trees */
        if (first->parent) {
            first = first->parent->child;
        } else {
            while (first->prev->next) {
                first = first->prev;
            }
        }

        if (second->parent) {
            second = second->parent->child;
        } else {
            for (; second->prev->next; second = second->prev);
        }

        /* check that both has the same (schema) parent or that they are top-level nodes */
        if ((first->parent && second->parent && first->parent->schema != second->parent->schema) ||
                (!first->parent && first->parent != second->parent)) {
            LOGERR(LY_EINVAL, "%s: incompatible trees with different parents.", __func__);
            return NULL;
        }
    }
    if (first == second) {
        LOGERR(LY_EINVAL, "%s: comparing the same tree does not make sense.", __func__);
        return NULL;
    }

    /* initiate resulting structure */
    result = lyd_diff_init_difflist(&size);

    /* the records about created and moved items are created in
     * bad order, so the records about created nodes (and their
     * possible moving) is stored separately and added to the
     * main result at the end.
     */
    result2 = lyd_diff_init_difflist(&size2);

    matchlist = malloc(sizeof *matchlist);
    matchlist->match = ly_set_new();
    matchlist->prev = NULL;

    ordset = ly_set_new();
    ordset_keys = ly_set_new();

    /*
     * compare trees
     */
    /* 1) newly created nodes + changed leafs/anyxmls */
    next1 = first;
    for (elem2 = next2 = second; elem2; elem2 = next2) {
        /* keep right pointer for searching in the first tree */
        elem1 = next1;

        /* search for elem2 instance in the first */
        LY_TREE_FOR(elem1, iter) {
            if (iter->schema != elem2->schema) {
                continue;
            }

            /* elem2 instance found */
            rc = lyd_diff_compare(iter, elem2, result, &size, &index, matchlist->match, ordset_keys, ordset);
            if (rc == -1) {
                goto error;
            } else if (rc == 0) {
                /* match */
                break;
            } /* else, continue */
        }

        if (!iter) {
            /* elem2 not found in the first tree */
            if (lyd_difflist_add(result2, &size2, index2++, LYD_DIFF_CREATED, elem1 ? elem1->parent : parent, elem2)) {
                goto error;
            }

            if (elem1 && (elem2->schema->flags & LYS_USERORDERED)) {
                /* store the correct place where the node is supposed to be moved after creation */
                /* if elem1 does not exist, all nodes were created and they will be created in
                 * correct order, so it is not needed to detect moves */
                for (aux = elem2->prev; aux->next; aux = aux->prev) {
                    if (aux->schema == elem2->schema) {
                        /* predecessor found */
                        break;
                    }
                }
                if (!aux->next) {
                    /* predecessor not found */
                    aux = NULL;
                }
                if (lyd_difflist_add(result2, &size2, index2++, LYD_DIFF_MOVEDAFTER2, aux, elem2)) {
                    goto error;
                }
            }
        }

        /* select element for the next run                                    1     2
         * - first, process all siblings of a single parent                  / \   / \
         * - then, go to children (deep)                                    3   4 7   8
         * - return to the parent's next sibling children                  / \
         *                                                                5   6
         */
        /* siblings first */
        next1 = elem1;
        next2 = elem2->next;

        if (!next2) {
            /* children */

            /* first pass of the siblings done, some additional work for future
             * detection of move may be needed */
            for (i = ordset->number; i > 0; i--) {
                ordered = (struct diff_ordered *)ordset->set.g[i - 1];
                if (ordered->items) {
                    /* already preprocessed ordered structure */
                    break;
                }
                ordered->items = calloc(ordered->count, sizeof *ordered->items);
                ordered->dist = NULL;
                /* zero the count to be used as a node position in lyd_diff_move_preprocess() */
                ordered->count = 0;
            }

            /* first, get the first sibling */
            if (elem2->parent == second->parent) {
                elem2 = second;
            } else {
                elem2 = elem2->parent->child;
            }

            /* and then find the first child */
            for (iter = elem2, i = 0; iter; iter = iter->next) {
                if (!(iter->validity & LYD_VAL_INUSE)) {
                    continue;
                }

                iter->validity &= ~LYD_VAL_INUSE;
                if ((iter->schema->nodetype & (LYS_LEAFLIST | LYS_LIST)) && (iter->schema->flags & LYS_USERORDERED)) {
                    for (j = ordset->number; j > 0; j--) {
                        ordered = (struct diff_ordered *)ordset->set.g[j - 1];
                        if (ordered->schema != iter->schema) {
                            continue;
                        }

                        /* store necessary information for move detection */
                        lyd_diff_move_preprocess(ordered, matchlist->match->set.d[i], iter);
                        break;
                    }
                }

                if ((iter->schema->nodetype & (LYS_CONTAINER | LYS_LIST)) && iter->child) {
                    while (!matchlist->match->set.d[i] || matchlist->match->set.d[i]->schema != iter->schema) {
                        i++;
                    }
                    next1 = matchlist->match->set.d[i]->child;
                    if (!next1) {
                        parent = matchlist->match->set.d[i];
                    }
                    matchlist->match->set.d[i] = NULL;
                    next2 = iter->child;
                    break;
                }
                i++;
            }

            if (!iter) {
                /* no child/data on next level */
                if (elem2 == second) {
                    /* done */
                    break;
                }
            } else {
                /* create new matchlist item */
                mlaux = malloc(sizeof *mlaux);
                mlaux->match = ly_set_new();
                mlaux->prev = matchlist;
                matchlist = mlaux;
            }
        }

        while (!next2) {
            /* parent */

            /* clean the last match set */
            ly_set_clean(matchlist->match);

            /* try to go to a cousin - child of the next parent's sibling */
            mlaux = matchlist->prev;
            for (i = 0; (i < mlaux->match->number) && !mlaux->match->set.d[i]; i++);
            for (iter = elem2->parent->next; iter; iter = iter->next) {
                if (!(iter->validity & LYD_VAL_INUSE)) {
                    continue;
                }

                iter->validity &= ~LYD_VAL_INUSE;
                if ((iter->schema->nodetype & (LYS_LEAFLIST | LYS_LIST)) && (iter->schema->flags & LYS_USERORDERED)) {
                    for (j = ordset->number ; j > 0; j--) {
                        ordered = (struct diff_ordered *)ordset->set.g[j - 1];
                        if (ordered->schema != iter->schema) {
                            continue;
                        }

                        /* store necessary information for move detection */
                        lyd_diff_move_preprocess(ordered, mlaux->match->set.d[i], iter);
                        break;
                    }
                }

                if ((iter->schema->nodetype & (LYS_CONTAINER | LYS_LIST)) && iter->child) {
                    while (!mlaux->match->set.d[i] || mlaux->match->set.d[i]->schema != iter->schema) {
                        i++;
                    }
                    next1 = mlaux->match->set.d[i]->child;
                    if (!next1) {
                        parent = mlaux->match->set.d[i];
                    }
                    mlaux->match->set.d[i] = NULL;
                    next2 = iter->child;
                    break;
                }
                i++;
            }

            /* if no cousin exists, continue next loop on higher level */
            if (!iter) {
                elem2 = elem2->parent;

                /* remove matchlist item */
                ly_set_free(matchlist->match);
                mlaux = matchlist;
                matchlist = matchlist->prev;
                free(mlaux);

                if (!matchlist->prev) { /* elem2->parent == second->parent */
                    /* done */
                    break;
                }
            }
        }
    }

    ly_set_free(matchlist->match);
    free(matchlist);
    matchlist = NULL;

    ly_set_free(ordset_keys);
    ordset_keys = NULL;

    /* 2) deleted nodes */
    LY_TREE_DFS_BEGIN(first, next1, elem1) {
        /* search for elem1s deleted in the second */
        if (elem1->validity & LYD_VAL_INUSE) {
            /* erase temporary LYD_VAL_INUSE flag and continue into children */
            elem1->validity &= ~LYD_VAL_INUSE;
        } else {
            /* elem1 has no matching node in second, add it into result */
            if (lyd_difflist_add(result, &size, index++, LYD_DIFF_DELETED, elem1, NULL)) {
                goto error;
            }

            /* skip subtree processing of data missing in the second tree */
            goto dfs_nextsibling;
        }

        /* modified LY_TREE_DFS_END() */
        /* select element for the next run - children first */
        if (elem1->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML)) {
            next1 = NULL;
        } else {
            next1 = elem1->child;
        }
        if (!next1) {
dfs_nextsibling:
            /* try siblings */
            next1 = elem1->next;
        }
        while (!next1) {
            /* parent is already processed, go to its sibling */

            elem1 = elem1->parent;
            if (elem1 == first->parent) {
                /* we are done, no next element to process */
                break;
            }

            next1 = elem1->next;
        }
    }

    /* 3) moved nodes (when user-ordered) */
    for (i = 0; i < ordset->number; i++) {
        ordered = (struct diff_ordered *)ordset->set.g[i];
        if (!ordered->dist->dist) {
            /* the dist list is sorted here, but the biggest dist is 0,
             * so nothing changed in order of these items between first
             * and second. We can continue with another user-ordered list.
             */
            continue;
        }

        /* get needed movements
         * - from the biggest distances try to apply node movements
         * on first tree node until they will be ordered as in the
         * second tree - i.e. until there will be no position difference
         */

        for (dist_iter = ordered->dist; ; dist_iter = dist_iter->next) {
            /* dist list is sorted at the beginning, since applying a move causes
             * just a small change in other distances, we assume the the biggest
             * dist is the next one (note that dist list is implemented as ring
             * list). This way we avoid sorting distances after each move. The loop
             * stops when all distances are zero.
             */
            dist_aux = dist_iter;
            while (!dist_iter->dist) {
                /* no dist, so no move. Try another, but when
                 * there is no dist at all, stop the loop
                 */
                dist_iter = dist_iter->next;
                if (dist_iter == dist_aux) {
                    /* all dist we zeroed */
                    goto movedone;
                }
            }
            /* something to move */

            /* get the item to move */
            for (k = 0; k < ordered->count; k++) {
                if (ordered->items[k].dist == dist_iter) {
                    break;
                }
            }

            /* apply the move (distance) */
            memcpy(&item_aux, &ordered->items[k], sizeof item_aux);
            if (dist_iter->dist > 0) {
                /* move to right (other move to left) */
                while (dist_iter->dist) {
                    memcpy(&ordered->items[k], &ordered->items[k + 1], sizeof *ordered->items);
                    ordered->items[k].dist->dist++; /* update moved item distance */
                    dist_iter->dist--;
                    k++;
                }
            } else {
                /* move to left (other move to right) */
                while (dist_iter->dist) {
                    memcpy(&ordered->items[k], &ordered->items[k - 1], sizeof *ordered->items);
                    ordered->items[k].dist->dist--; /* update moved item distance */
                    dist_iter->dist++;
                    k--;
                }
            }
            memcpy(&ordered->items[k], &item_aux, sizeof *ordered->items);

            /* store the transaction into the difflist */
            if (lyd_difflist_add(result, &size, index++, LYD_DIFF_MOVEDAFTER1, item_aux.first,
                                 (k > 0) ? ordered->items[k - 1].first : NULL)) {
                goto error;
            }
            continue;

movedone:
            break;
        }
    }

    diff_ordset_free(ordset);
    ordset = NULL;

    if (index2) {
        /* append result2 with newly created
         * (and possibly moved) nodes */
        if (index + index2 + 1 >= size) {
            /* result must be enlarged */
            size = index + index2 + 1;
            new = realloc(result->type, size * sizeof *result->type);
            if (!new) {
                LOGMEM;
                goto error;
            }
            result->type = new;

            new = realloc(result->first, size * sizeof *result->first);
            if (!new) {
                LOGMEM;
                goto error;
            }
            result->first = new;

            new = realloc(result->second, size * sizeof *result->second);
            if (!new) {
                LOGMEM ;
                goto error;
            }
            result->second = new;
        }

        /* append */
        memcpy(&result->type[index], result2->type, (index2 + 1) * sizeof *result->type);
        memcpy(&result->first[index], result2->first, (index2 + 1) * sizeof *result->first);
        memcpy(&result->second[index], result2->second, (index2 + 1) * sizeof *result->second);
    }
    lyd_free_diff(result2);

    ly_errno = LY_SUCCESS;
    return result;

error:

    while (matchlist) {
        mlaux = matchlist;
        matchlist = mlaux->prev;
        ly_set_free(mlaux->match);
        free(mlaux);

    }
    ly_set_free(ordset_keys);
    diff_ordset_free(ordset);

    lyd_free_diff(result);
    lyd_free_diff(result2);

    return NULL;
}

static void
lyd_insert_setinvalid(struct lyd_node *node)
{
    struct lyd_node *next, *elem, *parent_list;

    assert(node);

    /* overall validity of the node itself */
    node->validity = LYD_VAL_NOT;

    /* explore changed unique leafs */
    /* first, get know if there is a list in parents chain */
    for (parent_list = node->parent;
         parent_list && parent_list->schema->nodetype != LYS_LIST;
         parent_list = parent_list->parent);
    if (parent_list && !(parent_list->validity & LYD_VAL_UNIQUE)) {
        /* there is a list, so check if we inserted a leaf supposed to be unique */
        for (elem = node; elem; elem = next) {
            if (elem->schema->nodetype == LYS_LIST) {
                /* stop searching to the depth, children would be unique to a list in subtree */
                goto nextsibling;
            }

            if (elem->schema->nodetype == LYS_LEAF && (elem->schema->flags & LYS_UNIQUE)) {
                /* set flag to list for future validation */
                parent_list->validity |= LYD_VAL_UNIQUE;
                break;
            }

            if (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML)) {
                goto nextsibling;
            }

            /* select next elem to process */
            /* go into children */
            next = elem->child;
            /* go through siblings */
            if (!next) {
nextsibling:
                next = elem->next;
                if (!next) {
                    /* no sibling */
                    if (elem == node) {
                        /* we are done, back in start node */
                        break;
                    }
                }
            }
            /* go back to parents */
            while (!next) {
                if (elem->parent == node) {
                    /* we are done, back in start node */
                    break;
                }
                /* parent was actually already processed, so go to the parent's sibling */
                next = elem->parent->next;
            }
        }
    }

    if (node->parent) {
        /* if the inserted node is list/leaflist with constraint on max instances,
         * invalidate the parent to make it validate this */
        if (node->schema->nodetype & LYS_LEAFLIST) {
            if (((struct lys_node_leaflist *)node->schema)->max) {
                node->parent->validity |= LYD_VAL_MAND;
            }
        } else if (node->schema->nodetype & LYS_LIST) {
            if (((struct lys_node_list *)node->schema)->max) {
                node->parent->validity |= LYD_VAL_MAND;
            }
        }
    }
}

int
lyv_multicases(struct lyd_node *node, struct lys_node *schemanode, struct lyd_node *first_sibling,
               int autodelete, struct lyd_node *nodel)
{
    struct lys_node *sparent, *schoice, *scase, *saux;
    struct lyd_node *next, *iter;
    assert(node || schemanode);

    if (!schemanode) {
        schemanode = node->schema;
    }

    sparent = lys_parent(schemanode);
    if (!sparent || !(sparent->nodetype & (LYS_CHOICE | LYS_CASE))) {
        /* node is not under any choice */
        return EXIT_SUCCESS;
    } else if (!first_sibling) {
        /* nothing to check */
        return EXIT_SUCCESS;
    }

    /* remember which case to skip in which choice */
    if (sparent->nodetype == LYS_CHOICE) {
        schoice = sparent;
        scase = schemanode;
    } else {
        schoice = lys_parent(sparent);
        scase = sparent;
    }

autodelete:
    /* remove all nodes from other cases than 'sparent' */
    LY_TREE_FOR_SAFE(first_sibling, next, iter) {
        if (schemanode == iter->schema) {
            continue;
        }

        sparent = lys_parent(iter->schema);
        if (sparent && ((sparent->nodetype == LYS_CHOICE && sparent == schoice) /* another implicit case */
                || (sparent->nodetype == LYS_CASE && sparent != scase && lys_parent(sparent) == schoice)) /* another case */
                ) {
            if (autodelete) {
                if (iter == nodel) {
                    LOGVAL(LYE_MCASEDATA, LY_VLOG_LYD, iter, schoice->name);
                    return 2;
                }
                if (iter == first_sibling) {
                    first_sibling = next;
                }
                lyd_free(iter);
            } else {
                LOGVAL(LYE_MCASEDATA, node ? LY_VLOG_LYD : LY_VLOG_NONE, node, schoice->name);
                return 1;
            }
        }
    }

    if (first_sibling && (saux = lys_parent(schoice)) && (saux->nodetype & LYS_CASE)) {
        /* go recursively in case of nested choices */
        schoice = lys_parent(saux);
        scase = saux;
        goto autodelete;
    }

    return EXIT_SUCCESS;
}

API int
lyd_insert(struct lyd_node *parent, struct lyd_node *node)
{
    struct lys_node *sparent;
    struct lyd_node *iter;
    int invalid = 0;

    if (!node || !parent) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    /* check placing the node to the appropriate place according to the schema */
    for (sparent = lys_parent(node->schema);
         sparent && !(sparent->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_RPC | LYS_ACTION | LYS_NOTIF));
         sparent = lys_parent(sparent));
    if (sparent != parent->schema) {
        return EXIT_FAILURE;
    }

    if (node->parent != parent || (invalid = lyp_is_rpc_action(node->schema))) {
        /* it is not just moving under a parent node or it is in an RPC where
         * nodes order matters, so the validation will be necessary */
        invalid++;
    }

    if (node->parent || node->prev->next) {
        lyd_unlink_internal(node, invalid);
    }

    if (invalid == 1) {
        /* auto delete nodes from other cases, if any */
        lyv_multicases(node, NULL, parent->child, 1, NULL);
    }

    if (!parent->child) {
        /* add as the only child of the parent */
        parent->child = node;
    } else {
        /* add as the last child of the parent */
        parent->child->prev->next = node;
        node->prev = parent->child->prev;
        for (iter = node; iter->next; iter = iter->next);
        parent->child->prev = iter;
    }

    LY_TREE_FOR(node, iter) {
        iter->parent = parent;
        if (invalid) {
            lyd_insert_setinvalid(iter);
        }
    }

    return EXIT_SUCCESS;
}

static int
lyd_insert_sibling(struct lyd_node *sibling, struct lyd_node *node, int before)
{
    struct lys_node *par1, *par2;
    struct lyd_node *iter,*start = NULL;
    int invalid = 0;

    if (sibling == node) {
        return EXIT_SUCCESS;
    }

    /* check placing the node to the appropriate place according to the schema */
    for (par1 = lys_parent(sibling->schema);
         par1 && !(par1->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_INPUT | LYS_OUTPUT | LYS_ACTION | LYS_NOTIF));
         par1 = lys_parent(par1));
    for (par2 = lys_parent(node->schema);
         par2 && !(par2->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_INPUT | LYS_OUTPUT | LYS_ACTION | LYS_NOTIF));
         par2 = lys_parent(par2));
    if (par1 != par2) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    if (node->parent != sibling->parent || (invalid = lyp_is_rpc_action(node->schema)) || !node->parent) {
        /* a) it is not just moving under a parent node (invalid = 1) or
         * b) it is in an RPC where nodes order matters (invalid = 2) or
         * c) it is top-level where we don't know if it is the same tree (invalid = 1),
         * so the validation will be necessary */
        if (!node->parent && !invalid) {
            /* c) search in siblings */
            for (iter = node->prev; iter != node; iter = iter->prev) {
                if (iter == sibling) {
                    break;
                }
            }
            if (iter == node) {
                /* node and siblings are not currently in the same data tree */
                invalid++;
            }
        } else { /* a) and b) */
            invalid++;
        }
    }

    if (invalid == 1) {
        /* auto delete nodes from other cases */

        /* find first sibling node */
        if (sibling->parent) {
            start = sibling->parent->child;
        } else {
            for (start = sibling; start->prev->next; start = start->prev);
        }

        if (lyv_multicases(node, NULL, start, 1, sibling) == 2) {
            LOGVAL(LYE_SPEC, LY_VLOG_LYD, sibling, "Insert request refers node (%s) that is going to be auto-deleted.",
                   ly_errpath());
            return EXIT_FAILURE;
        }
        /* start could be autodeleted, so we cannot use it */
        start = NULL;
    }

    if (node->parent || node->next || node->prev->next) {
        lyd_unlink_internal(node, invalid);
    }

    node->parent = sibling->parent;
    if (invalid) {
        lyd_insert_setinvalid(node);
    }

    if (before) {
        if (sibling->prev->next) {
            /* adding into the list */
            sibling->prev->next = node;
        } else if (sibling->parent) {
            /* at the beginning */
            sibling->parent->child = node;
        }
        node->prev = sibling->prev;
        sibling->prev = node;
        node->next = sibling;
    } else {
        if (sibling->next) {
            /* adding into a middle - fix the prev pointer of the node after inserted nodes */
            node->next = sibling->next;
            sibling->next->prev = node;
        } else {
            /* at the end - fix the prev pointer of the first node */
            if (sibling->parent) {
                sibling->parent->child->prev = node;
            } else {
                for (start = sibling; start->prev->next; start = start->prev);
                start->prev = node;
            }
        }
        sibling->next = node;
        node->prev = sibling;
    }

    return EXIT_SUCCESS;
}

API int
lyd_insert_before(struct lyd_node *sibling, struct lyd_node *node)
{
    if (!node || !sibling || lyd_insert_sibling(sibling, node, 1)) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

API int
lyd_insert_after(struct lyd_node *sibling, struct lyd_node *node)
{
    if (!node || !sibling || lyd_insert_sibling(sibling, node, 0)) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static uint32_t
lys_module_pos(struct lys_module *module)
{
    int i;
    uint32_t pos = 1;

    for (i = 0; i < module->ctx->models.used; ++i) {
        if (module->ctx->models.list[i] == module) {
            return pos;
        }
        ++pos;
    }

    LOGINT;
    return 0;
}

static int
lys_module_node_pos_r(struct lys_node *first_sibling, struct lys_node *target, uint32_t *pos)
{
    const struct lys_node *next = NULL;

    /* the schema nodes are actually from data, lys_getnext skips non-data schema nodes for us */
    while ((next = lys_getnext(next, lys_parent(first_sibling), lys_node_module(first_sibling), 0))) {
        ++(*pos);
        if (target == next) {
            return 0;
        }
    }

    LOGINT;
    return 1;
}

static int
lyd_node_pos_cmp(const void *item1, const void *item2)
{
    uint32_t mpos1, mpos2;
    struct lyd_node_pos *np1, *np2;

    np1 = (struct lyd_node_pos *)item1;
    np2 = (struct lyd_node_pos *)item2;

    /* different modules? */
    if (lys_node_module(np1->node->schema) != lys_node_module(np2->node->schema)) {
        mpos1 = lys_module_pos(lys_node_module(np1->node->schema));
        mpos2 = lys_module_pos(lys_node_module(np2->node->schema));
        /* if lys_module_pos failed, there is nothing we can do anyway,
         * at least internal error will be printed */

        if (mpos1 > mpos2) {
            return 1;
        } else {
            return -1;
        }
    }

    if (np1->pos > np2->pos) {
        return 1;
    } else if (np1->pos < np2->pos) {
        return -1;
    }
    return 0;
}

API int
lyd_schema_sort(struct lyd_node *sibling, int recursive)
{
    uint32_t len, i;
    struct lyd_node *node;
    struct lys_node *first_ssibling;
    struct lyd_node_pos *array;

    if (!sibling) {
        ly_errno = LY_EINVAL;
        return -1;
    }

    /* something actually to sort */
    if (sibling->prev != sibling) {

        /* find the beginning */
        if (sibling->parent) {
            sibling = sibling->parent->child;
        } else {
            while (sibling->prev->next) {
                sibling = sibling->prev;
            }
        }

        /* find the data node schema parent */
        first_ssibling = sibling->schema;
        while (lys_parent(first_ssibling)
                && lys_parent(first_ssibling)->nodetype & (LYS_CHOICE | LYS_CASE | LYS_USES)) {
            first_ssibling = lys_parent(first_ssibling);
        }
        /* find the beginning */
        if (first_ssibling->parent) {
            first_ssibling = first_ssibling->parent->child;
        } else {
            while (first_ssibling->prev->next) {
                first_ssibling = first_ssibling->prev;
            }
        }

        /* count siblings */
        len = 0;
        for (node = sibling; node; node = node->next) {
            ++len;
        }

        array = malloc(len * sizeof *array);
        if (!array) {
            LOGMEM;
            return -1;
        }

        /* fill arrays with positions and corresponding nodes */
        for (i = 0, node = sibling; i < len; ++i, node = node->next) {
            array[i].pos = 0;
            if (lys_module_node_pos_r(first_ssibling, node->schema, &array[i].pos)) {
                free(array);
                return -1;
            }

            array[i].node = node;
        }

        /* sort the arrays */
        qsort(array, len, sizeof *array, lyd_node_pos_cmp);

        /* adjust siblings based on the sorted array */
        for (i = 0; i < len; ++i) {
            /* parent child */
            if (i == 0) {
                /* adjust sibling so that it still points to the beginning */
                sibling = array[i].node;
                if (array[i].node->parent) {
                    array[i].node->parent->child = array[i].node;
                }
            }

            /* prev */
            if (i > 0) {
                array[i].node->prev = array[i - 1].node;
            } else {
                array[i].node->prev = array[len - 1].node;
            }

            /* next */
            if (i < len - 1) {
                array[i].node->next = array[i + 1].node;
            } else {
                array[i].node->next = NULL;
            }
        }
        free(array);
    }

    /* sort all the children recursively */
    if (recursive) {
        LY_TREE_FOR(sibling, node) {
            if ((node->schema->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_RPC | LYS_ACTION | LYS_NOTIF))
                    && lyd_schema_sort(node->child, recursive)) {
                return -1;
            }
        }
    }

    return EXIT_SUCCESS;
}

API int
lyd_validate_leafref(struct lyd_node_leaf_list *leafref)
{
    if (!leafref || leafref->value_type != LY_TYPE_LEAFREF) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    if (leafref->value.leafref) {
        /* nothing to do */
        return EXIT_SUCCESS;
    }

    return resolve_unres_data_item((struct lyd_node *)leafref, UNRES_LEAFREF);
}

API int
lyd_validate(struct lyd_node **node, int options, ...)
{
    struct lyd_node *root, *next1, *next2, *iter, *action = NULL, *to_free = NULL;
    struct ly_ctx *ctx = NULL;
    int ret = EXIT_FAILURE, ap_flag = 0, i;
    va_list ap;
    struct unres_data *unres = NULL;
    struct ly_set *set;

    ly_errno = LY_SUCCESS;

    if (!node) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    unres = calloc(1, sizeof *unres);
    if (!unres) {
        LOGMEM;
        return EXIT_FAILURE;
    }

    if (!(*node)) {
        /* get context with schemas from the variable arguments */
        va_start(ap, options);
        ap_flag = 1;
        ctx = va_arg(ap, struct ly_ctx *);
        if (!ctx) {
            LOGERR(LY_EINVAL, "%s: Invalid variable argument.", __func__);
            goto error;
        }

        /* LYD_OPT_NOSIBLINGS has no meanings here */
        options &= ~LYD_OPT_NOSIBLINGS;
    } else {
        if (options & LYD_OPT_NOSIBLINGS) {
            /* ctx is NULL */
        } else {
            ctx = (*node)->schema->module->ctx;

            /* check that the node is the first sibling */
            while ((*node)->prev->next) {
                *node = (*node)->prev;
            }
        }
    }

    if ((options & LYD_OPT_RPC) && *node && ((*node)->schema->nodetype != LYS_RPC)) {
        options &= ~LYD_OPT_RPC;
        options |= LYD_OPT_ACTION;
    }

    if (lyd_check_topmandatory(*node, ctx, options)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(*node, next1, root) {
        LY_TREE_DFS_BEGIN(root, next2, iter) {
            if (to_free) {
                lyd_free(to_free);
                to_free = NULL;
            }

            if ((options & LYD_OPT_ACTION) && (iter->schema->nodetype == LYS_ACTION)) {
                options &= ~LYD_OPT_ACTION;
                options |= LYD_OPT_RPC;
                action = iter;
            }

            if (lyv_data_context(iter, options, unres)) {
                goto error;
            }
            if (lyv_data_content(iter, options, unres)) {
                if (ly_errno) {
                    goto error;
                } else {
                    /* safe deferred removal */
                    to_free = iter;
                    next2 = NULL;
                    goto nextsiblings;
                }
            }

            /* validation successful */
            if (iter->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) {
                iter->validity &= LYD_VAL_UNIQUE;
            } else {
                iter->validity = LYD_VAL_OK;
            }

            /* where go next? - modified LY_TREE_DFS_END */
            if (iter->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML)) {
                next2 = NULL;
            } else {
                next2 = iter->child;

                /* if we have empty non-presence container, we can remove it */
                if (!next2 && !(options & LYD_OPT_KEEPEMPTYCONT) && (iter->schema->nodetype == LYS_CONTAINER)
                        && !((struct lys_node_container *)iter->schema)->presence) {
                    lyd_free(to_free);
                    to_free = iter;
                }
            }
nextsiblings:
            if (!next2) {
                /* no children */
                if (iter == root) {
                    /* we are done */
                    break;
                }
                /* try siblings */
                next2 = iter->next;
            }
            while (!next2) {
                iter = iter->parent;

                /* if we have empty non-presence container, we can remove it */
                if (to_free && !(options & LYD_OPT_KEEPEMPTYCONT) && !to_free->next && to_free->prev == to_free &&
                        iter->schema->nodetype == LYS_CONTAINER &&
                        !((struct lys_node_container *)iter->schema)->presence) {
                    to_free = iter;
                } else {
                    lyd_free(to_free);
                    to_free = NULL;
                }

                /* parent is already processed, go to its sibling */
                if (iter->parent == root->parent) {
                    /* we are done */
                    break;
                }
                next2 = iter->next;
            } /* end of modified LY_TREE_DFS_END */
        }

        if (to_free) {
            if ((*node) == to_free) {
                /* we shouldn't be here */
                assert(0);
            }
            lyd_free(to_free);
            to_free = NULL;
        }

        if (options & LYD_OPT_NOSIBLINGS) {
            break;
        }

    }

    /* check for uniquness of top-level lists/leaflists because
     * only the inner instances were tested in lyv_data_content() */
    set = ly_set_new();
    LY_TREE_FOR(*node, root) {
        if (!(root->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)) || !(root->validity & LYD_VAL_UNIQUE)) {
            continue;
        }

        /* check each list/leaflist only once */
        i = set->number;
        if (ly_set_add(set, root->schema, 0) != i) {
            /* already checked */
            continue;
        }

        if (lyv_data_unique(root, *node)) {
            ly_set_free(set);
            goto error;
        }
    }
    ly_set_free(set);

    /* add default values if needed */
    if (action) {
        /* it will not get deleted */
        if (lyd_defaults_add_unres(&action, options, ctx, unres)) {
            goto error;
        }
    } else if (lyd_defaults_add_unres(node, options, ctx, unres)) {
        goto error;
    }

    ret = EXIT_SUCCESS;

error:
    if (ap_flag) {
        va_end(ap);
    }
    if (unres) {
        free(unres->node);
        free(unres->type);
        free(unres);
    }

    return ret;
}

/* create an attribute copy */
static struct lyd_attr *
lyd_dup_attr(struct ly_ctx *ctx, struct lyd_node *parent, struct lyd_attr *attr)
{
    struct lyd_attr *ret;

    /* allocate new attr */
    if (!parent->attr) {
        parent->attr = malloc(sizeof *parent->attr);
        ret = parent->attr;
    } else {
        for (ret = parent->attr; ret->next; ret = ret->next);
        ret->next = malloc(sizeof *ret);
        ret = ret->next;
    }
    if (!ret) {
        LOGMEM;
        return NULL;
    }

    /* fill new attr except */
    ret->next = NULL;
    ret->module = attr->module;
    ret->name = lydict_insert(ctx, attr->name, 0);
    ret->value = lydict_insert(ctx, attr->value, 0);

    return ret;
}

static int
lyd_unlink_internal(struct lyd_node *node, int permanent)
{
    struct lyd_node *iter, *next;
    struct ly_set *set, *data;
    unsigned int i, j;

    if (!node) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    if (permanent) {
        /* fix leafrefs */
        LY_TREE_DFS_BEGIN(node, next, iter) {
            /* the node is target of a leafref */
            if ((iter->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) && iter->schema->child) {
                set = (struct ly_set *)iter->schema->child;
                for (i = 0; i < set->number; i++) {
                    data = lyd_get_node2(iter, set->set.s[i]);
                    if (data) {
                        for (j = 0; j < data->number; j++) {
                            if (((struct lyd_node_leaf_list *)data->set.d[j])->value.leafref == iter) {
                                /* remove reference to the node we are going to replace */
                                ((struct lyd_node_leaf_list *)data->set.d[j])->value.leafref = NULL;
                            }
                        }
                        ly_set_free(data);
                    } else {
                        return EXIT_FAILURE;
                    }
                }
            }
            LY_TREE_DFS_END(node, next, iter)
        }

        /* invalidate parent to make sure it will be checked in future validation */
        if (node->parent) {
            node->parent->validity = LYD_VAL_MAND;
        }
    }

    /* unlink from siblings */
    if (node->prev->next) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        /* unlinking the last node */
        if (node->parent) {
            iter = node->parent->child;
        } else {
            iter = node->prev;
            while (iter->prev != node) {
                iter = iter->prev;
            }
        }
        /* update the "last" pointer from the first node */
        iter->prev = node->prev;
    }

    /* unlink from parent */
    if (node->parent) {
        if (node->parent->child == node) {
            /* the node is the first child */
            node->parent->child = node->next;
        }
        node->parent = NULL;
    }

    node->next = NULL;
    node->prev = node;

    return EXIT_SUCCESS;
}

API int
lyd_unlink(struct lyd_node *node)
{
    return lyd_unlink_internal(node, 1);
}

API struct lyd_node *
lyd_dup(const struct lyd_node *node, int recursive)
{
    const struct lyd_node *next, *elem;
    struct lyd_node *ret, *parent, *new_node;
    struct lyd_attr *attr;
    struct lyd_node_leaf_list *new_leaf;
    struct lyd_node_anyxml *new_axml;
    struct lys_type *type;

    if (!node) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    ret = NULL;
    parent = NULL;

    /* LY_TREE_DFS */
    for (elem = next = node; elem; elem = next) {

        /* fill specific part */
        switch (elem->schema->nodetype) {
        case LYS_LEAF:
        case LYS_LEAFLIST:
            new_leaf = malloc(sizeof *new_leaf);
            new_node = (struct lyd_node *)new_leaf;
            if (!new_node) {
                LOGMEM;
                return NULL;
            }

            new_leaf->value_str = lydict_insert(elem->schema->module->ctx,
                                                ((struct lyd_node_leaf_list *)elem)->value_str, 0);
            new_leaf->value_type = ((struct lyd_node_leaf_list *)elem)->value_type;

            /* value_str pointer is shared in these cases */
            if ((new_leaf->value_type == LY_TYPE_BINARY) || (new_leaf->value_type == LY_TYPE_STRING)) {
                new_leaf->value.string = new_leaf->value_str;
            } else {
                new_leaf->value = ((struct lyd_node_leaf_list *)elem)->value;
            }

            /* bits type must be treated specially */
            if (new_leaf->value_type == LY_TYPE_BITS) {
                for (type = &((struct lys_node_leaf *)elem->schema)->type; type->der->module; type = &type->der->type) {
                    if (type->base != LY_TYPE_BITS) {
                        LOGINT;
                        lyd_free(new_node);
                        lyd_free(ret);
                        return NULL;
                    }
                }

                new_leaf->value.bit = malloc(type->info.bits.count * sizeof *new_leaf->value.bit);
                if (!new_leaf->value.bit) {
                    LOGMEM;
                    lyd_free(new_node);
                    lyd_free(ret);
                    return NULL;
                }
                memcpy(new_leaf->value.bit, ((struct lyd_node_leaf_list *)elem)->value.bit,
                       type->info.bits.count * sizeof *new_leaf->value.bit);
            }
            break;
        case LYS_ANYXML:
            new_axml = malloc(sizeof *new_axml);
            new_node = (struct lyd_node *)new_axml;
            if (!new_node) {
                LOGMEM;
                return NULL;
            }

            if (((struct lyd_node_anyxml *)elem)->xml_struct) {
                new_axml->xml_struct = 1;
                new_axml->value.xml = lyxml_dup_elem(elem->schema->module->ctx,
                                                     ((struct lyd_node_anyxml *)elem)->value.xml, NULL, 1);
            } else {
                new_axml->xml_struct = 0;
                new_axml->value.str = lydict_insert(elem->schema->module->ctx,
                                                    ((struct lyd_node_anyxml *)elem)->value.str, 0);
            }
            break;
        case LYS_CONTAINER:
        case LYS_LIST:
        case LYS_NOTIF:
        case LYS_RPC:
        case LYS_ACTION:
            new_node = malloc(sizeof *new_node);
            if (!new_node) {
                LOGMEM;
                return NULL;
            }
            new_node->child = NULL;
            break;
        default:
            lyd_free(ret);
            LOGINT;
            return NULL;
        }

        /* fill common part */
        new_node->schema = elem->schema;
        new_node->attr = NULL;
        LY_TREE_FOR(elem->attr, attr) {
            lyd_dup_attr(elem->schema->module->ctx, new_node, attr);
        }
        new_node->next = NULL;
        new_node->prev = new_node;
        new_node->parent = NULL;
        new_node->validity = LYD_VAL_NOT;
        new_node->dflt = elem->dflt;
        new_node->when_status = elem->when_status & LYD_WHEN;

        if (!ret) {
            ret = new_node;
        }
        if (parent) {
            if (lyd_insert(parent, new_node)) {
                lyd_free(ret);
                LOGINT;
                return NULL;
            }
        }

        if (!recursive) {
            break;
        }

        /* LY_TREE_DFS_END */
        /* select element for the next run - children first */
        next = elem->child;
        /* child exception for lyd_node_leaf and lyd_node_leaflist */
        if (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML)) {
            next = NULL;
        }
        if (!next) {
            if (elem->parent == node->parent) {
                break;
            }
            /* no children, so try siblings */
            next = elem->next;
        } else {
            parent = new_node;
        }
        while (!next) {
            /* no siblings, go back through parents */
            elem = elem->parent;
            if (elem->parent == node->parent) {
                break;
            }
            if (!parent) {
                lyd_free(ret);
                LOGINT;
                return NULL;
            }
            parent = parent->parent;
            /* parent is already processed, go to its sibling */
            next = elem->next;
        }
    }

    return ret;
}

API void
lyd_free_attr(struct ly_ctx *ctx, struct lyd_node *parent, struct lyd_attr *attr, int recursive)
{
    struct lyd_attr *iter;

    if (!ctx || !attr) {
        return;
    }

    if (parent) {
        if (parent->attr == attr) {
            if (recursive) {
                parent->attr = NULL;
            } else {
                parent->attr = attr->next;
            }
        } else {
            for (iter = parent->attr; iter->next != attr; iter = iter->next);
            if (iter->next) {
                if (recursive) {
                    iter->next = NULL;
                } else {
                    iter->next = attr->next;
                }
            }
        }
    }

    if (!recursive) {
        attr->next = NULL;
    }

    for(iter = attr; iter; ) {
        attr = iter;
        iter = iter->next;

        lydict_remove(ctx, attr->name);
        lydict_remove(ctx, attr->value);
        free(attr);
    }
}

const struct lyd_node *
lyd_attr_parent(const struct lyd_node *root, struct lyd_attr *attr)
{
    const struct lyd_node *next, *elem;
    struct lyd_attr *node_attr;

    LY_TREE_DFS_BEGIN(root, next, elem) {
        for (node_attr = elem->attr; node_attr; node_attr = node_attr->next) {
            if (node_attr == attr) {
                return elem;
            }
        }
        LY_TREE_DFS_END(root, next, elem)
    }

    return NULL;
}

API struct lyd_attr *
lyd_insert_attr(struct lyd_node *parent, const struct lys_module *mod, const char *name, const char *value)
{
    struct lyd_attr *a, *iter;
    struct ly_ctx *ctx;
    const struct lys_module *module;
    const char *p;
    char *aux;

    if (!parent || !name || !value) {
        return NULL;
    }
    ctx = parent->schema->module->ctx;

    if ((p = strchr(name, ':'))) {
        /* search for the namespace */
        aux = strndup(name, p - name);
        if (!aux) {
            LOGMEM;
            return NULL;
        }
        module = ly_ctx_get_module(ctx, aux, NULL);
        free(aux);
        name = p + 1;

        if (!module) {
            /* module not found */
            LOGERR(LY_EINVAL, "Attribute prefix does not match any schema in the context.");
            return NULL;
        }
    } else if (mod) {
        module = mod;
    } else {
        /* no prefix -> module is the same as for the parent */
        module = parent->schema->module;
    }

    a = malloc(sizeof *a);
    if (!a) {
        LOGMEM;
        return NULL;
    }
    a->module = (struct lys_module *)module;
    a->next = NULL;
    a->name = lydict_insert(ctx, name, 0);
    a->value = lydict_insert(ctx, value, 0);

    if (!parent->attr) {
        parent->attr = a;
    } else {
        for (iter = parent->attr; iter->next; iter = iter->next);
        iter->next = a;
    }

    return a;
}

API void
lyd_free(struct lyd_node *node)
{
    struct lyd_node *next, *iter;

    if (!node) {
        return;
    }

    if (!(node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML))) {
        /* free children */
        LY_TREE_FOR_SAFE(node->child, next, iter) {
            lyd_free(iter);
        }
    } else if (node->schema->nodetype == LYS_ANYXML) {
        if (((struct lyd_node_anyxml *)node)->xml_struct) {
            lyxml_free_withsiblings(node->schema->module->ctx, ((struct lyd_node_anyxml *)node)->value.xml);
        } else {
            lydict_remove(node->schema->module->ctx, ((struct lyd_node_anyxml *)node)->value.str);
        }
    } else { /* LYS_LEAF | LYS_LEAFLIST */
        /* free value */
        switch (((struct lyd_node_leaf_list *)node)->value_type) {
        case LY_TYPE_BITS:
            if (((struct lyd_node_leaf_list *)node)->value.bit) {
                free(((struct lyd_node_leaf_list *)node)->value.bit);
            }
            /* fallthrough */
        default:
            lydict_remove(node->schema->module->ctx, ((struct lyd_node_leaf_list *)node)->value_str);
            break;
        }
    }

    lyd_unlink(node);
    lyd_free_attr(node->schema->module->ctx, node, node->attr, 1);
    free(node);
}

API void
lyd_free_withsiblings(struct lyd_node *node)
{
    struct lyd_node *iter, *aux;

    if (!node) {
        return;
    }

    /* optimization - avoid freeing (unlinking) the last node of the siblings list */
    /* so, first, free the node's predecessors to the beginning of the list ... */
    for(iter = node->prev; iter->next; iter = aux) {
        aux = iter->prev;
        lyd_free(iter);
    }
    /* ... then, the node is the first in the siblings list, so free them all */
    LY_TREE_FOR_SAFE(node, aux, iter) {
        lyd_free(iter);
    }
}

/**
 * Expectations:
 * - list exists in data tree
 * - the leaf (defined by the unique_expr) is not instantiated under the list
 *
 * NULL + ly_errno - error
 * NULL - no default value
 * pointer to the default value
 */
const char *
lyd_get_unique_default(const char* unique_expr, struct lyd_node *list)
{
    const struct lys_node *parent;
    const struct lys_node_leaf *sleaf = NULL;
    struct lys_tpdf *tpdf;
    struct lyd_node *last;
    const char *dflt = NULL;
    struct ly_set *s, *r;
    unsigned int i;

    assert(unique_expr);

    if (resolve_descendant_schema_nodeid(unique_expr, list->schema->child, LYS_LEAF, 1, 1, &parent)) {
        /* error, but unique expression was checked when the schema was parsed */
        return NULL;
    }

    sleaf = (struct lys_node_leaf *)parent;
    if (sleaf->dflt) {
        /* leaf has a default value */
        dflt = sleaf->dflt;
    } else if (!(sleaf->flags & LYS_MAND_TRUE)) {
        /* get the default value from the type */
        for (tpdf = sleaf->type.der; tpdf && !dflt; tpdf = tpdf->type.der) {
            dflt = tpdf->dflt;
        }
    }

    if (!dflt) {
        return NULL;
    }

    /* it has default value, but check if it can appear in the data tree under the list */
    s = ly_set_new();
    for (parent = lys_parent((struct lys_node *)sleaf); parent != list->schema; parent = lys_parent(parent)) {
        if (!(parent->nodetype & (LYS_CONTAINER | LYS_CASE | LYS_CHOICE | LYS_USES))) {
            /* This should be already detected when parsing schema */
            LOGINT;
            ly_set_free(s);
            return NULL;
        }
        ly_set_add(s, (void *)parent, LY_SET_OPT_USEASLIST);
    }
    ly_vlog_hide(1);
    for (i = 0, last = list; i < s->number; i++) {
        parent = s->set.s[i]; /* shortcut */

        switch (parent->nodetype) {
        case LYS_CONTAINER:
            if (last) {
                /* find instance in the data */
                r = lyd_get_node(last, parent->name);
                if (!r || r->number > 1) {
                    ly_set_free(r);
                    LOGINT;
                    dflt = NULL;
                    goto end;
                }
                if (r->number) {
                    last = r->set.d[0];
                } else {
                    last = NULL;
                }
                ly_set_free(r);
            }
            if (((struct lys_node_container *)parent)->presence) {
                /* not-instantiated presence container on path */
                dflt = NULL;
                goto end;
            }
            break;
        case LYS_CHOICE :
            /* check presence of another case */
            if (!last) {
                continue;
            }

            /* remember the case to be searched in choice by lyv_multicases() */
            if (i + 1 == s->number) {
                parent = (struct lys_node *)sleaf;
            } else if (s->set.s[i + 1]->nodetype == LYS_CASE && (i + 2 < s->number) &&
                    s->set.s[i + 2]->nodetype == LYS_CHOICE) {
                /* nested choices are covered by lyv_multicases, we just have to pass
                 * the lowest choice */
                i++;
                continue;
            } else {
                parent = s->set.s[i + 1];
            }
            if (lyv_multicases(NULL, (struct lys_node *)parent, last->child, 0, NULL)) {
                /* another case is present */
                ly_errno = LY_SUCCESS;
                dflt = NULL;
                goto end;
            }
            break;
        default:
            /* LYS_CASE, LYS_USES */
            continue;
        }
    }

end:
    ly_vlog_hide(0);
    ly_set_free(s);
    return dflt;
}

API char *
lyd_path(struct lyd_node *node)
{
    char *buf_backup = NULL, *buf = ly_buf(), *result = NULL;
    uint16_t index = LY_BUF_SIZE - 1;

    if (!node) {
        LOGERR(LY_EINVAL, "%s: NULL node parameter", __func__);
        return NULL;
    }

    /* backup the shared internal buffer */
    if (ly_buf_used && buf[0]) {
        buf_backup = strndup(buf, LY_BUF_SIZE - 1);
    }
    ly_buf_used++;

    /* build the path */
    buf[index] = '\0';
    ly_vlog_build_path_reverse(LY_VLOG_LYD, node, buf, &index);
    result = strdup(&buf[index]);

    /* restore the shared internal buffer */
    if (buf_backup) {
        strcpy(buf, buf_backup);
        free(buf_backup);
    }
    ly_buf_used--;

    return result;
}

static int
lyd_build_relative_data_path(const struct lyd_node *node, const char *schema_id, char *buf)
{
    const struct lys_node *snode, *schema;
    const char *end;
    int len = 0;

    schema = node->schema;

    while (1) {
        end = strchr(schema_id, '/');
        if (!end) {
            end = schema_id + strlen(schema_id);
        }

        snode = NULL;
        while ((snode = lys_getnext(snode, schema, NULL, LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE))) {
            if (!strncmp(snode->name, schema_id, end - schema_id) && !snode->name[end - schema_id]) {
                assert(snode->nodetype != LYS_LIST);
                if (!(snode->nodetype & (LYS_CHOICE | LYS_CASE))) {
                    len += sprintf(&buf[len], "%s%s", (len ? "/" : ""), snode->name);
                }
                /* shorthand case, skip it in schema */
                if (lys_parent(snode) && (lys_parent(snode)->nodetype == LYS_CHOICE) && (snode->nodetype != LYS_CASE)) {
                    schema_id = end + 1;
                    end = strchr(schema_id, '/');
                    if (!end) {
                        end = schema_id + strlen(schema_id);
                    }
                }
                schema = snode;
                break;
            }
        }
        if (!snode) {
            LOGINT;
            return -1;
        }

        if (!end[0]) {
            return len;
        }
        schema_id = end + 1;
    }

    LOGINT;
    return -1;
}

/*
 * actions (only for list):
 * -1 - compare keys and all uniques
 * 0  - compare only keys
 * n  - compare n-th unique
 */
int
lyd_list_equal(struct lyd_node *first, struct lyd_node *second, int action, int printval)
{
    struct lys_node_list *slist;
    const struct lys_node *snode = NULL;
    struct lyd_node *diter;
    const char *val1, *val2;
    char *path1, *path2, *uniq_str = ly_buf(), *buf_backup = NULL;
    uint16_t idx1, idx2, idx_uniq;
    int i, j;

    assert(first && (first->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)));
    assert(second && (second->schema->nodetype & (LYS_LIST | LYS_LEAFLIST)));
    assert(first->schema->nodetype == second->schema->nodetype);

    if (first->schema != second->schema) {
        return 0;
    }

    switch (first->schema->nodetype) {
    case LYS_LEAFLIST:
        /* compare values */
        if (ly_strequal(((struct lyd_node_leaf_list *)first)->value_str,
                        ((struct lyd_node_leaf_list *)second)->value_str, 1)) {
            if (printval) {
                LOGVAL(LYE_DUPLEAFLIST, LY_VLOG_LYD, second, second->schema->name,
                       ((struct lyd_node_leaf_list *)second)->value_str);
            }
            return 1;
        }
        return 0;
    case LYS_LIST:
        slist = (struct lys_node_list *)first->schema;

        /* compare unique leafs */
        if (action) {
            if (action > 0) {
                i = action - 1;
                if (i < slist->unique_size) {
                    goto uniquecheck;
                }
            }
            for (i = 0; i < slist->unique_size; i++) {
uniquecheck:
                for (j = 0; j < slist->unique[i].expr_size; j++) {
                    /* first */
                    diter = resolve_data_descendant_schema_nodeid(slist->unique[i].expr[j], first->child);
                    if (diter) {
                        val1 = ((struct lyd_node_leaf_list *)diter)->value_str;
                    } else {
                        /* use default value */
                        val1 = lyd_get_unique_default(slist->unique[i].expr[j], first);
                        if (ly_errno) {
                            return -1;
                        }
                    }

                    /* second */
                    diter = resolve_data_descendant_schema_nodeid(slist->unique[i].expr[j], second->child);
                    if (diter) {
                        val2 = ((struct lyd_node_leaf_list *)diter)->value_str;
                    } else {
                        /* use default value */
                        val2 = lyd_get_unique_default(slist->unique[i].expr[j], second);
                        if (ly_errno) {
                            return -1;
                        }
                    }

                    if (!val1 || !val2 || !ly_strequal(val1, val2, 1)) {
                        /* values differ */
                        break;
                    }
                }
                if (j && (j == slist->unique[i].expr_size)) {
                    /* all unique leafs are the same in this set, create this nice error */
                    if (!printval) {
                        return 1;
                    }

                    path1 = malloc(LY_BUF_SIZE);
                    path2 = malloc(LY_BUF_SIZE);
                    if (!path1 || !path2) {
                        LOGMEM;
                        return -1;
                    }
                    idx1 = idx2 = LY_BUF_SIZE - 1;
                    path1[idx1] = '\0';
                    path2[idx2] = '\0';
                    ly_vlog_build_path_reverse(LY_VLOG_LYD, first, path1, &idx1);
                    ly_vlog_build_path_reverse(LY_VLOG_LYD, second, path2, &idx2);

                    /* use internal buffer to rebuild the unique string */
                    if (ly_buf_used && uniq_str[0]) {
                        buf_backup = strndup(uniq_str, LY_BUF_SIZE - 1);
                    }
                    ly_buf_used++;
                    idx_uniq = 0;

                    for (j = 0; j < slist->unique[i].expr_size; ++j) {
                        if (j) {
                            uniq_str[idx_uniq++] = ' ';
                        }
                        idx_uniq += lyd_build_relative_data_path(first, slist->unique[i].expr[j], &uniq_str[idx_uniq]);
                    }

                    LOGVAL(LYE_NOUNIQ, LY_VLOG_LYD, second, uniq_str, &path1[idx1], &path2[idx2]);
                    free(path1);
                    free(path2);
                    if (buf_backup) {
                        strcpy(uniq_str, buf_backup);
                        free(buf_backup);
                    }
                    ly_buf_used--;
                    return 1;
                }

                if (action > 0) {
                    /* done */
                    return 0;
                }
            }
        }

        /* compare keys */
        for (i = 0; i < slist->keys_size; i++) {
            snode = (struct lys_node *)slist->keys[i];
            val1 = val2 = NULL;
            LY_TREE_FOR(first->child, diter) {
                if (diter->schema == snode) {
                    val1 = ((struct lyd_node_leaf_list *)diter)->value_str;
                    break;
                }
            }
            LY_TREE_FOR(second->child, diter) {
                if (diter->schema == snode) {
                    val2 = ((struct lyd_node_leaf_list *)diter)->value_str;
                    break;
                }
            }
            if (!ly_strequal(val1, val2, 1)) {
                return 0;
            }
        }

        if (printval) {
            LOGVAL(LYE_DUPLIST, LY_VLOG_LYD, second, second->schema->name);
        }
        return 1;
    default:
        LOGINT;
        return -1;
    }
}

API struct ly_set *
lyd_get_node(const struct lyd_node *data, const char *expr)
{
    struct lyxp_set xp_set;
    struct ly_set *set;
    uint16_t i;

    if (!data || !expr) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    memset(&xp_set, 0, sizeof xp_set);

    if (lyxp_eval(expr, data, &xp_set, 0) != EXIT_SUCCESS) {
        return NULL;
    }

    set = ly_set_new();
    if (!set) {
        LOGMEM;
        return NULL;
    }

    if (xp_set.type == LYXP_SET_NODE_SET) {
        for (i = 0; i < xp_set.used; ++i) {
            if (xp_set.val.nodes[i].type == LYXP_NODE_ELEM) {
                if (ly_set_add(set, xp_set.val.nodes[i].node, LY_SET_OPT_USEASLIST) < 0) {
                    ly_set_free(set);
                    set = NULL;
                    break;
                }
            }
        }
    }
    /* free xp_set content */
    lyxp_set_cast(&xp_set, LYXP_SET_EMPTY, data, 0);

    return set;
}

API struct ly_set *
lyd_get_node2(const struct lyd_node *data, const struct lys_node *schema)
{
    struct ly_set *ret, *ret_aux, *spath;
    const struct lys_node *siter;
    struct lyd_node *iter;
    unsigned int i, j;

    if (!data || !schema ||
            !(schema->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_ANYXML | LYS_NOTIF | LYS_RPC | LYS_ACTION))) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    ret = ly_set_new();
    spath = ly_set_new();
    if (!ret || !spath) {
        LOGMEM;
        goto error;
    }

    /* find data root */
    while (data->parent) {
        /* vertical move (up) */
        data = data->parent;
    }
    while (data->prev->next) {
        /* horizontal move (left) */
        data = data->prev;
    }

    /* build schema path */
    for (siter = schema; siter; ) {
        if (siter->nodetype == LYS_AUGMENT) {
            siter = ((struct lys_node_augment *)siter)->target;
            continue;
        } else if (siter->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_ANYXML | LYS_NOTIF | LYS_RPC | LYS_RPC)) {
            /* standard data node */
            ly_set_add(spath, (void*)siter, LY_SET_OPT_USEASLIST);

        } /* else skip the rest node types */
        siter = siter->parent;
    }
    if (!spath->number) {
        /* no valid path */
        goto error;
    }

    /* start searching */
    LY_TREE_FOR((struct lyd_node *)data, iter) {
        if (iter->schema == spath->set.s[spath->number - 1]) {
            ly_set_add(ret, iter, LY_SET_OPT_USEASLIST);
        }
    }
    for (i = spath->number - 1; i; i--) {
        if (!ret->number) {
            /* nothing found */
            break;
        }

        ret_aux = ly_set_new();
        if (!ret_aux) {
            LOGMEM;
            goto error;
        }
        for (j = 0; j < ret->number; j++) {
            LY_TREE_FOR(ret->set.d[j]->child, iter) {
                if (iter->schema == spath->set.s[i - 1]) {
                    ly_set_add(ret_aux, iter, LY_SET_OPT_USEASLIST);
                }
            }
        }
        ly_set_free(ret);
        ret = ret_aux;
    }

    ly_set_free(spath);
    return ret;

error:
    ly_set_free(ret);
    ly_set_free(spath);

    return NULL;
}

API struct lyd_node *
lyd_first_sibling(struct lyd_node *node)
{
    struct lyd_node *start;

    /* get the first sibling */
    if (node->parent) {
        start = node->parent->child;
    } else {
        for (start = node; start->prev->next; start = start->prev);
    }

    return start;
}

API struct ly_set *
ly_set_new(void)
{
    return calloc(1, sizeof(struct ly_set));
}

API void
ly_set_free(struct ly_set *set)
{
    if (!set) {
        return;
    }

    free(set->set.g);
    free(set);
}

API struct ly_set *
ly_set_dup(const struct ly_set *set)
{
    struct ly_set *new;

    if (!set) {
        return NULL;
    }

    new = malloc(sizeof *new);
    new->number = set->number;
    new->size = set->size;
    new->set.g = malloc(new->size * sizeof *(new->set.g));
    memcpy(new->set.g, set->set.g, new->size * sizeof *(new->set.g));

    return new;
}

API int
ly_set_add(struct ly_set *set, void *node, int options)
{
    unsigned int i;
    void **new;

    if (!set || !node) {
        ly_errno = LY_EINVAL;
        return -1;
    }

    if (!(options & LY_SET_OPT_USEASLIST)) {
        /* search for duplication */
        for (i = 0; i < set->number; i++) {
            if (set->set.g[i] == node) {
                /* already in set */
                return i;
            }
        }
    }

    if (set->size == set->number) {
        new = realloc(set->set.g, (set->size + 8) * sizeof *(set->set.g));
        if (!new) {
            LOGMEM;
            return -1;
        }
        set->size += 8;
        set->set.g = new;
    }

    set->set.g[set->number++] = node;

    return set->number - 1;
}

API int
ly_set_rm_index(struct ly_set *set, unsigned int index)
{
    if (!set || (index + 1) > set->number) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    if (index == set->number - 1) {
        /* removing last item in set */
        set->set.g[index] = NULL;
    } else {
        /* removing item somewhere in a middle, so put there the last item */
        set->set.g[index] = set->set.g[set->number - 1];
        set->set.g[set->number - 1] = NULL;
    }
    set->number--;

    return EXIT_SUCCESS;
}

API int
ly_set_rm(struct ly_set *set, void *node)
{
    unsigned int i;

    if (!set || !node) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    /* get index */
    for (i = 0; i < set->number; i++) {
        if (set->set.g[i] == node) {
            break;
        }
    }
    if (i == set->number) {
        /* node is not in set */
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    return ly_set_rm_index(set, i);
}

API int
ly_set_clean(struct ly_set *set)
{
    if (!set) {
        return EXIT_FAILURE;
    }

    free(set->set.g);
    set->size = 0;
    set->number = 0;
    set->set.g = NULL;

    return EXIT_SUCCESS;
}

API int
lyd_wd_cleanup(struct lyd_node **root, int options)
{
    struct lyd_node *wr, *next1, *next2, *iter, *to_free = NULL;

    if (!root) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }
    if (!(*root)) {
        /* nothing to do */
        return EXIT_SUCCESS;
    }

    LY_TREE_FOR_SAFE(*root, next1, wr) {
        LY_TREE_DFS_BEGIN(wr, next2, iter) {
            if (to_free) {
                lyd_free(to_free);
                to_free = NULL;
            }

            /* if we have leaf with default flag */
            if (iter->dflt) {
                /* if options have LYD_WD_EXPLICIT, remove only config nodes */
                if (!(options & LYD_WD_EXPLICIT) || (iter->schema->flags & LYS_CONFIG_W)) {
                    /* safe deferred removal */
                    to_free = iter;
                    next2 = NULL;
                    goto nextsiblings;
                } else {
                    /* remove default flag */
                    iter->dflt = 0;
                }
            }

            /* where go next? - modified LY_TREE_DFS_END */
            if (iter->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML)) {
                next2 = NULL;
            } else {
                next2 = iter->child;
            }
nextsiblings:
            if (!next2) {
                /* no children */
                if (iter == wr) {
                    /* we are done */
                    break;
                }
                /* try siblings */
                next2 = iter->next;
            }
            while (!next2) {
                iter = iter->parent;

                /* if we have empty non-presence container, we can remove it */
                if (to_free && !(options & LYD_OPT_KEEPEMPTYCONT) && !to_free->next && to_free->prev == to_free &&
                        iter->schema->nodetype == LYS_CONTAINER &&
                        !((struct lys_node_container *)iter->schema)->presence) {
                    to_free = iter;
                } else {
                    lyd_free(to_free);
                    to_free = NULL;
                }

                /* parent is already processed, go to its sibling */
                if (iter->parent == wr->parent) {
                    /* we are done */
                    break;
                }
                next2 = iter->next;

            } /* end of modified LY_TREE_DFS_END */
        }

        if (to_free) {
            if ((*root) == to_free) {
                (*root) = next1;
            }
            lyd_free(to_free);
            to_free = NULL;
        }
    }

    return EXIT_SUCCESS;
}

static int
lyd_wd_trim(struct lyd_node **root, int options)
{
    struct lyd_node *wr, *next1, *next2, *iter, *to_free = NULL;
    struct lys_node_leaf *leaf;
    const char *dflt;
    struct lys_tpdf *tpdf;

    LY_TREE_FOR_SAFE(*root, next1, wr) {
        LY_TREE_DFS_BEGIN(wr, next2, iter) {
            if (to_free) {
                lyd_free(to_free);
                to_free = NULL;
            }

            if (iter->schema->nodetype == LYS_LEAF) {
                leaf = (struct lys_node_leaf *)iter->schema;
                dflt = NULL;

                if (leaf->dflt) {
                    /* leaf has a default value */
                    dflt = leaf->dflt;
                } else {
                    /* get the default value from the type */
                    for (tpdf = leaf->type.der; tpdf && !dflt; tpdf = tpdf->type.der) {
                        dflt = tpdf->dflt;
                    }
                }
                if (dflt && ly_strequal(dflt, ((struct lyd_node_leaf_list * )iter)->value_str, 1)) {
                    if (options & LYD_WD_ALL_TAG) {
                        /* add tag */
                        iter->dflt = 1;
                    } else {
                        /* safe deferred removal */
                        to_free = iter;
                    }
                    next2 = NULL;
                    goto nextsiblings;
                }
            }

            /* where go next? - modified LY_TREE_DFS_END */
            if (iter->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML)) {
                next2 = NULL;
            } else {
                next2 = iter->child;
            }
nextsiblings:
            if (!next2) {
                /* no children */
                if (iter == wr) {
                    /* we are done */
                    break;
                }
                /* try siblings */
                next2 = iter->next;
            }
            while (!next2) {
                iter = iter->parent;

                /* if we have empty non-presence container, we can remove it */
                if (to_free && !(options & LYD_OPT_KEEPEMPTYCONT) && !to_free->next && to_free->prev == to_free &&
                        iter->schema->nodetype == LYS_CONTAINER &&
                        !((struct lys_node_container *)iter->schema)->presence) {
                    to_free = iter;
                } else {
                    lyd_free(to_free);
                    to_free = NULL;
                }

                /* parent is already processed, go to its sibling */
                if (iter->parent == wr->parent) {
                    /* we are done */
                    break;
                }
                next2 = iter->next;

            } /* end of modified LY_TREE_DFS_END */
        }

        if (to_free) {
            if ((*root) == to_free) {
                (*root) = next1;
            }
            lyd_free(to_free);
            to_free = NULL;
        }

        if (options & LYD_OPT_NOSIBLINGS) {
            break;
        }
    }

    return EXIT_SUCCESS;
}

/*
 * Returns choice's case that was instantiated in data tree
 * cases - set of all cases (from all choices) that were instantiated in data tree
 * choice - choice node to be examined
 */
static struct lys_node *
lyd_wd_get_inst_case(struct ly_set *cases, struct lys_node *choice)
{
    struct lys_node *siter, *cs;
    unsigned int i;

    if (!cases) {
        return NULL;
    }

    LY_TREE_FOR(choice->child, siter) {
        for (i = 0; i < cases->number; i++) {
            if (cases->set.s[i] == siter) {
                ly_set_rm_index(cases, i);
                return siter;
            } else {
                for (cs = lys_parent(cases->set.s[i]); cs && (cs->nodetype & (LYS_CHOICE | LYS_CASE)); cs = lys_parent(cs)) {
                    if (cs == siter) {
                        siter = cases->set.s[i];
                        ly_set_rm_index(cases, i);
                        return siter;
                    }
                }
            }
        }
    }

    return NULL;
}

static struct lyd_node *
lyd_wd_add_leaflist(struct ly_ctx *ctx, struct lyd_node *parent, struct lys_node_leaflist *llist,
                    const char *path, struct unres_data *unres, int options, int check_existence)
{
    struct lyd_node *ret = NULL, *iter, *last;
    struct lys_tpdf *tpdf;
    const char **dflt;
    uint8_t dflt_size, i;
    struct ly_set *nodeset;

    if (llist->module->version < 2) {
        /* default values on leaf-lists are allowed from YANG 1.1 */
        return NULL;
    } else if ((options & LYD_WD_MASK) == LYD_WD_EXPLICIT && (llist->flags & LYS_CONFIG_W)) {
        /* do not process config data in explicit mode */
        return NULL;
    } else if (lys_is_disabled((struct lys_node *)llist, 0)) {
        /* ignore disabled data */
        return NULL;
    }

    if (llist->dflt_size) {
        /* there are default values */
        dflt_size = llist->dflt_size;
        dflt = llist->dflt;
    } else if (!llist->min) {
        /* get the default value from the type */
        for (tpdf = llist->type.der; tpdf && !dflt; tpdf = tpdf->type.der) {
            dflt = &tpdf->dflt;
            dflt_size = 1;
        }
    }

    if (!dflt_size) {
        /* no default values to use */
        return NULL;
    }

    if (check_existence && parent) {
        /* check presence of any instance of the leaf-list */
        nodeset = lyd_get_node(parent, path);
        if (nodeset && nodeset->number) {
            ly_set_free(nodeset);
            return NULL;
        }
        ly_set_free(nodeset);
    }

    for (i = 0; i < dflt_size; i++) {
        last = lyd_new_path(parent, ctx, path, dflt[i], options & LYD_OPT_RPCREPLY ? LYD_PATH_OPT_OUTPUT : 0);
        if ((options & (LYD_WD_ALL_TAG | LYD_WD_IMPL_TAG)) && last) {
            /* remember the created nodes (if necessary) in unres */
            for (iter = last; ; iter = iter->child) {
                if ((!(options & LYD_OPT_TYPEMASK) || (options & LYD_OPT_CONFIG)) && (iter->when_status & LYD_WHEN)) {
                    if (unres_data_add(unres, (struct lyd_node *)iter, UNRES_WHEN)) {
                        goto error;
                    }
                }
                if (resolve_applies_must(iter) && unres_data_add(unres, iter, UNRES_MUST) == -1) {
                    goto error;
                }

                if (iter->schema->nodetype == LYS_LEAFLIST) {
                    break;
                }
            }
            /* we are in the added leaf-list instance */
            if (((struct lyd_node_leaf_list *)iter)->value_type == LY_TYPE_LEAFREF) {
                if (unres_data_add(unres, (struct lyd_node *)iter, UNRES_LEAFREF)) {
                    goto error;
                }
            } else if (((struct lyd_node_leaf_list *)iter)->value_type == LY_TYPE_INST) {
                if (unres_data_add(unres, (struct lyd_node *)iter, UNRES_INSTID)) {
                    goto error;
                }
            }

            /* add tag */
            iter->dflt = 1;
        }

        if (!ret) {
            ret = last;
        }
    }

    return ret;

error:
    lyd_free_withsiblings(ret);
    return NULL;
}

static struct lyd_node *
lyd_wd_add_leaf(struct ly_ctx *ctx, struct lyd_node *parent, struct lys_node_leaf *leaf,
                const char *path, struct unres_data *unres, int options, int check_existence)
{
    struct lyd_node *ret, *iter;
    struct lys_tpdf *tpdf;
    struct ly_set *nodeset;
    const char *dflt = NULL;

    if ((options & LYD_WD_MASK) == LYD_WD_EXPLICIT && (leaf->flags & LYS_CONFIG_W)) {
        /* do not process config data in explicit mode */
        return NULL;
    } else if (lys_is_disabled((struct lys_node *)leaf, 0)) {
        /* ignore disabled data */
        return NULL;
    }

    if (leaf->dflt) {
        /* leaf has a default value */
        dflt = leaf->dflt;
    } else if (!(leaf->flags & LYS_MAND_TRUE)) {
        /* get the default value from the type */
        for (tpdf = leaf->type.der; tpdf && !dflt; tpdf = tpdf->type.der) {
            dflt = tpdf->dflt;
        }
    }
    if (dflt) {
        if (check_existence && parent) {
            nodeset = lyd_get_node(parent, path);
            if (nodeset && nodeset->number) {
                ly_set_free(nodeset);
                return NULL;
            }
            ly_set_free(nodeset);
        }
        ret = lyd_new_path(parent, ctx, path, dflt, options & LYD_OPT_RPCREPLY ? LYD_PATH_OPT_OUTPUT : 0);
        if ((options & (LYD_WD_ALL_TAG | LYD_WD_IMPL_TAG)) && ret) {
            /* remember the created nodes (if necessary) in unres */
            for (iter = ret; ; iter = iter->child) {
                if ((!(options & LYD_OPT_TYPEMASK) || (options & LYD_OPT_CONFIG)) && (iter->when_status & LYD_WHEN)) {
                    if (unres_data_add(unres, (struct lyd_node *)iter, UNRES_WHEN)) {
                        lyd_free(ret);
                        return NULL;
                    }
                }
                if (resolve_applies_must(iter) && unres_data_add(unres, iter, UNRES_MUST) == -1) {
                    lyd_free(ret);
                    return NULL;
                }

                if (iter->schema->nodetype == LYS_LEAF) {
                    break;
                }
            }
            /* we are in the added leaf */
            if (((struct lyd_node_leaf_list *)iter)->value_type == LY_TYPE_LEAFREF) {
                if (unres_data_add(unres, (struct lyd_node *)iter, UNRES_LEAFREF)) {
                    lyd_free(ret);
                    return NULL;
                }
            } else if (((struct lyd_node_leaf_list *)iter)->value_type == LY_TYPE_INST) {
                if (unres_data_add(unres, (struct lyd_node *)iter, UNRES_INSTID)) {
                    lyd_free(ret);
                    return NULL;
                }
            }

            /* add tag */
            iter->dflt = 1;
        }
        return ret;
    }

    return NULL;
}

/*
 * search for default data in the schema subtree. Create the default nodes as (direct or indirect) children to parent.
 * If parent is NULL then create them from top-level.
 */
static struct lyd_node *
lyd_wd_add_empty(struct lyd_node *parent, struct lys_node *schema, struct unres_data *unres, int options)
{
    struct lys_node *next, *siter;
    struct lyd_node *ret = NULL, *iter;
    char *path = ly_buf(); /* initiated from lyd_wd_top() */
    char *c;
    int index = 0;

    if (schema->nodetype & (LYS_CONTAINER | LYS_LEAF)) {
        if (parent) {
            index = sprintf(path, "%s:%s", lys_node_module(schema)->name, schema->name);
        } else {
            index = sprintf(path, "/%s:%s", lys_node_module(schema)->name, schema->name);
        }
    }

    LY_TREE_DFS_BEGIN(schema, next, siter) {
        if  (options & (LYD_OPT_CONFIG | LYD_OPT_EDIT | LYD_OPT_GETCONFIG)) {
            /* do not process status data */
            if (siter->flags & LYS_CONFIG_R) {
                next = NULL;
                goto nextsibling;
            }
        }
        if (lys_is_disabled(siter, 0)) {
            /* ignore disabled data */
            next = NULL;
            goto nextsibling;
        }

        switch (siter->nodetype) {
        case LYS_LEAF:
            iter = lyd_wd_add_leaf(siter->module->ctx, parent, (struct lys_node_leaf *)siter, path, unres,
                                   options, parent ? 1 : 0);
            if (ly_errno != LY_SUCCESS) {
                if (!parent) {
                    lyd_free_withsiblings(ret);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Creating default element \"%s\" failed.", path);
                } else {
                    LOGVAL(LYE_SPEC, LY_VLOG_LYD, parent, "Creating default element \"%s\" failed.", path);
                }
                path[0] = '\0';
                return NULL;
            } else if (iter && !parent) {
                parent = ret = iter;
            } /* else already connected in parent */
            break;
        case LYS_LEAFLIST:
            iter = lyd_wd_add_leaflist(siter->module->ctx, parent, (struct lys_node_leaflist *)siter, path, unres,
                                       options, parent ? 1 : 0);
            if (ly_errno != LY_SUCCESS) {
                if (!parent) {
                    lyd_free_withsiblings(ret);
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Creating default leaf-list elements \"%s\" failed.", path);
                } else {
                    LOGVAL(LYE_SPEC, LY_VLOG_LYD, parent, "Creating default leaf-list elements \"%s\" failed.", path);
                }
                path[0] = '\0';
                return NULL;
            } else if (iter && !parent) {
                parent = ret = iter;
            } /* else already connected in parent */
            break;
        case LYS_CONTAINER:
            if (((struct lys_node_container *)siter)->presence) {
                /* don't go into presence containers */
                next = NULL;
                goto nextsibling;
            }
            break;
        case LYS_CHOICE:
            if (((struct lys_node_choice *)siter)->dflt) {
                next = ((struct lys_node_choice *)siter)->dflt;
            } else {
                /* forget about this choice */
                next = NULL;
            }
            goto nextsibling;
        case LYS_USES:
        case LYS_CASE:
            /* go into */
            break;
        default:
            /* do not go into children */
            next = NULL;
            goto nextsibling;
        }

        /* where to go next - modified LY_TREE_DFS_END() */
        if (siter->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML)) {
            next = NULL;
        } else {
            next = siter->child;
        }
nextsibling:
        if (!next) {
            /* no children */
            if (siter == schema) {
                /* done */
                break;
            }
            /* try siblings */
            next = siter->next;

            if (next && (siter->nodetype & (LYS_CONTAINER | LYS_LEAF))) {
                /* remove node from the path */
                c = strrchr(path, '/');
                *c = '\0';
                index = c - path;
            }
        }
        while (!next) {
            if (siter->nodetype & (LYS_CONTAINER | LYS_LEAF)) {
                /* remove node from the path */
                c = strrchr(path, '/');
                *c = '\0';
                index = c - path;
            }
            siter = lys_parent(siter);
            if (lys_parent(schema) == lys_parent(siter)) {
                /* done */
                break;
            }
            /* parent was already processed, so go to its sibling */
            if (lys_parent(siter) && lys_parent(siter)->nodetype != LYS_CHOICE) {
                next = siter->next;
                if (next && (siter->nodetype & (LYS_CONTAINER | LYS_LEAF))) {
                    /* remove node from the path */
                    c = strrchr(path, '/');
                    *c = '\0';
                    index = c - path;
                }
            }
        }
        /* add node into the path */
        if (next && (next->nodetype & (LYS_CONTAINER | LYS_LEAF))) {
            index += sprintf(&path[index], "/%s:%s", lys_node_module(next)->name, next->name);
        }
    }

    path[0] = '\0';
    return ret;
}

/* subroot is data node instance of the schema->parent
 */
static int
lyd_wd_add_inner(struct lyd_node *subroot, struct lys_node *schema, struct unres_data *unres, int options)
{
    struct lys_node *siter, *sch;
    struct lyd_node *iter;
    struct ly_set *nodeset, *caseset = NULL;
    char *path = ly_buf(); /* initiated from lyd_wd_top() */
    int rc = EXIT_FAILURE, chcase = 0;

    assert(subroot);

    sch = lys_parent(schema);
    if (sch) {
        /* only one choice's case will be processed,
         * so detect processing of a node under the
         * case */
        if (sch->nodetype == LYS_CHOICE) {
            /* shorthand case */
            chcase = 1;
        } else if (sch->nodetype == LYS_CASE) {
            chcase = 2;
        }
    }

    if (!chcase) {
        /* skip processing of the existing nodes since it was done previously
         * when processing parent choice */
        LY_TREE_FOR(subroot->child, iter) {
            /* examine if the case (even shorthand) is instantiated */
            siter = lys_parent(iter->schema);
            if (siter && (siter->nodetype & (LYS_CHOICE | LYS_CASE))) {
                if (!caseset) {
                    caseset = ly_set_new();
                }
                ly_set_add(caseset, (siter->nodetype == LYS_CASE) ? siter : iter->schema, 0);
            }

            /* recursion */
            if ((iter->schema->nodetype & (LYS_CONTAINER | LYS_LIST)) &&
                    ((options & LYD_WD_MASK) != LYD_WD_EXPLICIT
                    || ((iter->schema->flags & LYS_CONFIG_W) && (iter->schema->flags & LYS_INCL_STATUS)))) {
                lyd_wd_add_inner(iter, iter->schema->child, unres, options);
                if (ly_errno != LY_SUCCESS) {
                    goto error;
                }
            } /* else explicit mode with no status data in subtree -> do nothing */
        }
    }

    LY_TREE_FOR(schema, siter) {
        if (options & (LYD_OPT_CONFIG | LYD_OPT_EDIT | LYD_OPT_GETCONFIG)) {
            /* do not process status data */
            if (siter->flags & LYS_CONFIG_R) {
                continue;
            }
        }
        if (lys_is_disabled(siter, 0)) {
            /* ignore disabled data */
            continue;
        }

        switch (siter->nodetype) {
        case LYS_CONTAINER:
            sprintf(path, "%s:%s", lys_node_module(siter)->name, siter->name);
            nodeset = NULL;
            nodeset = lyd_get_node(subroot, path);
            path[0] = '\0';
            if (!nodeset) {
                goto error;
            }

            if (nodeset->number == 1) {
                /* recursion */
                if ((options & LYD_WD_MASK) != LYD_WD_EXPLICIT
                        || ((siter->flags & LYS_CONFIG_W) && (siter->flags & LYS_INCL_STATUS))) {
                    lyd_wd_add_inner(nodeset->set.d[0], siter->child, unres, options);
                } /* else explicit mode with no status data in subtree -> do nothing */
            } else {
                /* container does not exists, go recursively to add default nodes in its subtree */
                if (((struct lys_node_container *)siter)->presence) {
                    /* but only if it is not presence container */
                    ly_set_free(nodeset);
                    continue;
                } else if ((options & LYD_WD_MASK) == LYD_WD_EXPLICIT
                        && ((siter->flags & LYS_CONFIG_W) && !(siter->flags & LYS_INCL_STATUS))) {
                    /* do not process config data in explicit mode */
                    continue;
                }
                lyd_wd_add_empty(subroot, siter, unres, options);
            }
            ly_set_free(nodeset);
            if (ly_errno != LY_SUCCESS) {
                goto error;
            }

            break;
        case LYS_LEAF:
            sprintf(path, "%s:%s", lys_node_module(siter)->name, siter->name);
            lyd_wd_add_leaf(siter->module->ctx, subroot, (struct lys_node_leaf *)siter, path, unres, options, 1);
            if (ly_errno != LY_SUCCESS) {
                LOGVAL(LYE_SPEC, LY_VLOG_LYD, subroot, "Creating default element \"%s\" failed.", path);
                path[0] = '\0';
                goto error;
            } /* else if default, it was already connected into parent */
            path[0] = '\0';
            break;
        case LYS_LEAFLIST:
            sprintf(path, "%s:%s", lys_node_module(siter)->name, siter->name);
            lyd_wd_add_leaflist(siter->module->ctx, subroot, (struct lys_node_leaflist *)siter, path, unres, options, 1);
            if (ly_errno != LY_SUCCESS) {
                LOGVAL(LYE_SPEC, LY_VLOG_LYD, subroot, "Creating default leaf-list elements \"%s\" failed.", path);
                path[0] = '\0';
                goto error;
            } /* else if default, it was already connected into parent */
            path[0] = '\0';
            break;
        case LYS_CHOICE:
            /* check that no case is instantiated */
            sch = lyd_wd_get_inst_case(caseset, siter);
            if (!sch) {
                if (((struct lys_node_choice *)siter)->dflt) {
                    /* go to the default case */
                    lyd_wd_add_inner(subroot, ((struct lys_node_choice *)siter)->dflt, unres, options);
                }
            } else if (sch->nodetype == LYS_CASE) {
                /* add missing default nodes from present choice case */
                lyd_wd_add_inner(subroot, sch->child, unres, options);
            }
            if (ly_errno != LY_SUCCESS) {
                goto error;
            }
            break;
        case LYS_INPUT:
        case LYS_OUTPUT:
            assert(options & (LYD_OPT_RPC | LYD_OPT_ACTION | LYD_OPT_RPCREPLY));
            if ((siter->nodetype == LYS_INPUT) && (options & LYD_OPT_RPCREPLY)) {
                /* skip input */
                break;
            }
            if ((siter->nodetype == LYS_OUTPUT) && (options & (LYD_OPT_RPC | LYD_OPT_ACTION))) {
                /* skip output */
                break;
            }
            /* fallthrough */
        case LYS_USES:
        case LYS_CASE:
            /* go into */
            lyd_wd_add_inner(subroot, siter->child, unres, options);
            if (ly_errno != LY_SUCCESS) {
                goto error;
            }
            break;
        default:
            /* do nothing */
            break;
        }

        if (chcase == 1) {
            /* only one choice's case is processed, so we are done here */
            break;
        }
    }

    rc = EXIT_SUCCESS;

error:
    ly_set_free(caseset);
    return rc;
}

static int
lyd_wd_top(struct lyd_node **root, struct unres_data *unres, int options, struct ly_ctx *ctx)
{
    struct lys_node *siter, *sch;
    struct lyd_node *iter;
    struct ly_set *topset = NULL, *nodeset, *caseset = NULL;
    unsigned int i;
    int ret = EXIT_FAILURE;
    char *path = ly_buf(), *buf_backup = NULL;

    if ((options & LYD_WD_MASK) == LYD_WD_TRIM) {
        /* specific mode, we are not adding something, but removing something */
        return lyd_wd_trim(root, options);
    } else if ((options & LYD_WD_MASK) == LYD_WD_ALL_TAG) {
        /* as first part, mark the explicit default nodes ... */
        lyd_wd_trim(root, options);
        /* and then continue by adding the missing default nodes */
    } else if  ((options & LYD_WD_MASK) == LYD_WD_EXPLICIT
            && (options & (LYD_OPT_CONFIG | LYD_OPT_EDIT | LYD_OPT_GETCONFIG))) {
        /* Explicit mode, but the result is not supposed to contain status data,
         * so there is nothing to do */
        return EXIT_SUCCESS;
    }

    /* initiate internal buffer (needed before calls of lyd_wd_empty, for one) */
    if (ly_buf_used && path[0]) {
        buf_backup = strndup(path, LY_BUF_SIZE - 1);
    }
    ly_buf_used++;
    path[0] = '\0';

    if (!(options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY | LYD_OPT_NOTIF))) {
        topset = ly_set_new();
    }
    LY_TREE_FOR(*root, iter) {
        if ((!ctx || (options & LYD_OPT_NOSIBLINGS)) && !(options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY | LYD_OPT_NOTIF))) {
            ly_set_add(topset, lys_node_module(iter->schema)->data, 0);
        }
        if (options & (LYD_OPT_CONFIG | LYD_OPT_EDIT | LYD_OPT_GETCONFIG)) {
            /* do not process status data */
            if (iter->schema->flags & LYS_CONFIG_R) {
                continue;
            }
        }

        if (iter->schema->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
            if ((options & LYD_WD_MASK) == LYD_WD_EXPLICIT
                    && ((iter->schema->flags & LYS_CONFIG_W) && !(iter->schema->flags & LYS_INCL_STATUS))) {
                /* do not process config data in explicit mode */
                continue;
            }
            /* go into */
            if (lyd_wd_add_inner(iter, iter->schema->child, unres, options)) {
                goto cleanup;
            }
        }

        if (!ctx && (options & LYD_OPT_NOSIBLINGS)) {
            /* done, do not add any missing siblings */
            ret = EXIT_SUCCESS;
            goto cleanup;
        }

        /* examine if the node is in case and its siblings with default values
         * should be created */
        siter = lys_parent(iter->schema);
        if (siter && (siter->nodetype & (LYS_CHOICE | LYS_CASE))) {
            if (!caseset) {
                caseset = ly_set_new();
            }
            ly_set_add(caseset, (siter->nodetype == LYS_CASE) ? siter : iter->schema, 0);
        }
    }

    if (options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY | LYD_OPT_NOTIF)) {
        /* do not add any top-level default nodes in these cases */
        ret = EXIT_SUCCESS;
        goto cleanup;
    }

    if (ctx && !(options & LYD_OPT_NOSIBLINGS)) {
        /* add all module data into our internal set */
        for (i = 0; i < (unsigned int)ctx->models.used; i++) {
            if (ctx->models.list[i]->data) {
                ly_set_add(topset, ctx->models.list[i]->data, LY_SET_OPT_USEASLIST);
            }
        }
    }

    /* add missing top-level default nodes */
    for (i = 0; i < topset->number; i++) {
        LOGDBG("DEFAULTS: adding top level defaults for %s module, mode %x", lys_node_module(topset->set.s[i])->name,
               (options & LYD_WD_MASK));
        LY_TREE_FOR(topset->set.s[i], siter) {
            if  (options & (LYD_OPT_CONFIG | LYD_OPT_EDIT | LYD_OPT_GETCONFIG)) {
                /* do not process status data */
                if (siter->flags & LYS_CONFIG_R) {
                    continue;
                }
            }
            if (lys_is_disabled(siter, 0)) {
                /* ignore disabled data */
                continue;
            }

            switch (siter->nodetype) {
            case LYS_CONTAINER:
                if (((struct lys_node_container *)siter)->presence) {
                    continue;
                }

                if ((iter = *root)) {
                    sprintf(path, "/%s:%s", lys_node_module(siter)->name, siter->name);
                    nodeset = NULL;
                    nodeset = lyd_get_node(*root, path);
                    path[0] = '\0';
                    if (!nodeset) {
                        goto cleanup;
                    }
                    if (!nodeset->number) {
                        iter = NULL;
                    }
                    ly_set_free(nodeset);
                }

                if (!iter) {
                    /* container does not exists, go recursively to add default nodes in its subtree */
                    iter = lyd_wd_add_empty(NULL, siter, unres, options);
                    if (ly_errno != LY_SUCCESS) {
                        goto cleanup;
                    }
                } else {
                    iter = NULL;
                }
                break;
            case LYS_LEAF:
                sprintf(path, "/%s:%s", lys_node_module(siter)->name, siter->name);
                iter = lyd_wd_add_leaf(siter->module->ctx, *root, (struct lys_node_leaf *)siter, path, unres, options, 1);
                if (ly_errno != LY_SUCCESS) {
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Creating default element \"%s\" failed.", path);
                    path[0] = '\0';
                    goto cleanup;
                }
                path[0] = '\0';

                if (iter) {
                    if (!(*root)) {
                        *root = iter;
                    }
                    /* avoid lyd_insert_after() after the switch since leaf is already added into the tree */
                    iter = NULL;
                }
                break;
            case LYS_LEAFLIST:
                sprintf(path, "/%s:%s", lys_node_module(siter)->name, siter->name);
                iter = lyd_wd_add_leaflist(siter->module->ctx, *root, (struct lys_node_leaflist *)siter, path, unres,
                                           options, 1);
                if (ly_errno != LY_SUCCESS) {
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Creating default leaf-list elements \"%s\" failed.", path);
                    path[0] = '\0';
                    goto cleanup;
                }
                path[0] = '\0';

                if (iter) {
                    if (!(*root)) {
                        *root = iter;
                    }
                    /* avoid lyd_insert_after() after the switch since leaf-list instance is already added into the tree */
                    iter = NULL;
                }
                break;
            case LYS_CHOICE:
                /* check that no case is instantiated */
                sch = lyd_wd_get_inst_case(caseset, siter);
                if (!sch) {
                    if (((struct lys_node_choice *)siter)->dflt) {
                        /* go to the default case */
                        iter = lyd_wd_add_empty(NULL, ((struct lys_node_choice *)siter)->dflt, unres, options);
                    }
                } else if (sch->nodetype == LYS_CASE) {
                    /* add missing default nodes from present choice case */
                    iter = lyd_wd_add_empty(NULL, sch, unres, options);
                } else {
                    iter = NULL;
                }
                if (ly_errno != LY_SUCCESS) {
                    goto cleanup;
                }
                break;
            case LYS_USES:
                /* go into */
                iter = lyd_wd_add_empty(NULL, siter->child, unres, options);
                if (ly_errno != LY_SUCCESS) {
                    goto cleanup;
                }
                break;
            default:
                /* do nothing */
                iter = NULL;
                break;
            }

            /* add to the top-level */
            if (iter) {
                if (!(*root)) {
                    *root = iter;
                } else {
                    lyd_insert_after((*root)->prev, iter);
                    if (ly_errno != LY_SUCCESS) {
                        goto cleanup;
                    }
                }
            }

        }
    }

    if (*root && (options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY))) {
        lyd_schema_sort((*root)->child, 0);
    }

    ret = EXIT_SUCCESS;

cleanup:
    ly_set_free(topset);
    ly_set_free(caseset);

    if (buf_backup) {
        /* return previous internal buffer content */
        strcpy(path, buf_backup);
        free(buf_backup);
    }
    ly_buf_used--;

    return ret;
}

int
lyd_defaults_add_unres(struct lyd_node **root, int options, struct ly_ctx *ctx, struct unres_data *unres)
{
    int options_aux;

    assert(root);
    assert((options & LYD_WD_TRIM) || unres);

    if ((options & LYD_OPT_NOSIBLINGS) && !(*root)) {
        LOGERR(LY_EINVAL, "Cannot add default values for one module (LYD_OPT_NOSIBLINGS) without any data.");
        return EXIT_FAILURE;
    }

    if (options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY | LYD_OPT_NOTIF)) {
        if (!(*root)) {
            LOGERR(LY_EINVAL, "Cannot add default values to RPC, RPC reply, and notification without at least the empty container.");
            return EXIT_FAILURE;
        }
        if ((options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY)) && !((*root)->schema->nodetype & (LYS_RPC | LYS_ACTION))) {
            LOGERR(LY_EINVAL, "Not valid RPC/action data.");
            return EXIT_FAILURE;
        }
        if ((options & LYD_OPT_NOTIF) && ((*root)->schema->nodetype != LYS_NOTIF)) {
            LOGERR(LY_EINVAL, "Not valid notification data.");
            return EXIT_FAILURE;
        }
    } else if (*root && (*root)->parent) {
        /* we have inner node, so it will be considered as
         * a root of subtree where to add default nodes and
         * no of its siblings will be affected */
        options |= LYD_OPT_NOSIBLINGS;
        ctx = NULL;
    }

    /* add default values if needed */
    if (options & LYD_WD_EXPLICIT) {
        options_aux = (options & ~LYD_WD_MASK) | LYD_WD_IMPL_TAG;
    } else if (!(options & LYD_WD_MASK)) {
        options_aux = options | LYD_WD_IMPL_TAG;
    } else {
        options_aux = options;
    }
    if (lyd_wd_top(root, unres, options_aux, ctx)) {
        return EXIT_FAILURE;
    }

    /* check leafrefs and/or instids if any */
    if (unres && unres->count) {
        if (!(*root) || resolve_unres_data(unres, (options & LYD_OPT_NOAUTODEL) ? NULL : root, options)) {
            /* leafref & instid checking failed */
            return EXIT_FAILURE;
        }
    }

    if (options_aux != options) {
        /* cleanup default nodes used for internal purposes (not asked by caller) */
        if (lyd_wd_cleanup(root, options)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

API int
lyd_wd_add(struct ly_ctx *ctx, struct lyd_node **root, int options)
{
    int rc, mode;
    struct unres_data *unres = NULL;

    ly_errno = LY_SUCCESS;

    if (!root || (!ctx && !(*root)) || (ctx && !(*root) && (options & LYD_OPT_NOSIBLINGS))) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    mode = options & LYD_WD_MASK;
    if (!mode) {
        /* nothing to do */
        return EXIT_SUCCESS;
    } else if (mode != LYD_WD_TRIM && mode != LYD_WD_ALL &&
            mode != LYD_WD_ALL_TAG && mode != LYD_WD_IMPL_TAG) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    if (mode != LYD_WD_TRIM) {
        unres = calloc(1, sizeof *unres);
        if (!unres) {
            LOGMEM;
            return EXIT_FAILURE;
        }
    }

    if (*root && (ctx || !(options & LYD_OPT_NOSIBLINGS))) {
        /* check that the node is the first sibling */
        while ((*root)->prev->next) {
            *root = (*root)->prev;
        }
    }

    rc = lyd_defaults_add_unres(root, options, ctx, unres);

    /* cleanup */
    if (unres) {
        free(unres->node);
        free(unres->type);
        free(unres);
    }

    return rc;
}

API struct lys_module *
lyd_node_module(const struct lyd_node *node)
{
    return node->schema->module->type ? ((struct lys_submodule *)node->schema->module)->belongsto : node->schema->module;
}
