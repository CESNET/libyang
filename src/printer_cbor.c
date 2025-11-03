/**
 * @file printer_cbor.c
 * @author Meher Rushi <meherrushi2@gmail.com>
 * @brief CBOR printer for libyang data structure
 *
 * Copyright (c) 2015 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#ifdef ENABLE_CBOR_SUPPORT

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "log.h"
#include "ly_common.h"
#include "out.h"
#include "out_internal.h"
#include "parser_data.h"
#include "plugins_exts/metadata.h"
#include "plugins_types.h"
#include "printer_data.h"
#include "printer_internal.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "cbor.h"

/**
 * @brief CBOR printer context.
 */
struct cborpr_ctx {
    struct ly_out *out;                     /**< output specification */
    const struct lyd_node *root;            /**< root node of the subtree being printed */
    const struct lyd_node *parent;          /**< parent of the node being printed */
    uint32_t options;                       /**< [Data printer flags](@ref dataprinterflags) */
    const struct ly_ctx *ctx;               /**< libyang context */

    struct ly_set open;                     /**< currently open array(s) */
    const struct lyd_node *first_leaflist;  /**< first printed leaf-list instance, used when printing its metadata/attributes */
    
    cbor_item_t *root_map;                  /**< root CBOR map */
};

static LY_ERR cbor_print_node(struct cborpr_ctx *pctx, const struct lyd_node *node, cbor_item_t *parent_map);

/**
 * @brief Compare 2 nodes, despite it is regular data node or an opaq node, and
 * decide if they corresponds to the same schema node.
 *
 * @return 1 - matching nodes, 0 - non-matching nodes
 */
static int
matching_node(const struct lyd_node *node1, const struct lyd_node *node2)
{
    assert(node1 || node2);

    if (!node1 || !node2) {
        return 0;
    } else if (node1->schema != node2->schema) {
        return 0;
    }
    if (!node1->schema) {
        /* compare node names */
        struct lyd_node_opaq *onode1 = (struct lyd_node_opaq *)node1;
        struct lyd_node_opaq *onode2 = (struct lyd_node_opaq *)node2;

        if ((onode1->name.name != onode2->name.name) || (onode1->name.prefix != onode2->name.prefix)) {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Open a CBOR array for the specified @p node
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] node First node of the array.
 * @return LY_ERR value.
 */
static LY_ERR
cbor_print_array_open(struct cborpr_ctx *pctx, const struct lyd_node *node)
{
    LY_CHECK_RET(ly_set_add(&pctx->open, (void *)node, 0, NULL));
    return LY_SUCCESS;
}

/**
 * @brief Get know if the array for the provided @p node is currently open.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] node Data node to check.
 * @return 1 in case the printer is currently in the array belonging to the provided @p node.
 * @return 0 in case the provided @p node is not connected with the currently open array (or there is no open array).
 */
static int
is_open_array(struct cborpr_ctx *pctx, const struct lyd_node *node)
{
    if (pctx->open.count && matching_node(node, pctx->open.dnodes[pctx->open.count - 1])) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief Close the most inner CBOR array.
 *
 * @param[in] pctx CBOR printer context.
 */
static void
cbor_print_array_close(struct cborpr_ctx *pctx)
{
    ly_set_rm_index(&pctx->open, pctx->open.count - 1, NULL);
}

/**
 * @brief Get the node's module name to use as the @p node prefix in CBOR.
 *
 * @param[in] node Node to process.
 * @return The name of the module where the @p node belongs, it can be NULL in case the module name
 * cannot be determined (source format is XML and the refered namespace is unknown/not implemented in the current context).
 */
static const char *
node_prefix(const struct lyd_node *node)
{
    if (node->schema) {
        return node->schema->module->name;
    } else {
        struct lyd_node_opaq *onode = (struct lyd_node_opaq *)node;
        const struct lys_module *mod;

        switch (onode->format) {
        case LY_VALUE_CBOR:
        case LY_VALUE_JSON:
            return onode->name.module_name;
        case LY_VALUE_XML:
            mod = ly_ctx_get_module_implemented_ns(onode->ctx, onode->name.module_ns);
            if (!mod) {
                return NULL;
            }
            return mod->name;
        default:
            /* cannot be created */
            LOGINT(LYD_CTX(node));
        }
    }

    return NULL;
}

/**
 * @brief Compare 2 nodes if the belongs to the same module (if they come from the same namespace)
 *
 * Accepts both regulard a well as opaq nodes.
 *
 * @param[in] node1 The first node to compare.
 * @param[in] node2 The second node to compare.
 * @return 0 in case the nodes' modules are the same
 * @return 1 in case the nodes belongs to different modules
 */
int
cbor_nscmp(const struct lyd_node *node1, const struct lyd_node *node2)
{
    assert(node1 || node2);

    if (!node1 || !node2) {
        return 1;
    } else if (node1->schema && node2->schema) {
        if (node1->schema->module == node2->schema->module) {
            /* belongs to the same module */
            return 0;
        } else {
            /* different modules */
            return 1;
        }
    } else {
        const char *pref1 = node_prefix(node1);
        const char *pref2 = node_prefix(node2);

        if ((pref1 && pref2) && (pref1 == pref2)) {
            return 0;
        } else {
            return 1;
        }
    }
}

/**
 * @brief Create CBOR member name as [prefix:]name
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] node The data node being printed.
 * @param[in] is_attr Flag if the metadata sign (@) is supposed to be added before the identifier.
 * @return Newly allocated string with the member name, NULL on error.
 */
static char *
cbor_print_member_name(struct cborpr_ctx *pctx, const struct lyd_node *node, ly_bool is_attr)
{
    char *name = NULL;
    const char *prefix_str = node_prefix(node);
    const char *node_name = node->schema->name;
    
    if (cbor_nscmp(node, pctx->parent)) {
        /* print "namespace" */
        if (is_attr) {
            if (asprintf(&name, "@%s:%s", prefix_str, node_name) == -1) {
                return NULL;
            }
        } else {
            if (asprintf(&name, "%s:%s", prefix_str, node_name) == -1) {
                return NULL;
            }
        }
    } else {
        if (is_attr) {
            if (asprintf(&name, "@%s", node_name) == -1) {
                return NULL;
            }
        } else {
            name = strdup(node_name);
        }
    }
    
    return name;
}

/**
 * @brief More generic alternative to cbor_print_member_name() to print some special cases of the member names.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] parent Parent node to compare modules deciding if the prefix is printed.
 * @param[in] format Format to decide how to process the @p prefix.
 * @param[in] name Name structure to provide name and prefix to print. If NULL, only "" name is printed.
 * @param[in] is_attr Flag if the metadata sign (@) is supposed to be added before the identifier.
 * @return Newly allocated string with the member name, NULL on error.
 */
static char *
cbor_print_member_name2(struct cborpr_ctx *pctx, const struct lyd_node *parent, LY_VALUE_FORMAT format,
        const struct ly_opaq_name *name, ly_bool is_attr)
{
    const char *module_name = NULL, *name_str;
    char *result = NULL;

    /* determine prefix string */
    if (name) {
        switch (format) {
        case LY_VALUE_CBOR:
        case LY_VALUE_JSON:
            module_name = name->module_name;
            break;
        case LY_VALUE_XML: {
            const struct lys_module *mod = NULL;

            if (name->module_ns) {
                mod = ly_ctx_get_module_implemented_ns(pctx->ctx, name->module_ns);
            }
            if (mod) {
                module_name = mod->name;
            }
            break;
        }
        default:
            /* cannot be created */
            LOGINT_RET(pctx->ctx);
        }

        name_str = name->name;
    } else {
        name_str = "";
    }

    /* create the member name */
    if (module_name && (!parent || (node_prefix(parent) != module_name))) {
        if (is_attr) {
            if (asprintf(&result, "@%s:%s", module_name, name_str) == -1) {
                return NULL;
            }
        } else {
            if (asprintf(&result, "%s:%s", module_name, name_str) == -1) {
                return NULL;
            }
        }
    } else {
        if (is_attr) {
            if (asprintf(&result, "@%s", name_str) == -1) {
                return NULL;
            }
        } else {
            result = strdup(name_str);
        }
    }

    return result;
}

/**
 * @brief Print data value to CBOR item.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] ctx Context used to print the value.
 * @param[in] val Data value to be printed.
 * @param[in] local_mod Module of the current node.
 * @param[out] item_p Pointer to store the created CBOR item.
 * @return LY_ERR value.
 */
static LY_ERR
cbor_print_value(struct cborpr_ctx *pctx, const struct ly_ctx *ctx, const struct lyd_value *val,
        const struct lys_module *local_mod, cbor_item_t **item_p)
{
    ly_bool dynamic;
    LY_DATA_TYPE basetype;
    const char *value;
    cbor_item_t *item = NULL;

    value = val->realtype->plugin->print(ctx, val, LY_VALUE_JSON, (void *)local_mod, &dynamic, NULL);
    LY_CHECK_RET(!value, LY_EINVAL);
    basetype = val->realtype->basetype;

print_val:
    /* leafref is not supported */
    switch (basetype) {
    case LY_TYPE_UNION:
        /* use the resolved type */
        val = &val->subvalue->value;
        basetype = val->realtype->basetype;
        goto print_val;

    case LY_TYPE_BINARY:
    case LY_TYPE_STRING:
    case LY_TYPE_BITS:
    case LY_TYPE_ENUM:
    case LY_TYPE_INST:
    case LY_TYPE_IDENT:
        /* string types */
        item = cbor_build_string(value);
        break;

    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
    case LY_TYPE_DEC64: {
        /* numeric types stored as strings in CBOR */
        item = cbor_build_string(value);
        break;
    }

    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32: {
        /* signed integer types */
        int64_t num = strtoll(value, NULL, 10);
        if (num >= 0) {
            item = cbor_build_uint64(num);
        } else {
            item = cbor_build_negint64(-num - 1);
        }
        break;
    }

    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32: {
        /* unsigned integer types */
        uint64_t num = strtoull(value, NULL, 10);
        item = cbor_build_uint64(num);
        break;
    }

    case LY_TYPE_BOOL:
        /* boolean */
        if (strcmp(value, "true") == 0) {
            item = cbor_build_bool(true);
        } else {
            item = cbor_build_bool(false);
        }
        break;

    case LY_TYPE_EMPTY:
        /* empty type is represented as [null] */
        item = cbor_new_definite_array(1);
        if (item) {
            cbor_item_t *null_item = cbor_build_ctrl(CBOR_CTRL_NULL);
            if (null_item) {
                cbor_array_push(item, null_item);
                cbor_decref(&null_item);
            }
        }
        break;

    default:
        /* error */
        LOGINT_RET(pctx->ctx);
    }

    if (dynamic) {
        free((char *)value);
    }

    *item_p = item;
    return item ? LY_SUCCESS : LY_EMEM;
}

/**
 * @brief Print all the attributes of the opaq node to CBOR map.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] node Opaq node where the attributes are placed.
 * @param[in] attr_map CBOR map to add attributes to.
 * @return LY_ERR value.
 */
static LY_ERR
cbor_print_attribute(struct cborpr_ctx *pctx, const struct lyd_node_opaq *node, cbor_item_t *attr_map)
{
    struct lyd_attr *attr;
    cbor_item_t *value_item = NULL;

    for (attr = node->attr; attr; attr = attr->next) {
        char *key = cbor_print_member_name2(pctx, &node->node, attr->format, &attr->name, 0);
        LY_CHECK_RET(!key, LY_EMEM);

        if (attr->hints & (LYD_VALHINT_STRING | LYD_VALHINT_OCTNUM | LYD_VALHINT_HEXNUM | LYD_VALHINT_NUM64)) {
            value_item = cbor_build_string(attr->value);
        } else if (attr->hints & (LYD_VALHINT_BOOLEAN | LYD_VALHINT_DECNUM)) {
            if (strcmp(attr->value, "true") == 0) {
                value_item = cbor_build_bool(true);
            } else if (strcmp(attr->value, "false") == 0) {
                value_item = cbor_build_bool(false);
            } else {
                /* numeric value as string */
                value_item = cbor_build_string(attr->value);
            }
        } else if (attr->hints & LYD_VALHINT_EMPTY) {
            value_item = cbor_new_definite_array(1);
            if (value_item) {
                cbor_item_t *null_item = cbor_build_ctrl(CBOR_CTRL_NULL);
                if (null_item) {
                    cbor_array_push(value_item, null_item);
                    cbor_decref(&null_item);
                }
            }
        } else {
            /* unknown value format with no hints, use universal string */
            value_item = cbor_build_string(attr->value);
        }

        if (!value_item) {
            free(key);
            return LY_EMEM;
        }

        struct cbor_pair pair = {
            .key = cbor_move(cbor_build_string(key)),
            .value = cbor_move(value_item)
        };
        free(key);

        if (!cbor_map_add(attr_map, pair)) {
            cbor_decref(&pair.key);
            cbor_decref(&pair.value);
            return LY_EMEM;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Print all the metadata of the node to CBOR map.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] node Node where the metadata are placed.
 * @param[in] wdmod With-defaults module to mark that default attribute is supposed to be printed.
 * @param[in] meta_map CBOR map to add metadata to.
 * @return LY_ERR value.
 */
static LY_ERR
cbor_print_metadata(struct cborpr_ctx *pctx, const struct lyd_node *node, const struct lys_module *wdmod,
        cbor_item_t *meta_map)
{
    struct lyd_meta *meta;
    cbor_item_t *value_item = NULL;
    char *key = NULL;

    if (wdmod) {
        if (asprintf(&key, "%s:default", wdmod->name) == -1) {
            return LY_EMEM;
        }
        value_item = cbor_build_bool(true);
        if (!value_item) {
            free(key);
            return LY_EMEM;
        }

        struct cbor_pair pair = {
            .key = cbor_move(cbor_build_string(key)),
            .value = cbor_move(value_item)
        };
        free(key);

        if (!cbor_map_add(meta_map, pair)) {
            cbor_decref(&pair.key);
            cbor_decref(&pair.value);
            return LY_EMEM;
        }
    }

    for (meta = node->meta; meta; meta = meta->next) {
        if (!lyd_metadata_should_print(meta)) {
            continue;
        }

        if (asprintf(&key, "%s:%s", meta->annotation->module->name, meta->name) == -1) {
            return LY_EMEM;
        }

        LY_CHECK_RET(cbor_print_value(pctx, LYD_CTX(node), &meta->value, NULL, &value_item));

        struct cbor_pair pair = {
            .key = cbor_move(cbor_build_string(key)),
            .value = cbor_move(value_item)
        };
        free(key);

        if (!cbor_map_add(meta_map, pair)) {
            cbor_decref(&pair.key);
            cbor_decref(&pair.value);
            return LY_EMEM;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Check if a value can be printed for at least one metadata.
 *
 * @param[in] node Node to check.
 * @return 1 if node has printable meta otherwise 0.
 */
static ly_bool
node_has_printable_meta(const struct lyd_node *node)
{
    struct lyd_meta *iter;

    if (!node->meta) {
        return 0;
    }

    LY_LIST_FOR(node->meta, iter) {
        if (lyd_metadata_should_print(iter)) {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Print attributes/metadata of the given @p node to CBOR map.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] node Data node where the attributes/metadata are placed.
 * @param[in] parent_map CBOR map to add the metadata to.
 * @param[in] inner Flag if the @p node is an inner node in the tree.
 * @return LY_ERR value.
 */
static LY_ERR
cbor_print_attributes(struct cborpr_ctx *pctx, const struct lyd_node *node, cbor_item_t *parent_map, ly_bool inner)
{
    const struct lys_module *wdmod = NULL;
    cbor_item_t *meta_map = NULL;
    char *key = NULL;

    if (node->schema && (node->schema->nodetype != LYS_CONTAINER) && (((node->flags & LYD_DEFAULT) &&
            (pctx->options & (LYD_PRINT_WD_ALL_TAG | LYD_PRINT_WD_IMPL_TAG))) ||
            ((pctx->options & LYD_PRINT_WD_ALL_TAG) && lyd_is_default(node)))) {
        /* we have implicit OR explicit default node */
        /* get with-defaults module */
        wdmod = ly_ctx_get_module_implemented(LYD_CTX(node), "ietf-netconf-with-defaults");
    }

    if (node->schema && (wdmod || node_has_printable_meta(node))) {
        meta_map = cbor_new_indefinite_map();
        LY_CHECK_RET(!meta_map, LY_EMEM);

        LY_CHECK_RET(cbor_print_metadata(pctx, node, wdmod, meta_map));

        if (inner) {
            key = cbor_print_member_name2(pctx, lyd_parent(node), LY_VALUE_JSON, NULL, 1);
        } else {
            key = cbor_print_member_name(pctx, node, 1);
        }
        LY_CHECK_RET(!key, LY_EMEM);

        struct cbor_pair pair = {
            .key = cbor_move(cbor_build_string(key)),
            .value = cbor_move(meta_map)
        };
        free(key);

        if (!cbor_map_add(parent_map, pair)) {
            cbor_decref(&pair.key);
            cbor_decref(&pair.value);
            return LY_EMEM;
        }
    } else if (!node->schema && ((struct lyd_node_opaq *)node)->attr) {
        meta_map = cbor_new_indefinite_map();
        LY_CHECK_RET(!meta_map, LY_EMEM);

        LY_CHECK_RET(cbor_print_attribute(pctx, (struct lyd_node_opaq *)node, meta_map));

        if (inner) {
            key = cbor_print_member_name2(pctx, lyd_parent(node), LY_VALUE_JSON, NULL, 1);
        } else {
            key = cbor_print_member_name2(pctx, lyd_parent(node), ((struct lyd_node_opaq *)node)->format,
                    &((struct lyd_node_opaq *)node)->name, 1);
        }
        LY_CHECK_RET(!key, LY_EMEM);

        struct cbor_pair pair = {
            .key = cbor_move(cbor_build_string(key)),
            .value = cbor_move(meta_map)
        };
        free(key);

        if (!cbor_map_add(parent_map, pair)) {
            cbor_decref(&pair.key);
            cbor_decref(&pair.value);
            return LY_EMEM;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Print leaf data node including its metadata.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] node Data node to print.
 * @param[in] parent_map CBOR map to add the leaf to.
 * @return LY_ERR value.
 */
static LY_ERR
cbor_print_leaf(struct cborpr_ctx *pctx, const struct lyd_node *node, cbor_item_t *parent_map)
{
    char *key = NULL;
    cbor_item_t *value_item = NULL;

    key = cbor_print_member_name(pctx, node, 0);
    LY_CHECK_RET(!key, LY_EMEM);

    LY_CHECK_ERR_RET(cbor_print_value(pctx, LYD_CTX(node), &((const struct lyd_node_term *)node)->value,
            node->schema->module, &value_item), free(key), LY_EINVAL);

    struct cbor_pair pair = {
        .key = cbor_move(cbor_build_string(key)),
        .value = cbor_move(value_item)
    };
    
    if (!cbor_map_add(parent_map, pair)) {
        free(key);
        cbor_decref(&pair.key);
        cbor_decref(&pair.value);
        return LY_EMEM;
    }
    free(key);

    /* print attributes as sibling */
    cbor_print_attributes(pctx, node, parent_map, 0);

    return LY_SUCCESS;
}

/**
 * @brief Print anydata/anyxml content to CBOR item.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] any Anydata node to print.
 * @param[out] item_p Pointer to store the created CBOR item.
 * @return LY_ERR value.
 */
static LY_ERR
cbor_print_any_content(struct cborpr_ctx *pctx, struct lyd_node_any *any, cbor_item_t **item_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *iter;
    const struct lyd_node *prev_parent;
    uint32_t prev_opts, *prev_lo, temp_lo = 0;
    cbor_item_t *content_map = NULL;

    assert(any->schema->nodetype & LYD_NODE_ANY);

    if ((any->schema->nodetype == LYS_ANYDATA) && (any->value_type != LYD_ANYDATA_DATATREE)) {
        LOGINT_RET(pctx->ctx);
    }
    if (any->value_type == LYD_ANYDATA_LYB) {
        uint32_t parser_options = LYD_PARSE_ONLY | LYD_PARSE_OPAQ | LYD_PARSE_STRICT;

        /* turn logging off */
        prev_lo = ly_temp_log_options(&temp_lo);

        /* try to parse it into a data tree */
        if (lyd_parse_data_mem(pctx->ctx, any->value.mem, LYD_LYB, parser_options, 0, &iter) == LY_SUCCESS) {
            /* successfully parsed */
            free(any->value.mem);
            any->value.tree = iter;
            any->value_type = LYD_ANYDATA_DATATREE;
        }

        /* turn logging on again */
        ly_temp_log_options(prev_lo);
    }

    switch (any->value_type) {
    case LYD_ANYDATA_DATATREE:
        /* create a map for the content */
        content_map = cbor_new_indefinite_map();
        LY_CHECK_RET(!content_map, LY_EMEM);

        /* print data tree */
        prev_parent = pctx->parent;
        prev_opts = pctx->options;
        pctx->parent = &any->node;
        pctx->options &= ~LYD_PRINT_WITHSIBLINGS;
        LY_LIST_FOR(any->value.tree, iter) {
            ret = cbor_print_node(pctx, iter, content_map);
            LY_CHECK_ERR_RET(ret, cbor_decref(&content_map), ret);
        }
        pctx->parent = prev_parent;
        pctx->options = prev_opts;

        *item_p = content_map;
        break;
    case LYD_ANYDATA_JSON:
        if (!any->value.json) {
            /* no content */
            if (any->schema->nodetype == LYS_ANYXML) {
                *item_p = cbor_build_ctrl(CBOR_CTRL_NULL);
            } else {
                *item_p = cbor_new_indefinite_map();
            }
        } else {
            /* JSON content - store as string */
            *item_p = cbor_build_string(any->value.json);
        }
        break;
    case LYD_ANYDATA_STRING:
    case LYD_ANYDATA_XML:
        if (!any->value.str) {
            /* no content */
            if (any->schema->nodetype == LYS_ANYXML) {
                *item_p = cbor_build_ctrl(CBOR_CTRL_NULL);
            } else {
                *item_p = cbor_new_indefinite_map();
            }
        } else {
            /* print as a string */
            *item_p = cbor_build_string(any->value.str);
        }
        break;
    case LYD_ANYDATA_LYB:
        /* LYB format is not supported */
        LOGWRN(pctx->ctx, "Unable to print anydata content (type %d) as CBOR.", any->value_type);
        *item_p = cbor_build_ctrl(CBOR_CTRL_NULL);
        break;
    }

    return LY_SUCCESS;
}

/**
 * @brief Print content of a single container/list data node including its metadata.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] node Data node to print.
 * @param[out] item_p Pointer to store the created CBOR map.
 * @return LY_ERR value.
 */
static LY_ERR
cbor_print_inner(struct cborpr_ctx *pctx, const struct lyd_node *node, cbor_item_t **item_p)
{
    struct lyd_node *child;
    const struct lyd_node *prev_parent;
    cbor_item_t *inner_map = NULL;

    /* create map for inner node */
    inner_map = cbor_new_indefinite_map();
    LY_CHECK_RET(!inner_map, LY_EMEM);

    /* print attributes first */
    cbor_print_attributes(pctx, node, inner_map, 1);

    /* print children */
    prev_parent = pctx->parent;
    pctx->parent = node;
    LY_LIST_FOR(lyd_child(node), child) {
        LY_CHECK_ERR_RET(cbor_print_node(pctx, child, inner_map), cbor_decref(&inner_map), LY_EINVAL);
    }
    pctx->parent = prev_parent;

    *item_p = inner_map;
    return LY_SUCCESS;
}

/**
 * @brief Print container data node including its metadata.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] node Data node to print.
 * @param[in] parent_map CBOR map to add the container to.
 * @return LY_ERR value.
 */
static LY_ERR
cbor_print_container(struct cborpr_ctx *pctx, const struct lyd_node *node, cbor_item_t *parent_map)
{
    char *key = NULL;
    cbor_item_t *inner_map = NULL;

    key = cbor_print_member_name(pctx, node, 0);
    LY_CHECK_RET(!key, LY_EMEM);

    LY_CHECK_ERR_RET(cbor_print_inner(pctx, node, &inner_map), free(key), LY_EINVAL);

    struct cbor_pair pair = {
        .key = cbor_move(cbor_build_string(key)),
        .value = cbor_move(inner_map)
    };
    
    if (!cbor_map_add(parent_map, pair)) {
        free(key);
        cbor_decref(&pair.key);
        cbor_decref(&pair.value);
        return LY_EMEM;
    }
    free(key);

    return LY_SUCCESS;
}

/**
 * @brief Print anydata/anyxml data node including its metadata.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] node Data node to print.
 * @param[in] parent_map CBOR map to add the anydata to.
 * @return LY_ERR value.
 */
static LY_ERR
cbor_print_any(struct cborpr_ctx *pctx, const struct lyd_node *node, cbor_item_t *parent_map)
{
    char *key = NULL;
    cbor_item_t *any_item = NULL;

    key = cbor_print_member_name(pctx, node, 0);
    LY_CHECK_RET(!key, LY_EMEM);

    LY_CHECK_ERR_RET(cbor_print_any_content(pctx, (struct lyd_node_any *)node, &any_item), free(key), LY_EINVAL);

    struct cbor_pair pair = {
        .key = cbor_move(cbor_build_string(key)),
        .value = cbor_move(any_item)
    };
    
    if (!cbor_map_add(parent_map, pair)) {
        free(key);
        cbor_decref(&pair.key);
        cbor_decref(&pair.value);
        return LY_EMEM;
    }
    free(key);

    /* print attributes as sibling */
    cbor_print_attributes(pctx, node, parent_map, 0);

    return LY_SUCCESS;
}

/**
 * @brief Check whether a node is the last printed instance of a (leaf-)list.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] node Last printed node.
 * @return Whether it is the last printed instance or not.
 */
static ly_bool
cbor_print_array_is_last_inst(struct cborpr_ctx *pctx, const struct lyd_node *node)
{
    if (!is_open_array(pctx, node)) {
        /* no array open */
        return 0;
    }

    if ((pctx->root == node) && !(pctx->options & LYD_PRINT_WITHSIBLINGS)) {
        /* the only printed instance */
        return 1;
    }

    if (!node->next || (node->next->schema != node->schema)) {
        /* last instance */
        return 1;
    }

    return 0;
}

/**
 * @brief Print single leaf-list or list instance.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] node Data node to print.
 * @param[in] parent_map CBOR map to add the node to.
 * @param[in,out] array_p Pointer to the array being built.
 * @return LY_ERR value.
 */
static LY_ERR
cbor_print_leaf_list(struct cborpr_ctx *pctx, const struct lyd_node *node, cbor_item_t *parent_map, cbor_item_t **array_p)
{
    const struct lys_module *wdmod = NULL;
    cbor_item_t *value_item = NULL;
    cbor_item_t *inner_map = NULL;
    char *key = NULL;

    if (!is_open_array(pctx, node)) {
        /* start new array */
        *array_p = cbor_new_indefinite_array();
        LY_CHECK_RET(!*array_p, LY_EMEM);

        key = cbor_print_member_name(pctx, node, 0);
        LY_CHECK_ERR_RET(!key, cbor_decref(array_p), LY_EMEM);

        LY_CHECK_RET(cbor_print_array_open(pctx, node));
    }

    if (node->schema->nodetype == LYS_LIST) {
        /* print list's content */
        LY_CHECK_RET(cbor_print_inner(pctx, node, &inner_map));
        cbor_array_push(*array_p, inner_map);
        cbor_decref(&inner_map);
    } else {
        assert(node->schema->nodetype == LYS_LEAFLIST);

        LY_CHECK_RET(cbor_print_value(pctx, LYD_CTX(node), &((const struct lyd_node_term *)node)->value,
                node->schema->module, &value_item));
        cbor_array_push(*array_p, value_item);
        cbor_decref(&value_item);

        if (!pctx->first_leaflist) {
            if (((node->flags & LYD_DEFAULT) && (pctx->options & (LYD_PRINT_WD_ALL_TAG | LYD_PRINT_WD_IMPL_TAG))) ||
                    ((pctx->options & LYD_PRINT_WD_ALL_TAG) && lyd_is_default(node))) {
                /* we have implicit OR explicit default node, get with-defaults module */
                wdmod = ly_ctx_get_module_implemented(LYD_CTX(node), "ietf-netconf-with-defaults");
            }
            if (wdmod || node_has_printable_meta(node)) {
                /* we will be printing metadata for these siblings */
                pctx->first_leaflist = node;
            }
        }
    }

    if (cbor_print_array_is_last_inst(pctx, node)) {
        /* add completed array to parent map */
        if (key) {
            struct cbor_pair pair = {
                .key = cbor_move(cbor_build_string(key)),
                .value = cbor_move(*array_p)
            };
            free(key);

            if (!cbor_map_add(parent_map, pair)) {
                cbor_decref(&pair.key);
                cbor_decref(&pair.value);
                return LY_EMEM;
            }
        }
        cbor_print_array_close(pctx);
        *array_p = NULL;
    }

    return LY_SUCCESS;
}

/**
 * @brief Print leaf-list's metadata or opaque nodes attributes.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] parent_map CBOR map to add metadata to.
 * @return LY_ERR value.
 */
static LY_ERR
cbor_print_meta_attr_leaflist(struct cborpr_ctx *pctx, cbor_item_t *parent_map)
{
    const struct lyd_node *prev, *node, *iter;
    const struct lys_module *wdmod = NULL, *iter_wdmod;
    const struct lyd_node_opaq *opaq = NULL;
    cbor_item_t *meta_array = NULL;
    cbor_item_t *meta_map = NULL;
    char *key = NULL;

    assert(pctx->first_leaflist);

    if (pctx->options & (LYD_PRINT_WD_ALL_TAG | LYD_PRINT_WD_IMPL_TAG)) {
        /* get with-defaults module */
        wdmod = ly_ctx_get_module_implemented(pctx->ctx, "ietf-netconf-with-defaults");
    }

    /* node is the first instance of the leaf-list */
    for (node = pctx->first_leaflist, prev = pctx->first_leaflist->prev;
            prev->next && matching_node(node, prev);
            node = prev, prev = node->prev) {}

    /* create metadata array */
    meta_array = cbor_new_indefinite_array();
    LY_CHECK_RET(!meta_array, LY_EMEM);

    if (node->schema) {
        key = cbor_print_member_name(pctx, node, 1);
    } else {
        opaq = (struct lyd_node_opaq *)node;
        key = cbor_print_member_name2(pctx, lyd_parent(node), opaq->format, &opaq->name, 1);
    }
    LY_CHECK_ERR_RET(!key, cbor_decref(&meta_array), LY_EMEM);

    LY_LIST_FOR(node, iter) {
        if (iter->schema && ((iter->flags & LYD_DEFAULT) || ((pctx->options & LYD_PRINT_WD_ALL_TAG) && lyd_is_default(iter)))) {
            iter_wdmod = wdmod;
        } else {
            iter_wdmod = NULL;
        }

        if ((iter->schema && (node_has_printable_meta(iter) || iter_wdmod)) || (opaq && opaq->attr)) {
            meta_map = cbor_new_indefinite_map();
            if (!meta_map) {
                free(key);
                cbor_decref(&meta_array);
                return LY_EMEM;
            }

            if (iter->schema) {
                LY_CHECK_ERR_RET(cbor_print_metadata(pctx, iter, iter_wdmod, meta_map),
                        free(key); cbor_decref(&meta_array); cbor_decref(&meta_map), LY_EINVAL);
            } else {
                LY_CHECK_ERR_RET(cbor_print_attribute(pctx, (struct lyd_node_opaq *)iter, meta_map),
                        free(key); cbor_decref(&meta_array); cbor_decref(&meta_map), LY_EINVAL);
            }

            cbor_array_push(meta_array, meta_map);
            cbor_decref(&meta_map);
        } else {
            cbor_item_t *null_item = cbor_build_ctrl(CBOR_CTRL_NULL);
            cbor_array_push(meta_array, null_item);
            cbor_decref(&null_item);
        }

        if (!matching_node(iter, iter->next)) {
            break;
        }
    }

    /* add metadata array to parent map */
    struct cbor_pair pair = {
        .key = cbor_move(cbor_build_string(key)),
        .value = cbor_move(meta_array)
    };
    free(key);

    if (!cbor_map_add(parent_map, pair)) {
        cbor_decref(&pair.key);
        cbor_decref(&pair.value);
        return LY_EMEM;
    }

    return LY_SUCCESS;
}

/**
 * @brief Print opaq data node including its attributes.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] node Opaq node to print.
 * @param[in] parent_map CBOR map to add the node to.
 * @param[in,out] array_p Pointer to the array being built (for leaf-lists/lists).
 * @return LY_ERR value.
 */
static LY_ERR
cbor_print_opaq(struct cborpr_ctx *pctx, const struct lyd_node_opaq *node, cbor_item_t *parent_map, cbor_item_t **array_p)
{
    ly_bool first = 1, last = 1;
    uint32_t hints;
    char *key = NULL;
    cbor_item_t *value_item = NULL;

    if (node->hints == LYD_HINT_DATA) {
        /* useless and confusing hints */
        hints = 0;
    } else {
        hints = node->hints;
    }

    if (hints & (LYD_NODEHINT_LIST | LYD_NODEHINT_LEAFLIST)) {
        if (node->prev->next && matching_node(node->prev, &node->node)) {
            first = 0;
        }
        if (node->next && matching_node(&node->node, node->next)) {
            last = 0;
        }
    }

    if (first) {
        key = cbor_print_member_name2(pctx, pctx->parent, node->format, &node->name, 0);
        LY_CHECK_RET(!key, LY_EMEM);

        if (hints & (LYD_NODEHINT_LIST | LYD_NODEHINT_LEAFLIST)) {
            *array_p = cbor_new_indefinite_array();
            LY_CHECK_ERR_RET(!*array_p, free(key), LY_EMEM);
            LY_CHECK_ERR_RET(cbor_print_array_open(pctx, &node->node), free(key), LY_EINVAL);
        }
    }

    if (node->child || (hints & LYD_NODEHINT_LIST) || (hints & LYD_NODEHINT_CONTAINER)) {
        cbor_item_t *inner_map = NULL;
        LY_CHECK_ERR_RET(cbor_print_inner(pctx, &node->node, &inner_map), free(key), LY_EINVAL);

        if (hints & (LYD_NODEHINT_LIST | LYD_NODEHINT_LEAFLIST)) {
            cbor_array_push(*array_p, inner_map);
            cbor_decref(&inner_map);
        } else {
            struct cbor_pair pair = {
                .key = cbor_move(cbor_build_string(key)),
                .value = cbor_move(inner_map)
            };
            free(key);
            key = NULL;

            if (!cbor_map_add(parent_map, pair)) {
                cbor_decref(&pair.key);
                cbor_decref(&pair.value);
                return LY_EMEM;
            }
        }
    } else {
        if (hints & LYD_VALHINT_EMPTY) {
            value_item = cbor_new_definite_array(1);
            if (value_item) {
                cbor_item_t *null_item = cbor_build_ctrl(CBOR_CTRL_NULL);
                if (null_item) {
                    cbor_array_push(value_item, null_item);
                    cbor_decref(&null_item);
                }
            }
        } else if ((hints & (LYD_VALHINT_BOOLEAN | LYD_VALHINT_DECNUM)) && !(hints & LYD_VALHINT_NUM64)) {
            if (strcmp(node->value, "true") == 0) {
                value_item = cbor_build_bool(true);
            } else if (strcmp(node->value, "false") == 0) {
                value_item = cbor_build_bool(false);
            } else {
                value_item = cbor_build_string(node->value);
            }
        } else {
            /* string or a large number */
            value_item = cbor_build_string(node->value);
        }

        LY_CHECK_ERR_RET(!value_item, free(key), LY_EMEM);

        if (hints & (LYD_NODEHINT_LIST | LYD_NODEHINT_LEAFLIST)) {
            cbor_array_push(*array_p, value_item);
            cbor_decref(&value_item);
        } else {
            struct cbor_pair pair = {
                .key = cbor_move(cbor_build_string(key)),
                .value = cbor_move(value_item)
            };
            free(key);
            key = NULL;

            if (!cbor_map_add(parent_map, pair)) {
                cbor_decref(&pair.key);
                cbor_decref(&pair.value);
                return LY_EMEM;
            }
        }

        if (!(hints & LYD_NODEHINT_LEAFLIST)) {
            /* attributes */
            cbor_print_attributes(pctx, (const struct lyd_node *)node, parent_map, 0);
        } else if (!pctx->first_leaflist && node->attr) {
            /* attributes printed later */
            pctx->first_leaflist = &node->node;
        }
    }

    if (last && (hints & (LYD_NODEHINT_LIST | LYD_NODEHINT_LEAFLIST))) {
        if (key) {
            struct cbor_pair pair = {
                .key = cbor_move(cbor_build_string(key)),
                .value = cbor_move(*array_p)
            };
            free(key);

            if (!cbor_map_add(parent_map, pair)) {
                cbor_decref(&pair.key);
                cbor_decref(&pair.value);
                return LY_EMEM;
            }
        }
        cbor_print_array_close(pctx);
        *array_p = NULL;
    }

    if (key) {
        free(key);
    }

    return LY_SUCCESS;
}

/**
 * @brief Print all the types of data node including its metadata.
 *
 * @param[in] pctx CBOR printer context.
 * @param[in] node Data node to print.
 * @param[in] parent_map CBOR map to add the node to.
 * @return LY_ERR value.
 */
static LY_ERR
cbor_print_node(struct cborpr_ctx *pctx, const struct lyd_node *node, cbor_item_t *parent_map)
{
    static cbor_item_t *array = NULL;

    if (!lyd_node_should_print(node, pctx->options)) {
        if (cbor_print_array_is_last_inst(pctx, node)) {
            cbor_print_array_close(pctx);
        }
        return LY_SUCCESS;
    }

    if (!node->schema) {
        LY_CHECK_RET(cbor_print_opaq(pctx, (const struct lyd_node_opaq *)node, parent_map, &array));
    } else {
        switch (node->schema->nodetype) {
        case LYS_RPC:
        case LYS_ACTION:
        case LYS_NOTIF:
        case LYS_CONTAINER:
            LY_CHECK_RET(cbor_print_container(pctx, node, parent_map));
            break;
        case LYS_LEAF:
            LY_CHECK_RET(cbor_print_leaf(pctx, node, parent_map));
            break;
        case LYS_LEAFLIST:
        case LYS_LIST:
            LY_CHECK_RET(cbor_print_leaf_list(pctx, node, parent_map, &array));
            break;
        case LYS_ANYDATA:
        case LYS_ANYXML:
            LY_CHECK_RET(cbor_print_any(pctx, node, parent_map));
            break;
        default:
            LOGINT(pctx->ctx);
            return EXIT_FAILURE;
        }
    }

    if (pctx->first_leaflist && !matching_node(node->next, pctx->first_leaflist)) {
        cbor_print_meta_attr_leaflist(pctx, parent_map);
        pctx->first_leaflist = NULL;
    }

    return LY_SUCCESS;
}

LY_ERR
cbor_print_data(struct ly_out *out, const struct lyd_node *root, uint32_t options)
{
    const struct lyd_node *node;
    struct cborpr_ctx pctx = {0};
    unsigned char *buffer = NULL;
    size_t buffer_size = 0;

    if (!root) {
        /* empty data - print empty map */
        cbor_item_t *empty_map = cbor_new_indefinite_map();
        LY_CHECK_RET(!empty_map, LY_EMEM);
        
        buffer_size = cbor_serialize_alloc(empty_map, &buffer, &buffer_size);
        cbor_decref(&empty_map);
        
        if (buffer_size == 0) {
            return LY_EMEM;
        }
        
        ly_write_(out, (const char *)buffer, buffer_size);
        free(buffer);
        ly_print_flush(out);
        return LY_SUCCESS;
    }

    pctx.out = out;
    pctx.parent = NULL;
    pctx.options = options;
    pctx.ctx = LYD_CTX(root);

    /* create root map */
    pctx.root_map = cbor_new_indefinite_map();
    LY_CHECK_RET(!pctx.root_map, LY_EMEM);

    /* print content */
    LY_LIST_FOR(root, node) {
        pctx.root = node;
        LY_CHECK_ERR_RET(cbor_print_node(&pctx, node, pctx.root_map), 
                cbor_decref(&pctx.root_map); ly_set_erase(&pctx.open, NULL), LY_EINVAL);
        if (!(options & LYD_PRINT_WITHSIBLINGS)) {
            break;
        }
    }

    /* serialize CBOR to buffer */
    buffer_size = cbor_serialize_alloc(pctx.root_map, &buffer, &buffer_size);
    cbor_decref(&pctx.root_map);

    if (buffer_size == 0) {
        ly_set_erase(&pctx.open, NULL);
        return LY_EMEM;
    }

    /* write to output */
    ly_write_(out, (const char *)buffer, buffer_size);
    free(buffer);

    assert(!pctx.open.count);
    ly_set_erase(&pctx.open, NULL);

    ly_print_flush(out);
    return LY_SUCCESS;
}

#endif /* ENABLE_CBOR_SUPPORT */