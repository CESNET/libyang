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

static int
lyd_check_topmandatory(struct ly_ctx *ctx, struct lyd_node *data, int options)
{
    assert(ctx);
    int i;

    /* TODO what about LYD_OPT_NOTIF, LYD_OPT_RPC and LYD_OPT_RPCREPLY ? */
    if ((options & LYD_OPT_NOSIBLINGS) || (options & (LYD_OPT_EDIT | LYD_OPT_GET | LYD_OPT_GETCONFIG))) {
        return EXIT_SUCCESS;
    }

    /* LYD_OPT_DATA || LYD_OPT_CONFIG */

    /* check for missing mandatory elements (from the top level) according to schemas in context */
    for (i = 0; i < ctx->models.used; i++) {
        if (!ctx->models.list[i]->data) {
            continue;
        }
        if (ly_check_mandatory(data, ctx->models.list[i]->data, (options & LYD_OPT_TYPEMASK) ? 0 : 1)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

static struct lyd_node *
lyd_parse_(struct ly_ctx *ctx, const struct lys_node *parent, const char *data, LYD_FORMAT format, int options)
{
    struct lyxml_elem *xml, *xmlnext;
    struct lyd_node *result = NULL;
    int xmlopt = LYXML_PARSE_MULTIROOT;
    struct unres_data *unres = NULL;

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
        LY_TREE_FOR_SAFE(xml, xmlnext, xml) {
            lyxml_free(ctx, xml);
        }
        break;
    case LYD_JSON:
        result = lyd_parse_json(ctx, parent, data, options);
        break;
    default:
        /* error */
        return NULL;
    }

    if (!ly_errno) {
        if (!result) {
            /* is empty data tree really valid ? */
            lyd_validate(&result, options, ctx);
        } else {
            /* check for missing top level mandatory nodes */
            lyd_check_topmandatory(ctx, result, options);

            /* add default nodes if requested */
            if (options & LYD_WD_MASK) {
                unres = calloc(1, sizeof *unres);
                if (!unres) {
                    LOGMEM;
                    return NULL;
                }

                lyd_wd_top(ctx, &result, unres, options);

                if (unres->count) {
                    /* check unresolved checks (when-stmt) */
                    resolve_unres_data(unres,  (options & LYD_OPT_NOAUTODEL) ? NULL : &result, options);
                }
            }
        }
    }

    if (unres) {
        free(unres->node);
        free(unres->type);
        free(unres);
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
    const struct lys_node *rpc = NULL;

    if (lyp_check_options(options)) {
        LOGERR(LY_EINVAL, "%s: Invalid options (multiple data type flags set).", __func__);
        return NULL;
    }

    if (options & LYD_OPT_RPCREPLY) {
        rpc = va_arg(ap,  struct lys_node*);
        if (!rpc || (rpc->nodetype != LYS_RPC)) {
            LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
            return NULL;
        }
    }

    return lyd_parse_(ctx, rpc, data, format, options);
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

API struct lyd_node *
lyd_parse_fd(struct ly_ctx *ctx, int fd, LYD_FORMAT format, int options, ...)
{
    struct lyd_node *ret;
    struct stat sb;
    char *data;
    va_list ap;

    if (!ctx || (fd == -1)) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    if (fstat(fd, &sb) == -1) {
        LOGERR(LY_ESYS, "Failed to stat the file descriptor (%s).", strerror(errno));
        return NULL;
    }

    if (!sb.st_size) {
        return NULL;
    }

    data = mmap(NULL, sb.st_size + 1, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        LOGERR(LY_ESYS, "Mapping file descriptor into memory failed.");
        return NULL;
    }

    va_start(ap, options);
    ret = lyd_parse_data_(ctx, data, format, options, ap);

    va_end(ap);
    munmap(data, sb.st_size + 1);

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
    ret = lyd_parse_fd(ctx, fd, format, options);

    va_end(ap);
    close(fd);

    return ret;
}

static struct lyd_node *
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

    if (!parent) {
        siblings = module->data;
    } else {
        if (!parent->schema) {
            return NULL;
        }
        siblings = parent->schema->child;
    }

    if (lys_get_data_sibling(module, siblings, name, LYS_CONTAINER | LYS_LIST | LYS_NOTIF | LYS_RPC, &snode)
            || !snode) {
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

    if (!parent) {
        siblings = module->data;
    } else {
        if (!parent->schema) {
            ly_errno = LY_EINVAL;
            return NULL;
        }
        siblings = parent->schema->child;
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
            if (slist->keys[i]->name == leaf->schema->name) {
                LOGVAL(LYE_SPEC, LY_VLOG_LYD, leaf, "List key value cannot be changed.");
                return EXIT_FAILURE;
            }
        }
    }

    backup = leaf->value_str;
    leaf->value_str = lydict_insert(leaf->schema->module->ctx, val_str ? val_str : "", 0);

    /* resolve the type correctly */
    if (lyp_parse_value(leaf, NULL, 1)) {
        lydict_remove(leaf->schema->module->ctx, leaf->value_str);
        leaf->value_str = backup;
        return EXIT_FAILURE;
    }

    /* value is correct, remove backup */
    lydict_remove(leaf->schema->module->ctx, backup);

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

    if (!parent) {
        siblings = module->data;
    } else {
        if (!parent->schema) {
            return NULL;
        }
        siblings = parent->schema->child;
    }

    if (lys_get_data_sibling(module, siblings, name, LYS_ANYXML, &snode) || !snode) {
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

    if (!parent) {
        siblings = module->data;
    } else {
        if (!parent->schema) {
            return NULL;
        }
        siblings = parent->schema->child;
    }

    if (lys_get_data_sibling(module, siblings, name, LYS_ANYXML, &snode) || !snode) {
        return NULL;
    }

    return _lyd_new_anyxml(parent, snode, NULL, val_xml);
}

API struct lyd_node *
lyd_output_new(const struct lys_node *schema)
{
    struct lyd_node *ret;

    if (!schema || !(schema->nodetype & (LYS_CONTAINER | LYS_LIST))
            || !lys_parent(schema) || (lys_parent(schema)->nodetype != LYS_OUTPUT)) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

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

    return ret;
}

API struct lyd_node *
lyd_output_new_leaf(const struct lys_node *schema, const char *val_str)
{
    struct lyd_node *ret;

    if (!schema || (schema->nodetype != LYS_LEAF)
            || !lys_parent(schema) || (lys_parent(schema)->nodetype != LYS_OUTPUT)) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    ret = lyd_create_leaf(schema, val_str);

    /* resolve the type correctly */
    if (lyp_parse_value((struct lyd_node_leaf_list *)ret, NULL, 1)) {
        lyd_free((struct lyd_node *)ret);
        return NULL;
    }

    return ret;
}

API struct lyd_node *
lyd_output_new_anyxml_str(const struct lys_node *schema, char *val_str)
{
    if (!schema || (schema->nodetype != LYS_ANYXML)
            || !lys_parent(schema) || (lys_parent(schema)->nodetype != LYS_OUTPUT)) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    return lyd_create_anyxml(schema, val_str, NULL);
}

API struct lyd_node *
lyd_output_new_anyxml_xml(const struct lys_node *schema, struct lyxml_elem *val_xml)
{
    if (!schema || (schema->nodetype != LYS_ANYXML)
            || !lys_parent(schema) || (lys_parent(schema)->nodetype != LYS_OUTPUT)) {
        ly_errno = LY_EINVAL;
        return NULL;
    }

    return lyd_create_anyxml(schema, NULL, val_xml);
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

        if ((r = parse_schema_list_predicate(predicate, &name, &nam_len, &value, &val_len, &has_predicate)) < 1) {
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
    const char *mod_name, *name, *node_mod_name, *id;
    struct lyd_node *ret = NULL, *node, *parent = NULL;
    const struct lys_node *schild, *sparent;
    const struct lys_node_list *slist;
    const struct lys_module *module, *prev_mod;
    int r, i, parsed = 0, mod_name_len, nam_len, is_relative = -1, first_iter = 1;

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

    if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL)) < 1) {
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
            if (schild->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_NOTIF | LYS_RPC)) {
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

                /* RPC check */
                if (options & LYD_PATH_OPT_OUTPUT) {
                    if (lys_parent(schild) && (lys_parent(schild)->nodetype == LYS_INPUT)) {
                        continue;
                    }

                    /* special case when we are creating a new child of an RPC output,
                     * we do not want to create the RPC container (it is not part of
                     * the data) and we then need to insert it as a top-level sibling
                     * of the data_tree, if any */
                    if (schild->nodetype == LYS_RPC) {
                        if (!id[0]) {
                            /* only create the RPC container of output? that does not make sense */
                            LOGERR(LY_EINVAL, "%s: RPC is not included in data when creating output nodes, nothing to create.");
                            lyd_free(ret);
                            return NULL;
                        }

                        node = NULL;
                        is_relative = 0;
                        goto next_iter;
                    }
                } else {
                    if (lys_parent(schild) && (lys_parent(schild)->nodetype == LYS_OUTPUT)) {
                        continue;
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
            node = _lyd_new(is_relative ? parent : NULL, schild);
            break;
        case LYS_LEAF:
        case LYS_LEAFLIST:
            if (id[0]) {
                LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
                return NULL;
            }
            node = _lyd_new_leaf(is_relative ? parent : NULL, schild, value);
            break;
        case LYS_ANYXML:
            if (id[0]) {
                LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
                return NULL;
            }
            str = strdup(value);
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
            if ((options & LYD_PATH_OPT_OUTPUT) && lyd_schema_sort(node, 0)) {
                lyd_free(ret);
                return NULL;
            } else {
                sparent = node->schema;
                do {
                    sparent = lys_parent(sparent);
                } while (sparent && (sparent->nodetype != LYS_INPUT));
                if (sparent && lyd_schema_sort(node, 0)) {
                    lyd_free(ret);
                    return NULL;
                }
            }
            ret = node;
            first_iter = 0;
        }

        parsed = 0;
        if ((schild->nodetype == LYS_LIST) && lyd_new_path_list_keys(node, name, id, &parsed)) {
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

next_iter:
        /* prepare for another iteration */
        parent = node;
        sparent = schild;
        prev_mod = lys_node_module(schild);

        /* parse another node */
        if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL)) < 1) {
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
            /* got through siblings */
            if (!next) {
nextsibling:
                next = elem->next;
                if (!next) {
                    /* no children */
                    if (elem == node) {
                        /* we are done, back in start node */
                        break;
                    }
                    /* try siblings */
                    next = elem->next;
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
        if (node && iter == node) {
            continue;
        }

        sparent = lys_parent(iter->schema);
        if ((sparent->nodetype == LYS_CHOICE && sparent == schoice) /* another implicit case */
                || (sparent->nodetype == LYS_CASE && sparent != scase && lys_parent(sparent) == schoice) /* another case */
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
         sparent && !(sparent->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_RPC | LYS_OUTPUT | LYS_NOTIF));
         sparent = lys_parent(sparent));
    if (sparent != parent->schema) {
        return EXIT_FAILURE;
    }

    if (node->parent != parent || lyp_is_rpc(node->schema)) {
        /* it is not just moving under a parent node or it is in an RPC where
         * nodes order matters, so the validation will be necessary */
        invalid = 1;
    }

    if (node->parent || node->prev->next) {
        lyd_unlink(node);
    }

    /* auto delete nodes from other cases, if any */
    lyv_multicases(node, NULL, parent->child, 1, NULL);

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
         par1 && !(par1->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_INPUT | LYS_OUTPUT | LYS_NOTIF));
         par1 = lys_parent(par1));
    for (par2 = lys_parent(node->schema);
         par2 && !(par2->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_INPUT | LYS_OUTPUT | LYS_NOTIF));
         par2 = lys_parent(par2));
    if ((par1 != par2)
            && !(par1 && (par1->nodetype == LYS_OUTPUT) && !par2)
            && !(par2 && (par2->nodetype == LYS_OUTPUT) && !par1)) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    if (node->parent != sibling->parent || !node->parent || (invalid = lyp_is_rpc(node->schema))) {
        /* a) it is not just moving under a parent node (invalid = 1) or
         * b) it is top-level where we don't know if it is the same tree (invalid = 1), or
         * c) it is in an RPC where nodes order matters (invalid = 2),
         * so the validation will be necessary */
        if (!node->parent) {
            /* b) search in siblings */
            for (iter = node->prev; iter != node; iter = iter->prev) {
                if (iter == sibling) {
                    break;
                }
            }
            if (iter == node) {
                /* node and siblings are not currently in the same data tree */
                invalid++;
            }
        } else { /* a) and c) */
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
        lyd_unlink(node);
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

/* compare all siblings */
static int
lys_module_node_pos_r(struct lys_node *siblings, struct lys_node *target, uint32_t *pos)
{
    const struct lys_node *next = NULL;

    while ((next = lys_getnext(next, lys_parent(siblings), lys_node_module(siblings), 0))) {
        ++(*pos);
        if (target == next) {
            return 0;
        }

        if ((next->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_RPC | LYS_NOTIF)) && next->child) {
            if (!lys_module_node_pos_r(next->child, target, pos)) {
                return 0;
            }
        }
    }

    return 1;
}

static uint32_t
lys_module_node_pos(struct lys_node *node)
{
    uint32_t pos = 0;

    assert(node->module->data);
    if (lys_module_node_pos_r(lys_node_module(node)->data, node, &pos)) {
        LOGINT;
        return 0;
    }

    return pos;
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
    struct lyd_node_pos *array;

    if (!sibling) {
        ly_errno = LY_EINVAL;
        return -1;
    }

    /* nothing to sort */
    if (sibling->prev == sibling) {
        return EXIT_SUCCESS;
    }

    /* find the beginning */
    if (sibling->parent) {
        sibling = sibling->parent->child;
    } else {
        while (sibling->prev->next) {
            sibling = sibling->prev;
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
        array[i].pos = lys_module_node_pos(node->schema);
        if (!array[i].pos) {
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

    /* sort all the children recursively */
    if (recursive) {
        LY_TREE_FOR(sibling, node) {
            if ((node->schema->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_RPC | LYS_NOTIF))
                    && lyd_schema_sort(node->child, recursive)) {
                return -1;
            }
        }
    }

    return EXIT_SUCCESS;
}

API int
lyd_validate(struct lyd_node **node, int options, ...)
{
    struct lyd_node *root, *next1, *next2, *iter, *to_free = NULL;
    struct ly_ctx *ctx = NULL;
    int ret = EXIT_FAILURE, ap_flag = 0, options_aux;
    va_list ap;
    struct unres_data *unres = NULL;

    if (!node) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    unres = calloc(1, sizeof *unres);
    if (!unres) {
        LOGMEM;
        return EXIT_FAILURE;
    }

    ly_errno = 0;

    if (!(*node)) {
        /* get context with schemas from the variable arguments */
        va_start(ap, options);
        ap_flag = 1;
        ctx = va_arg(ap,  struct ly_ctx*);
        if (!ctx) {
            LOGERR(LY_EINVAL, "%s: Invalid variable argument.", __func__);
            goto error;
        }
    } else {
        ctx = (*node)->schema->module->ctx;

        if (!(options & LYD_OPT_NOSIBLINGS)) {
            /* check that the node is the first sibling */
            while((*node)->prev->next) {
                *node = (*node)->prev;
            }
        }
    }

    if (lyd_check_topmandatory(ctx, *node, options)) {
        goto error;
    }

    LY_TREE_FOR_SAFE(*node, next1, root) {
        LY_TREE_DFS_BEGIN(root, next2, iter) {
            if (to_free) {
                lyd_free(to_free);
                to_free = NULL;
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
            iter->validity = LYD_VAL_OK;

            /* where go next? - modified LY_TREE_DFS_END */
            if (iter->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML)) {
                next2 = NULL;
            } else {
                next2 = iter->child;

                /* if we have empty non-presence container, we can remove it */
                if (!next2 && !(options & LYD_OPT_KEEPEMPTYCONT) && iter->schema->nodetype == LYS_CONTAINER
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
                if (!(options & LYD_OPT_KEEPEMPTYCONT) && to_free && !to_free->next && to_free->prev == to_free &&
                        iter->schema->nodetype == LYS_CONTAINER &&
                        !((struct lys_node_container *)iter->schema)->presence) {
                    lyd_free(to_free);
                    to_free = iter;
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

    /* add default values if needed */
    if (options & LYD_WD_EXPLICIT) {
        options_aux = (options & ~LYD_WD_MASK) | LYD_WD_IMPL_TAG;
    } else if (!(options & LYD_WD_MASK)) {
        options_aux = options | LYD_WD_IMPL_TAG;
    } else {
        options_aux = options;
    }
    if (lyd_wd_top(ctx ? ctx : (*node)->schema->module->ctx, node, unres, options_aux)) {
        goto error;
    }

    ret = EXIT_SUCCESS;

    if (unres->count) {
        /* check unresolved checks (when-stmt) */
        if (resolve_unres_data(unres,  (options & LYD_OPT_NOAUTODEL) ? NULL : node, options)) {
            ret = EXIT_FAILURE;
        }
    }

    if (options != options_aux) {
        /* cleanup default nodes */
        if (lyd_wd_cleanup(node, options)) {
            ret = EXIT_FAILURE;
        }
    }


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

API int
lyd_unlink(struct lyd_node *node)
{
    struct lyd_node *iter, *next;
    struct ly_set *set, *data;
    unsigned int i, j;

    if (!node) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

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
                }
            }
        }
        LY_TREE_DFS_END(node, next, iter)
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

            new_leaf->value = ((struct lyd_node_leaf_list *)elem)->value;
            new_leaf->value_str = lydict_insert(elem->schema->module->ctx,
                                                ((struct lyd_node_leaf_list *)elem)->value_str, 0);
            new_leaf->value_type = ((struct lyd_node_leaf_list *)elem)->value_type;
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

struct lyd_node *
lyd_attr_parent(struct lyd_node *root, struct lyd_attr *attr)
{
    struct lyd_node *next, *elem;
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
            lyxml_free(node->schema->module->ctx, ((struct lyd_node_anyxml *)node)->value.xml);
        } else {
            lydict_remove(node->schema->module->ctx, ((struct lyd_node_anyxml *)node)->value.str);
        }
    } else { /* LYS_LEAF | LYS_LEAFLIST */
        /* free value */
        switch (((struct lyd_node_leaf_list *)node)->value_type) {
        case LY_TYPE_BINARY:
        case LY_TYPE_STRING:
            lydict_remove(node->schema->module->ctx, ((struct lyd_node_leaf_list *)node)->value.string);
            break;
        case LY_TYPE_BITS:
            if (((struct lyd_node_leaf_list *)node)->value.bit) {
                free(((struct lyd_node_leaf_list *)node)->value.bit);
            }
            break;
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
static const char *
lyd_get_default(const char* unique_expr, struct lyd_node_leaf_list *list)
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
    for (parent = sleaf->parent; parent != list->schema; parent = parent->parent) {
        if (!(parent->nodetype & (LYS_CONTAINER | LYS_CASE | LYS_CHOICE | LYS_USES))) {
            /* This should be already detected when parsing schema */
            LOGINT;
            ly_set_free(s);
            return NULL;
        }
        ly_set_add(s, (void *)parent);
    }
    ly_vlog_hide(1);
    for (i = 0, last = (struct lyd_node *)list; i < s->number; i++) {
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

int
lyd_list_equal(struct lyd_node *first, struct lyd_node *second)
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
            LOGVAL(LYE_DUPLEAFLIST, LY_VLOG_LYD, second, second->schema->name,
                   ((struct lyd_node_leaf_list *)second)->value_str);
            return 1;
        }
        return 0;
    case LYS_LIST:
        slist = (struct lys_node_list *)first->schema;

        /* compare unique leafs */
        for (i = 0; i < slist->unique_size; i++) {
            for (j = 0; j < slist->unique[i].expr_size; j++) {
                /* first */
                diter = resolve_data_descendant_schema_nodeid(slist->unique[i].expr[j], first->child);
                if (diter) {
                    val1 = ((struct lyd_node_leaf_list *)diter)->value_str;
                } else {
                    /* use default value */
                    val1 = lyd_get_default(slist->unique[i].expr[j], (struct lyd_node_leaf_list *)first);
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
                    val2 = lyd_get_default(slist->unique[i].expr[j], (struct lyd_node_leaf_list *)second);
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
        }

        if (second->validity == LYD_VAL_UNIQUE) {
            /* only unique part changed somewhere, so it is no need to check keys */
            return 0;
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

        LOGVAL(LYE_DUPLIST, LY_VLOG_LYD, second, second->schema->name);
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
            if ((xp_set.node_type[i] == LYXP_NODE_ELEM) || (xp_set.node_type[i] == LYXP_NODE_TEXT)) {
                if (ly_set_add(set, xp_set.value.nodes[i])) {
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
            !(schema->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_ANYXML | LYS_NOTIF | LYS_RPC))) {
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
        } else if (siter->nodetype == LYS_OUTPUT) {
            /* done for RPC reply */
            break;
        } else if (siter->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_ANYXML | LYS_NOTIF | LYS_RPC)) {
            /* standard data node */
            ly_set_add(spath, (void*)siter);

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
            ly_set_add(ret, iter);
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
                    ly_set_add(ret_aux, iter);
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

API int
ly_set_add(struct ly_set *set, void *node)
{
    unsigned int i;
    void **new;

    if (!set || !node) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    /* search for duplication */
    for (i = 0; i < set->number; i++) {
        if (set->set.g[i] == node) {
            /* already in set */
            return EXIT_SUCCESS;
        }
    }

    if (set->size == set->number) {
        new = realloc(set->set.g, (set->size + 8) * sizeof *(set->set.g));
        if (!new) {
            LOGMEM;
            return EXIT_FAILURE;
        }
        set->size += 8;
        set->set.g = new;
    }

    set->set.g[set->number++] = node;

    return EXIT_SUCCESS;
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
                    lyd_free(to_free);
                    to_free = iter;
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
                    lyd_free(to_free);
                    to_free = iter;
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
 * data First data node on level where to search for instance of the choice data
 * schema Schema node of the choice
 */
static struct lyd_node *
lyd_wd_get_choice_inst(struct lyd_node *data, struct lys_node *schema)
{
    struct lyd_node *iter;
    struct lys_node *sparent;

    /* check that no case is instantiated */
    LY_TREE_FOR(data, iter) {
        for (sparent = iter->schema->parent; sparent; sparent = sparent->parent) {
            if (!(sparent->nodetype & (LYS_CASE | LYS_CHOICE | LYS_USES))) {
                sparent = NULL;
                break;
            } else if (sparent == schema) {
                /* instance found */
                return iter;
            }
        }
    }

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
        ret = lyd_new_path(parent, ctx, path, dflt, 0);
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

    if (parent) {
        index = sprintf(path, "%s:%s", lys_node_module(schema)->name, schema->name);
    } else {
        index = sprintf(path, "/%s:%s", lys_node_module(schema)->name, schema->name);
    }

    LY_TREE_DFS_BEGIN(schema, next, siter) {
        if  (options & (LYD_OPT_CONFIG | LYD_OPT_EDIT | LYD_OPT_GETCONFIG)) {
            /* do not process status data */
            if (siter->flags & LYS_CONFIG_R) {
                next = NULL;
                goto nextsibling;
            }
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
            if (schema->parent == lys_parent(siter)) {
                /* done */
                break;
            }
            /* parent was already processed, so go to its sibling */
            if (siter->parent && lys_parent(siter)->nodetype != LYS_CHOICE) {
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
    struct lys_node *siter;
    struct lyd_node *iter;
    struct ly_set *nodeset;
    char *path = ly_buf(); /* initiated from lyd_wd_top() */

    assert(subroot);

    LY_TREE_FOR(subroot->child, iter) {
        if (iter->schema->nodetype != LYS_LIST) {
            continue;
        }

        /* LYS_LIST - go into */
        lyd_wd_add_inner(iter, iter->schema->child, unres, options);
        if (ly_errno != LY_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    LY_TREE_FOR(schema, siter) {
        if  (options & (LYD_OPT_CONFIG | LYD_OPT_EDIT | LYD_OPT_GETCONFIG)) {
            /* do not process status data */
            if (siter->flags & LYS_CONFIG_R) {
                continue;
            }
        }

        switch(siter->nodetype) {
        case LYS_CONTAINER:
            sprintf(path, "%s:%s", lys_node_module(siter)->name, siter->name);
            nodeset = NULL;
            nodeset = lyd_get_node(subroot, path);
            path[0] = '\0';
            if (!nodeset) {
                return EXIT_FAILURE;
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
                return EXIT_FAILURE;
            }

            break;
        case LYS_LEAF:
            sprintf(path, "%s:%s", lys_node_module(siter)->name, siter->name);
            lyd_wd_add_leaf(siter->module->ctx, subroot, (struct lys_node_leaf *)siter, path, unres, options, 1);
            if (ly_errno != LY_SUCCESS) {
                LOGVAL(LYE_SPEC, LY_VLOG_LYD, subroot, "Creating default element \"%s\" failed.", path);
                path[0] = '\0';
                return EXIT_FAILURE;
            } /* else if default, it was already connected into parent */
            path[0] = '\0';
            break;
        case LYS_CHOICE:
            if (((struct lys_node_choice *)siter)->dflt) {
                /* check that no case is instantiated */
                iter = lyd_wd_get_choice_inst(subroot->child, siter);
                if (!iter) {
                    /* go to the default case */
                    lyd_wd_add_inner(subroot, ((struct lys_node_choice *)siter)->dflt, unres, options);
                } else if (lys_parent(iter->schema)->nodetype == LYS_CASE) {
                    /* add missing default nodes from present choice case */
                    lyd_wd_add_inner(subroot, lys_parent(iter->schema)->child, unres, options);
                } else { /* shorthand case */
                    if (!(iter->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
                        /* go into */
                        lyd_wd_add_inner(iter, iter->schema->child, unres, options);
                    }
                }
                if (ly_errno != LY_SUCCESS) {
                    return EXIT_FAILURE;
                }
            }
            break;
        case LYS_USES:
        case LYS_CASE:
            /* go into */
            lyd_wd_add_inner(subroot, siter->child, unres, options);
            if (ly_errno != LY_SUCCESS) {
                return EXIT_FAILURE;
            }
            break;
        default:
            /* do nothing */
            break;
        }

        if (siter->parent && lys_parent(siter)->nodetype == LYS_CHOICE) {
            /* only the default case is processed */
            return EXIT_SUCCESS;
        }
    }

    return EXIT_SUCCESS;
}

int
lyd_wd_top(struct ly_ctx *ctx, struct lyd_node **root, struct unres_data *unres, int options)
{
    struct lys_node *siter;
    struct lyd_node *iter;
    struct ly_set *modset = NULL, *nodeset;
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

    /* initiate internal buffer */
    if (ly_buf_used && path[0]) {
        buf_backup = strndup(path, LY_BUF_SIZE - 1);
    }
    ly_buf_used++;
    path[0] = '\0';

    modset = ly_set_new();
    LY_TREE_FOR(*root, iter) {
        if (!ctx) {
            ly_set_add(modset, lys_node_module(iter->schema));
        }
        if (options & (LYD_OPT_CONFIG | LYD_OPT_EDIT | LYD_OPT_GETCONFIG)) {
            /* do not process status data */
            if (iter->schema->flags & LYS_CONFIG_R) {
                continue;
            }
        }

        if (iter->schema->nodetype & (LYS_CONTAINER | LYS_LIST)) {
            if ((options & LYD_WD_MASK) == LYD_WD_EXPLICIT
                    && ((iter->schema->flags & LYS_CONFIG_W) && !(iter->schema->flags & LYS_INCL_STATUS))) {
                /* do not process config data in explicit mode */
                continue;
            }
            /* go into */
            if (lyd_wd_add_inner(iter, iter->schema->child, unres, options)) {
                goto error;
            }
        }
    }

    if (ctx) {
        /* add modules into our internal set */
        for (i = 0; i < (unsigned int)ctx->models.used; i++) {
            if (ctx->models.list[i]->data) {
                ly_set_add(modset, ctx->models.list[i]);
            }
        }
    }

    /* add missing top-level default nodes */
    for (i = 0; i < modset->number; i++) {
        LOGDBG("DEFAULTS: adding top level defaults for %s module, mode %x", ((struct lys_module *)modset->set.g[i])->name,
               (options & LYD_WD_MASK));
        LY_TREE_FOR(((struct lys_module *)modset->set.g[i])->data, siter) {
            if  (options & (LYD_OPT_CONFIG | LYD_OPT_EDIT | LYD_OPT_GETCONFIG)) {
                /* do not process status data */
                if (siter->flags & LYS_CONFIG_R) {
                    continue;
                }
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
                        goto error;
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
                        goto error;
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
                    goto error;
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
            case LYS_CHOICE:
                if (((struct lys_node_choice *)siter)->dflt) {
                    /* check that no case is instantiated */
                    iter = lyd_wd_get_choice_inst(*root, siter);
                    if (!iter) {
                        /* go to the default case */
                        iter = lyd_wd_add_empty(NULL, ((struct lys_node_choice *)siter)->dflt, unres, options);
                        if (ly_errno != LY_SUCCESS) {
                            goto error;
                        }
                    }
                }
                break;
            case LYS_USES:
                /* go into */
                iter = lyd_wd_add_empty(NULL, siter->child, unres, options);
                if (ly_errno != LY_SUCCESS) {
                    goto error;
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
                        goto error;
                    }
                }
            }

        }
    }

    ret = EXIT_SUCCESS;

error:
    /* cleanup */
    ly_set_free(modset);

    if (buf_backup) {
        /* return previous internal buffer content */
        strcpy(path, buf_backup);
        free(buf_backup);
    }
    ly_buf_used--;

    return ret;
}

API int
lyd_wd_add(struct ly_ctx *ctx, struct lyd_node **root, int options)
{
    int rc, mode;
    struct unres_data *unres = NULL;

    if (!root || (!ctx && !(*root))) {
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
    rc = lyd_wd_top(ctx, root, unres, options);
    if (unres && unres->count && resolve_unres_data(unres, root, options)) {
        rc = EXIT_FAILURE;
    }

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
