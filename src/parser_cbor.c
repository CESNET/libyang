/**
 * @file parser_cbor.c
 * @author Meher Rushi <meherrushi2@gmail.com>
 * @brief CBOR data parser for libyang
 *
 * Copyright (c) 2020 - 2023 CESNET, z.s.p.o.
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

#include "compat.h"
#include "context.h"
#include "dict.h"
#include "in_internal.h"
#include "lcbor.h"
#include "log.h"
#include "ly_common.h"
#include "parser_data.h"
#include "parser_internal.h"
#include "plugins_exts.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "validation.h"


#include <stdio.h>
#include <string.h>

void print_json(cbor_item_t *item);

void print_json_string(const cbor_item_t *item)
{
    size_t length = cbor_string_length(item);
    char *str = (char *)cbor_string_handle(item);
    printf("\"%.*s\"", (int)length, str);
}

void print_json_map(const cbor_item_t *item)
{
    printf("{");
    size_t size = cbor_map_size(item);
    struct cbor_pair *pairs = cbor_map_handle(item);

    for (size_t i = 0; i < size; ++i)
    {
        print_json(pairs[i].key);
        printf(": ");
        print_json(pairs[i].value);
        if (i < size - 1)
            printf(", ");
    }

    printf("}");
}

void print_json_bool(const cbor_item_t *item)
{
    printf(cbor_is_bool(item) && cbor_ctrl_value(item) ? "true" : "false");
}

void print_json(cbor_item_t *item)
{
    if (cbor_isa_map(item))
    {
        print_json_map(item);
    }
    else if (cbor_isa_string(item))
    {
        print_json_string(item);
    }
    else if (cbor_is_bool(item))
    {
        print_json_bool(item);
    }
    else
    {
        printf("null"); // fallback for unsupported types
    }
}

static LY_ERR lydcbor_parse_subtree(struct lyd_cbor_ctx *lydctx, struct lyd_node *parent,
                                   struct lyd_node **first_p, struct ly_set *parsed, const cbor_item_t *cbor_obj);

/**
 * @brief Free the CBOR parser context
 *
 * @param[in] lydctx Data parser context to free.
 */
static void
lyd_cbor_ctx_free(struct lyd_ctx *lydctx)
{
    struct lyd_cbor_ctx *ctx = (struct lyd_cbor_ctx *)lydctx;

    if (lydctx)
    {
        lyd_ctx_free(lydctx);
        lycbor_ctx_free(ctx->cborctx);
        free(ctx);
    }
}

/**
 * @brief Convert a CBOR item to a string representation.
 *
 * This function handles the low-level CBOR to string conversion,
 * similar to how JSON parser converts JSON values to strings.
 *
 * @param[in] item CBOR item to convert.
 * @param[out] str_val String value (allocated, caller must free).
 * @param[out] str_len String length.
 * @return LY_ERR value.
 */
static LY_ERR
lydcbor_item_to_string(const cbor_item_t *item, char **str_val, size_t *str_len)
{
    LY_ERR ret = LY_SUCCESS;

    assert(item && str_val && str_len);
    *str_val = NULL;
    *str_len = 0;

    switch (cbor_typeof(item))
    {
    case CBOR_TYPE_UINT:
    {
        uint64_t val = cbor_get_int(item);
        int len = snprintf(NULL, 0, "%" PRIu64, val);
        if (len < 0)
        {
            return LY_ESYS;
        }
        *str_val = malloc(len + 1);
        LY_CHECK_ERR_RET(!*str_val, LOGMEM(NULL), LY_EMEM);
        sprintf(*str_val, "%" PRIu64, val);
        *str_len = len;
        break;
    }
    case CBOR_TYPE_NEGINT:
    {
        int64_t val = -1 - (int64_t)cbor_get_int(item);
        int len = snprintf(NULL, 0, "%" PRId64, val);
        if (len < 0)
        {
            return LY_ESYS;
        }
        *str_val = malloc(len + 1);
        LY_CHECK_ERR_RET(!*str_val, LOGMEM(NULL), LY_EMEM);
        sprintf(*str_val, "%" PRId64, val);
        *str_len = len;
        break;
    }
    case CBOR_TYPE_BYTESTRING:
        *str_len = cbor_bytestring_length(item);
        *str_val = malloc(*str_len + 1);
        LY_CHECK_ERR_RET(!*str_val, LOGMEM(NULL), LY_EMEM);
        memcpy(*str_val, cbor_bytestring_handle(item), *str_len);
        (*str_val)[*str_len] = '\0';
        break;
    case CBOR_TYPE_STRING:
        *str_len = cbor_string_length(item);
        *str_val = malloc(*str_len + 1);
        LY_CHECK_ERR_RET(!*str_val, LOGMEM(NULL), LY_EMEM);
        memcpy(*str_val, cbor_string_handle(item), *str_len);
        (*str_val)[*str_len] = '\0';
        break;
    case CBOR_TYPE_FLOAT_CTRL:
        if (cbor_float_ctrl_is_ctrl(item))
        {
            switch (cbor_ctrl_value(item))
            {
            case CBOR_CTRL_TRUE:
                *str_val = strdup("true");
                *str_len = 4;
                break;
            case CBOR_CTRL_FALSE:
                *str_val = strdup("false");
                *str_len = 5;
                break;
            case CBOR_CTRL_NULL:
                *str_val = strdup("");
                *str_len = 0;
                break;
            default:
                LOGVAL(NULL, LYVE_SYNTAX, "Unsupported CBOR control value");
                ret = LY_EVALID;
                break;
            }
            LY_CHECK_ERR_RET(!*str_val, LOGMEM(NULL), LY_EMEM);
        }
        else
        {
            /* Float value */
            double val = cbor_float_get_float(item);
            int len = snprintf(NULL, 0, "%g", val);
            if (len < 0)
            {
                return LY_ESYS;
            }
            *str_val = malloc(len + 1);
            LY_CHECK_ERR_RET(!*str_val, LOGMEM(NULL), LY_EMEM);
            sprintf(*str_val, "%g", val);
            *str_len = len;
        }
        break;
    default:
        LOGVAL(NULL, LYVE_SYNTAX, "Unsupported CBOR data type %d", cbor_typeof(item));
        ret = LY_EVALID;
        break;
    }

    return ret;
}

/**
 * @brief Get string key from CBOR map item.
 *
 * For named identifier format, keys should be strings.
 * For SID format, keys would be integers (future implementation).
 */
static LY_ERR
lydcbor_get_key_string(struct lyd_cbor_ctx *lydctx, const cbor_item_t *key_item,
                       char **key_str, size_t *key_len)
{
    LY_ERR ret = LY_SUCCESS;

    assert(lydctx && key_item && key_str && key_len);

    switch (lydctx->cborctx->format)
    {
    case LYD_CBOR_NAMED:
        /* Keys must be strings for named format */
        if (!cbor_isa_string(key_item))
        {
            LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "CBOR map key must be string for named identifier format");
            return LY_EVALID;
        }
        ret = lydcbor_item_to_string(key_item, key_str, key_len);
        break;
    case LYD_CBOR_SID:
        /* Future: Handle SID integer keys */
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "CBOR SID format not yet implemented");
        ret = LY_ENOT;
        break;
    default:
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Unknown CBOR format");
        ret = LY_EINVAL;
        break;
    }

    return ret;
}

/**
 * @brief Parse a single CBOR value according to schema node.
 *
 * @param[in] lydctx CBOR parser context.
 * @param[in] snode Schema node for the value.
 * @param[in] cbor_item CBOR item to parse.
 * @param[out] node Created data node.
 * @return LY_ERR value.
 */
static LY_ERR
lydcbor_parse_value(struct lyd_cbor_ctx *lydctx, const struct lysc_node *snode,
                    const void *cbor_item, struct lyd_node **node)
{
    LY_ERR ret = LY_SUCCESS;
    const cbor_item_t *item = (const cbor_item_t *)cbor_item;
    char *str_val = NULL;
    size_t str_len = 0;

    assert(lydctx && snode && item && node);

    /* Convert CBOR value to string */
    LY_CHECK_GOTO(ret = lydcbor_item_to_string(item, &str_val, &str_len), cleanup);

    /* Create data node based on schema node type */
    switch (snode->nodetype)
    {
    case LYS_LEAF:
    case LYS_LEAFLIST:
        ret = lyd_create_term(snode, str_val, str_len, 0, 0, NULL, LY_VALUE_JSON, NULL, LYD_HINT_DATA, NULL, node);
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        /* For anydata/anyxml, we store the CBOR directly */
        ret = lyd_create_any(snode, cbor_item, LYD_ANYDATA_CBOR, 0, node);
        break;
    default:
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Invalid schema node type for CBOR value");
        ret = LY_EVALID;
        break;
    }

cleanup:
    free(str_val);
    return ret;
}

static LY_ERR
lydcbor_parse_node_value(struct lyd_cbor_ctx *lydctx, const struct lysc_node *snode,
                         struct lyd_node **node, const cbor_item_t *cbor_value)
{
    LY_ERR ret = LY_SUCCESS;

    assert(lydctx && snode && node && cbor_value);
    *node = NULL;

    switch (snode->nodetype)
    {
    case LYS_CONTAINER:
        ret = lyd_create_inner(snode, node);
        break;
    case LYS_LIST:
        ret = lyd_create_inner(snode, node);
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
    {
        char *str_val = NULL;
        size_t str_len = 0;

        ret = lydcbor_item_to_string(cbor_value, &str_val, &str_len);
        if (ret == LY_SUCCESS)
        {
            ret = lyd_create_term(snode, str_val, str_len, 0, 0, NULL, LY_VALUE_JSON, NULL, LYD_HINT_DATA, NULL, node);
        }
        free(str_val);
        break;
    }
    case LYS_ANYDATA:
    case LYS_ANYXML:
        ret = lyd_create_any(snode, cbor_value, LYD_ANYDATA_CBOR, 0, node);
        break;
    default:
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Invalid schema node type %d", snode->nodetype);
        ret = LY_EVALID;
        break;
    }

    return ret;
}

static LY_ERR
lydcbor_parse_terminal(struct lyd_cbor_ctx *lydctx, const struct lysc_node *snode,
                       const cbor_item_t *cbor_value, struct lyd_node **first_p, struct ly_set *parsed)
{
    LY_ERR ret;
    struct lyd_node *node = NULL;

    ret = lydcbor_parse_node_value(lydctx, snode, &node, cbor_value);
    LY_CHECK_RET(ret);

    if (!node)
    {
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Failed to create terminal node for \"%s\"", snode->name);
        return LY_EVALID;
    }

    /* Insert into tree */
    ret = lyd_insert_sibling(*first_p, node, first_p);
    if (ret)
    {
        lyd_free_tree(node);
        return ret;
    }

    /* Add to parsed set */
    return ly_set_add(parsed, node, 1, NULL);
}

static LY_ERR
lydcbor_parse_container(struct lyd_cbor_ctx *lydctx, const struct lysc_node *snode,
                        const cbor_item_t *cbor_value, struct lyd_node **first_p, struct ly_set *parsed)
{
    LY_ERR ret;
    struct lyd_node *node = NULL;

    ret = lyd_create_inner(snode, &node);
    LY_CHECK_RET(ret);

    if (!node)
    {
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Failed to create container node for \"%s\"", snode->name);
        return LY_EVALID;
    }

    /* Insert into tree first */
    ret = lyd_insert_sibling(*first_p, node, first_p);
    if (ret)
    {
        lyd_free_tree(node);
        return ret;
    }

    /* Add to parsed set */
    ret = ly_set_add(parsed, node, 1, NULL);
    LY_CHECK_RET(ret);

    /* Parse container children */
    if (cbor_isa_map(cbor_value) && cbor_map_size(cbor_value) > 0)
    {
        struct lyd_node *child_first = NULL;
        ret = lydcbor_parse_subtree(lydctx, node, &child_first, parsed, cbor_value);
        if (ret)
        {
            return ret;
        }

        /* Link children to container */
        if (child_first)
        {
            lyd_insert_child(node, child_first);
        }
    }

    return LY_SUCCESS;
}

static LY_ERR
lydcbor_parse_list_array(struct lyd_cbor_ctx *lydctx, const struct lysc_node *snode,
                         const cbor_item_t *array_item, struct lyd_node **first_p, struct ly_set *parsed)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *node = NULL;
    size_t array_size;
    cbor_item_t **array_handle;

    assert(lydctx && snode && array_item && parsed);

    if (!cbor_isa_array(array_item))
    {
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Expected CBOR array for list");
        return LY_EVALID;
    }

    if (snode->nodetype != LYS_LIST)
    {
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Schema node must be list");
        return LY_EVALID;
    }

    array_size = cbor_array_size(array_item);
    array_handle = cbor_array_handle(array_item);

    if (!array_handle && array_size > 0)
    {
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Invalid CBOR array structure");
        return LY_EVALID;
    }

    for (size_t i = 0; i < array_size; ++i)
    {
        const cbor_item_t *item = array_handle[i];

        if (!item)
        {
            LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Null array element at index %zu", i);
            ret = LY_EVALID;
            goto cleanup;
        }

        if (!cbor_isa_map(item))
        {
            LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "List entry must be a CBOR map");
            ret = LY_EVALID;
            goto cleanup;
        }

        ret = lyd_create_inner(snode, &node);
        LY_CHECK_GOTO(ret, cleanup);

        if (!node)
        {
            LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Failed to create list node");
            ret = LY_EVALID;
            goto cleanup;
        }

        /* Insert the list node */
        ret = lyd_insert_sibling(*first_p, node, first_p);
        LY_CHECK_GOTO(ret, cleanup);

        /* Add to parsed set */
        LY_CHECK_GOTO(ret = ly_set_add(parsed, node, 1, NULL), cleanup);

        /* Parse list entry content */
        struct lyd_node *child_first = NULL;
        ret = lydcbor_parse_subtree(lydctx, node, &child_first, parsed, item);
        LY_CHECK_GOTO(ret, cleanup);

        /* Link children to list entry */
        if (child_first)
        {
            lyd_insert_child(node, child_first);
        }

        node = NULL; /* Reset pointer after successful processing */
    }

cleanup:
    if (ret && node)
    {
        lyd_free_tree(node);
    }
    return ret;
}

static LY_ERR
lydcbor_parse_list(struct lyd_cbor_ctx *lydctx, const struct lysc_node *snode,
                   const cbor_item_t *cbor_value, struct lyd_node **first_p, struct ly_set *parsed)
{
    LY_ERR ret = LY_SUCCESS;

    if (cbor_isa_array(cbor_value))
    {
        /* Array of list entries */
        ret = lydcbor_parse_list_array(lydctx, snode, cbor_value, first_p, parsed);
    }
    else if (cbor_isa_map(cbor_value))
    {
        /* Single list entry */
        struct lyd_node *node = NULL;

        ret = lyd_create_inner(snode, &node);
        LY_CHECK_RET(ret);

        /* Insert into tree */
        ret = lyd_insert_sibling(*first_p, node, first_p);
        if (ret)
        {
            lyd_free_tree(node);
            return ret;
        }

        /* Add to parsed set */
        ret = ly_set_add(parsed, node, 1, NULL);
        LY_CHECK_RET(ret);

        /* Parse list entry content */
        struct lyd_node *child_first = NULL;
        ret = lydcbor_parse_subtree(lydctx, node, &child_first, parsed, cbor_value);
        if (ret)
        {
            return ret;
        }

        /* Link children to list entry */
        if (child_first)
        {
            lyd_insert_child(node, child_first);
        }
    }
    else
    {
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "List \"%s\" value must be a CBOR map or array", snode->name);
        ret = LY_EVALID;
    }

    return ret;
}

static LY_ERR
lydcbor_parse_any(struct lyd_cbor_ctx *lydctx, const struct lysc_node *snode,
                  const cbor_item_t *cbor_value, struct lyd_node **first_p, struct ly_set *parsed)
{
    LY_ERR ret;
    struct lyd_node *node = NULL;

    ret = lyd_create_any(snode, cbor_value, LYD_ANYDATA_CBOR, 0, &node);
    LY_CHECK_RET(ret);

    if (!node)
    {
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Failed to create any node for \"%s\"", snode->name);
        return LY_EVALID;
    }

    /* Insert into tree */
    ret = lyd_insert_sibling(*first_p, node, first_p);
    if (ret)
    {
        lyd_free_tree(node);
        return ret;
    }

    /* Add to parsed set */
    return ly_set_add(parsed, node, 1, NULL);
}


/**
 * @brief Parse CBOR metadata/attributes.
 *
 * @param[in] lydctx CBOR parser context.
 * @param[in] cbor_item CBOR item containing metadata.
 * @param[in,out] node Data node to attach metadata to.
 * @return LY_ERR value.
 */
static LY_ERR
lydcbor_parse_metadata(struct lyd_cbor_ctx *lydctx, const void *cbor_item, struct lyd_node *node)
{
    /* Future implementation for CBOR metadata parsing */
    (void)lydctx;
    (void)cbor_item;
    (void)node;
    return LY_SUCCESS;
}


/**
 * @brief Get module prefix from a qualified name.
 *
 * @param[in] qname Qualified name (prefix:name or just name).
 * @param[in] qname_len Length of the qualified name.
 * @param[out] prefix Extracted prefix (points into qname, not allocated).
 * @param[out] prefix_len Length of the prefix.
 * @param[out] name Local name (points into qname, not allocated).
 * @param[out] name_len Length of the local name.
 * @return LY_SUCCESS on success.
 */
static LY_ERR
lydcbor_parse_qname(const char *qname, size_t qname_len, const char **prefix, size_t *prefix_len,
                    const char **name, size_t *name_len)
{
    const char *colon;

    assert(qname && name && name_len);

    *name = qname;
    *name_len = qname_len;

    if (prefix)
    {
        *prefix = NULL;
    }
    if (prefix_len)
    {
        *prefix_len = 0;
    }

    /* Look for module prefix separator */
    colon = ly_strnchr(qname, ':', qname_len);
    if (colon)
    {
        /* We have a module prefix */
        if (prefix)
        {
            *prefix = qname;
            *prefix_len = colon - qname;
        }

        /* Local name starts after the colon */
        *name = colon + 1;
        *name_len = qname_len - (colon - qname) - 1;

        /* Validate we have both prefix and name */
        if ((colon == qname) || (*name_len == 0))
        {
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Get schema node from CBOR node name, following lydjson_get_snode logic.
 *
 * @param[in] lydctx CBOR parser context.
 * @param[in] name Node name.
 * @param[in] name_len Length of node name.
 * @param[in] parent Data parent node.
 * @param[out] snode Schema node found.
 * @param[out] ext Extension instance if found.
 * @return LY_ERR value.
 */
static LY_ERR
lydcbor_get_snode(struct lyd_cbor_ctx *lydctx, const char *name, size_t name_len,
                  struct lyd_node *parent, const struct lysc_node **snode,
                  const struct lysc_ext_instance **ext)
{
    LY_ERR ret = LY_SUCCESS, r;
    const char *prefix = NULL, *local_name = NULL;
    size_t prefix_len = 0, local_name_len = 0;
    const struct lys_module *mod = NULL;
    const struct lysc_node *sparent = NULL;
    uint32_t getnext_opts;

    assert(lydctx && name && snode);
    *snode = NULL;
    if (ext)
    {
        *ext = NULL;
    }

    /* Parse qualified name */
    LY_CHECK_RET(lydcbor_parse_qname(name, name_len, &prefix, &prefix_len, &local_name, &local_name_len));

    /* Get parent schema node */
    if (parent)
    {
        sparent = parent->schema;
        if (!sparent)
        {
            /* Opaque parent */
            LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Cannot parse \"%.*s\" node with opaque parent.",
                   (int)local_name_len, local_name);
            ret = LY_EVALID;
            goto cleanup;
        }
    }
    else
    {
        sparent = NULL;
    }

    /* Resolve module if prefix is present */
    if (prefix)
    {
        mod = ly_ctx_get_module_implemented2(lydctx->cborctx->ctx, prefix, prefix_len);
        if (!mod)
        {
            if (lydctx->parse_opts & LYD_PARSE_STRICT)
            {
                LOGVAL(lydctx->cborctx->ctx, LYVE_REFERENCE, "Unknown module \"%.*s\".", (int)prefix_len, prefix);
                ret = LY_EVALID;
                goto cleanup;
            }
            if (!(lydctx->parse_opts & LYD_PARSE_OPAQ))
            {
                LOGVAL(lydctx->cborctx->ctx, LYVE_REFERENCE, "Unknown module \"%.*s\".", (int)prefix_len, prefix);
                ret = LY_EVALID;
                goto cleanup;
            }
        }
    }
    else if (!sparent)
    {
        /* Top-level node without prefix - need to find module */
        /* Try to find the node in all implemented modules */
        const struct lys_module *iter_mod;
        uint32_t idx = 0;
        ly_bool found = 0;

        while ((iter_mod = ly_ctx_get_module_iter(lydctx->cborctx->ctx, &idx)))
        {
            if (!iter_mod->implemented)
            {
                continue;
            }

            /* Check if node exists in this module */
            if (lys_find_child(NULL, iter_mod, local_name, local_name_len, 0, 0))
            {
                if (found)
                {
                    /* Ambiguous name */
                    LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Ambiguous node name \"%.*s\", use module prefix.",
                           (int)local_name_len, local_name);
                    ret = LY_EVALID;
                    goto cleanup;
                }
                mod = iter_mod;
                found = 1;
            }
        }

        if (!found && !(lydctx->parse_opts & LYD_PARSE_OPAQ))
        {
            LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Unknown node \"%.*s\".", (int)local_name_len, local_name);
            ret = LY_EVALID;
            goto cleanup;
        }
    }

    /* Set getnext options */
    getnext_opts = lydctx->int_opts & LYD_INTOPT_REPLY ? LYS_GETNEXT_OUTPUT : 0;
    if (parent && (parent->schema->nodetype & (LYS_RPC | LYS_ACTION)))
    {
        if (lydctx->int_opts & LYD_INTOPT_RPC)
        {
            getnext_opts = 0;
        }
        else if (lydctx->int_opts & LYD_INTOPT_REPLY)
        {
            getnext_opts = LYS_GETNEXT_OUTPUT;
        }
    }

    /* Find schema node */
    if (sparent)
    {
        /* Search in parent's children */
        *snode = lys_find_child(sparent, sparent->module, local_name, local_name_len, 0, getnext_opts);

        /* Try to find extension data if regular node not found */
        if (!*snode && ext)
        {
            r = ly_nested_ext_schema(parent, sparent, prefix, prefix_len, LY_VALUE_JSON, NULL,
                                     local_name, local_name_len, snode, ext);
            if (r != LY_ENOT)
            {
                if (r)
                {
                    ret = r;
                    goto cleanup;
                }
            }
        }
    }
    else
    {
        /* Top-level node */
        if (mod)
        {
            /* Search in specific module */
            *snode = lys_find_child(NULL, mod, local_name, local_name_len, 0, getnext_opts);
        }
        /* Extension data for top-level not typically handled */
    }

    /* Handle missing schema node */
    if (!*snode)
    {
        if (lydctx->parse_opts & LYD_PARSE_STRICT)
        {
            if (prefix)
            {
                LOGVAL(lydctx->cborctx->ctx, LYVE_REFERENCE, "Unknown element \"%.*s\" in module \"%.*s\".",
                       (int)local_name_len, local_name, (int)prefix_len, prefix);
            }
            else
            {
                LOGVAL(lydctx->cborctx->ctx, LYVE_REFERENCE, "Unknown element \"%.*s\".",
                       (int)local_name_len, local_name);
            }
            ret = LY_EVALID;
            goto cleanup;
        }
        else if (!(lydctx->parse_opts & LYD_PARSE_OPAQ))
        {
            /* Log error but continue if not in strict mode and opaque allowed */
            if (prefix)
            {
                LOGVAL(lydctx->cborctx->ctx, LYVE_REFERENCE, "Unknown element \"%.*s\" in module \"%.*s\".",
                       (int)local_name_len, local_name, (int)prefix_len, prefix);
            }
            else
            {
                LOGVAL(lydctx->cborctx->ctx, LYVE_REFERENCE, "Unknown element \"%.*s\".",
                       (int)local_name_len, local_name);
            }
            ret = LY_EVALID;
            goto cleanup;
        }
        /* If opaque parsing allowed, *snode remains NULL and caller handles it */
    }

cleanup:
    return ret;
}

static LY_ERR
lydcbor_parse_leaflist_array(struct lyd_cbor_ctx *lydctx, const struct lysc_node *snode,
                             const cbor_item_t *array_item, struct lyd_node **first_p, struct ly_set *parsed)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *node = NULL;
    size_t array_size;
    cbor_item_t **array_handle;

    assert(lydctx && snode && array_item && parsed);

    if (!cbor_isa_array(array_item))
    {
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Expected CBOR array for leaf-list");
        return LY_EVALID;
    }

    if (snode->nodetype != LYS_LEAFLIST)
    {
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Schema node must be leaf-list");
        return LY_EVALID;
    }

    array_size = cbor_array_size(array_item);
    array_handle = cbor_array_handle(array_item);

    if (!array_handle && array_size > 0)
    {
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Invalid CBOR array structure");
        return LY_EVALID;
    }

    for (size_t i = 0; i < array_size; ++i)
    {
        const cbor_item_t *item = array_handle[i];

        if (!item)
        {
            LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Null array element at index %zu", i);
            ret = LY_EVALID;
            goto cleanup;
        }

        LY_CHECK_GOTO(ret = lydcbor_parse_node_value(lydctx, snode, &node, item), cleanup);

        if (!node)
        {
            LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Failed to create node for array element %zu", i);
            ret = LY_EVALID;
            goto cleanup;
        }

        /* Insert the node */
        ret = lyd_insert_sibling(*first_p, node, first_p);
        LY_CHECK_GOTO(ret, cleanup);

        /* Add to parsed set */
        LY_CHECK_GOTO(ret = ly_set_add(parsed, node, 1, NULL), cleanup);
        node = NULL; /* Reset pointer after successful insertion */
    }

cleanup:
    if (ret && node)
    {
        lyd_free_tree(node);
    }
    return ret;
}

static LY_ERR
lydcbor_parse_subtree(struct lyd_cbor_ctx *lydctx, struct lyd_node *parent,
                      struct lyd_node **first_p, struct ly_set *parsed, const cbor_item_t *cbor_obj)
{
    LY_ERR ret = LY_SUCCESS;

    printf("Entering lydcbor_parse_subtree\n");
    printf("CBOR object:\n");
    print_json(cbor_obj);
    printf("\n");

    const struct lysc_node *snode = NULL;
    char *key_str = NULL;
    size_t key_len = 0;

    assert(lydctx && first_p && parsed && cbor_obj);

    /* assuming that the top level structure is always a map
    to be modified to include anything else that it can support */

    if (!cbor_isa_map(cbor_obj))
    {
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Expected CBOR map");
        return LY_EVALID;
    }

    size_t map_size = cbor_map_size(cbor_obj);
    struct cbor_pair *pairs = cbor_map_handle(cbor_obj);

    if (!pairs && map_size > 0)
    {
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Invalid CBOR map structure");
        return LY_EVALID;
    }

    for (size_t i = 0; i < map_size; ++i)
    {
        const cbor_item_t *key_item = pairs[i].key;
        const cbor_item_t *value_item = pairs[i].value;

        if (!key_item || !value_item)
        {
            LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Null key or value at map index %zu", i);
            ret = LY_EVALID;
            goto cleanup;
        }

        /* Get key string */
        LY_CHECK_GOTO(ret = lydcbor_get_key_string(lydctx, key_item, &key_str, &key_len), cleanup);

        /* Find schema node */
        LY_CHECK_GOTO(ret = lydcbor_get_snode(lydctx, key_str, key_len,
                                              parent, &snode, NULL),
                      cleanup);

        /* Handle different node types */
        if (snode->nodetype & (LYS_LEAF | LYS_LEAFLIST))
        {
            if (snode->nodetype == LYS_LEAFLIST && cbor_isa_array(value_item))
            {
                ret = lydcbor_parse_leaflist_array(lydctx, snode, value_item, first_p, parsed);
            }
            else
            {
                ret = lydcbor_parse_terminal(lydctx, snode, value_item, first_p, parsed);
            }
        }
        else if (snode->nodetype == LYS_CONTAINER)
        {
            ret = lydcbor_parse_container(lydctx, snode, value_item, first_p, parsed);
        }
        else if (snode->nodetype == LYS_LIST)
        {
            ret = lydcbor_parse_list(lydctx, snode, value_item, first_p, parsed);
        }
        else if (snode->nodetype & (LYS_ANYDATA | LYS_ANYXML))
        {
            ret = lydcbor_parse_any(lydctx, snode, value_item, first_p, parsed);
        }
        else
        {
            LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Invalid schema node type %d for \"%s\"",
                   snode->nodetype, snode->name);
            ret = LY_EVALID;
        }

        LY_CHECK_GOTO(ret, cleanup);

        free(key_str);
        key_str = NULL;
    }

cleanup:
    free(key_str);
    return ret;
}

/**
 * @brief Create a new CBOR parser context.
 *
 * @param[in] ctx libyang context.
 * @param[in] ext Extension instance providing context for the top level element, NULL if none.
 * @param[in] parse_opts Parse options, see @ref dataparseroptions.
 * @param[in] val_opts Validation options, see @ref datavalidationoptions.
 * @param[in] format CBOR format variant (named or SID).
 * @param[out] lydctx_p Pointer to the created CBOR parser context.
 * @return LY_ERR value.
 */
static LY_ERR
lydcbor_ctx_init(const struct ly_ctx *ctx, struct ly_in *in, uint32_t parse_opts,
                 uint32_t val_opts, struct lyd_cbor_ctx **lydctx_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_cbor_ctx *lydctx = NULL;

    assert(lydctx_p);

    /* Initialize context with calloc to ensure all fields are zero */
    lydctx = calloc(1, sizeof *lydctx);
    LY_CHECK_ERR_RET(!lydctx, LOGMEM(ctx), LY_EMEM);
    lydctx->parse_opts = parse_opts;
    lydctx->val_opts = val_opts;
    lydctx->free = lyd_cbor_ctx_free;

    /* Create low-level CBOR context */
    LY_CHECK_GOTO(ret = lycbor_ctx_new(ctx, in, &lydctx->cborctx), cleanup);

    *lydctx_p = lydctx;
    return ret;

cleanup:
    if (lydctx)
    {
        lyd_cbor_ctx_free((struct lyd_ctx *)lydctx);
    }
    return ret;
}

LY_ERR
lyd_parse_cbor(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
               struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, uint32_t int_opts,
               struct ly_set *parsed, ly_bool *subtree_sibling, struct lyd_ctx **lydctx_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_cbor_ctx *lydctx = NULL;
    cbor_item_t *cbor_data = NULL;
    struct cbor_load_result result = {0};

    /* Initialize context */
    LY_CHECK_GOTO(ret = lydcbor_ctx_init(ctx, in, parse_opts, val_opts, &lydctx), cleanup);

    lydctx->int_opts = int_opts;
    lydctx->ext = ext;

    /*
     * Loads CBOR data from the current input buffer.
     *
     * Parameters:
     *   in->current - Pointer to the current position in the input buffer.
     *   in->length  - Length of the data to be loaded.
     *   &result     - Pointer to a variable where the result status will be stored.
     *
     * Returns:
     *   cbor_data - Pointer to the loaded CBOR data structure, or NULL on failure.
     */
    /* need to convert in->current from  const char* to cbor_data type */
    cbor_data = cbor_load(in->current, in->length, &result);
    lydctx->cborctx->cbor_data = cbor_data;

    if (!cbor_data)
    {
        LOGVAL(ctx, LYVE_SYNTAX, "Failed to parse CBOR data: no data returned from cbor_load().");
        ret = LY_EVALID;
        goto cleanup;
    }
    if (result.error.code != CBOR_ERR_NONE)
    {
        LOGVAL(ctx, LYVE_SYNTAX, "Failed to parse CBOR data: parsing error (code %d).", result.error.code);
        ret = LY_EVALID;
        goto cleanup;
    }

    /* Probably need to check if the obtained data is a operational node and
    then write functions to parse them accordingly. If not then continue below */

    /* Parse the CBOR structure */
    ret = lydcbor_parse_subtree(lydctx, parent, first_p, parsed, cbor_data);

cleanup:
    if (cbor_data)
    {
        cbor_decref(&cbor_data);
    }

    if (ret)
    {
        if (lydctx)
        {
            lyd_cbor_ctx_free((struct lyd_ctx *)lydctx);
            lydctx = NULL;
        }
    }

    *lydctx_p = (struct lyd_ctx *)lydctx;
    return ret;
}

#endif /* ENABLE_CBOR_SUPPORT */