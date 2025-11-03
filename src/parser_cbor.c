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

void print_json_array(const cbor_item_t *item)
{
    printf("[");
    size_t size = cbor_array_size(item);
    cbor_item_t **handle = cbor_array_handle(item);

    for (size_t i = 0; i < size; ++i)
    {
        print_json(handle[i]);
        if (i < size - 1)
            printf(", ");
    }

    printf("]");
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

void print_json_number(const cbor_item_t *item)
{
    if (cbor_isa_uint(item))
    {
        printf("%lu", cbor_get_uint64(item));
    }
    else if (cbor_isa_negint(item))
    {
        printf("-%lu", cbor_get_uint64(item) + 1);
    }
    else if (cbor_isa_float_ctrl(item))
    {
        if (cbor_float_get_width(item) == CBOR_FLOAT_64)
        {
            printf("%f", cbor_float_get_float8(item));
        }
        else if (cbor_float_get_width(item) == CBOR_FLOAT_32)
        {
            printf("%f", cbor_float_get_float4(item));
        }
        else if (cbor_float_get_width(item) == CBOR_FLOAT_16)
        {
            printf("%f", cbor_float_get_float2(item));
        }
    }
}

void print_json_bool(const cbor_item_t *item)
{
    printf(cbor_is_bool(item) && cbor_ctrl_value(item) ? "true" : "false");
}

void print_json_null(const cbor_item_t *item)
{
    printf("null");
}

void print_json(cbor_item_t *item)
{
    if (!item)
    {
        printf("null");
        return;
    }

    if (cbor_isa_map(item))
    {
        print_json_map(item);
    }
    else if (cbor_isa_array(item))
    {
        print_json_array(item);
    }
    else if (cbor_isa_string(item))
    {
        print_json_string(item);
    }
    else if (cbor_isa_uint(item) || cbor_isa_negint(item))
    {
        print_json_number(item);
    }
    else if (cbor_isa_float_ctrl(item))
    {
        // Check if it's a control value (null, undefined, true, false)
        if (cbor_float_get_width(item) == CBOR_FLOAT_0)
        {
            uint8_t ctrl = cbor_ctrl_value(item);
            if (ctrl == 20)
            {
                printf("false");
            }
            else if (ctrl == 21)
            {
                printf("true");
            }
            else if (ctrl == 22)
            {
                printf("null");
            }
            else if (ctrl == 23)
            {
                printf("undefined");
            }
            else
            {
                printf("null"); // unknown control value
            }
        }
        else
        {
            print_json_number(item);
        }
    }
    else if (cbor_is_bool(item))
    {
        print_json_bool(item);
    }
    else
    {
        printf("null"); // fallback for truly unsupported types
    }
}

/**
 * @brief Free the CBOR data parser context.
 *
 * CBOR implementation of lyd_ctx_free_clb().
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
 * @brief Parse CBOR member-name as [\@][prefix:][name]
 *
 * \@ - metadata flag, maps to 1 in @p is_meta_p
 * prefix - name of the module of the data node
 * name - name of the data node
 *
 * All the output parameters are mandatory. Function only parses the member-name.
 *
 * @param[in] value String to parse
 * @param[in] value_len Length of the @p value.
 * @param[out] name_p Pointer to the beginning of the parsed name.
 * @param[out] name_len_p Pointer to the length of the parsed name.
 * @param[out] prefix_p Pointer to the beginning of the parsed prefix. If the member-name does not contain prefix, result is NULL.
 * @param[out] prefix_len_p Pointer to the length of the parsed prefix. If the member-name does not contain prefix, result is 0.
 * @param[out] is_meta_p Pointer to the metadata flag, set to 1 if the member-name contains \@, 0 otherwise.
 */
static void
lydcbor_parse_name(const char *value, size_t value_len, const char **name_p, size_t *name_len_p, const char **prefix_p,
                   size_t *prefix_len_p, ly_bool *is_meta_p)
{
    const char *name, *prefix = NULL;
    size_t name_len, prefix_len = 0;
    ly_bool is_meta = 0;

    name = memchr(value, ':', value_len);
    if (name != NULL)
    {
        prefix = value;
        if (*prefix == '@')
        {
            is_meta = 1;
            prefix++;
        }
        prefix_len = name - prefix;
        name++;
        name_len = value_len - (prefix_len + 1) - is_meta;
    }
    else
    {
        name = value;
        if (name[0] == '@')
        {
            is_meta = 1;
            name++;
        }
        name_len = value_len - is_meta;
    }

    *name_p = name;
    *name_len_p = name_len;
    *prefix_p = prefix;
    *prefix_len_p = prefix_len;
    *is_meta_p = is_meta;
}

/**
 * @brief Get correct prefix (module_name) inside the @p node.
 *
 * @param[in] node Data node to get inherited prefix.
 * @param[in] local_prefix Local prefix to replace the inherited prefix.
 * @param[in] local_prefix_len Length of the @p local_prefix string. In case of 0, the inherited prefix is taken.
 * @param[out] prefix_p Pointer to the resulting prefix string.
 * @param[out] prefix_len_p Pointer to the length of the resulting @p prefix_p string.
 * @return LY_ERR value.
 */
static LY_ERR
lydcbor_get_node_prefix(struct lyd_node *node, const char *local_prefix, size_t local_prefix_len, const char **prefix_p,
                        size_t *prefix_len_p)
{
    struct lyd_node_opaq *onode;
    const char *module_name = NULL;

    assert(prefix_p && prefix_len_p);

    if (local_prefix_len)
    {
        *prefix_p = local_prefix;
        *prefix_len_p = local_prefix_len;
        return LY_SUCCESS;
    }

    while (node)
    {
        if (node->schema)
        {
            module_name = node->schema->module->name;
            break;
        }
        onode = (struct lyd_node_opaq *)node;
        if (onode->name.module_name)
        {
            module_name = onode->name.module_name;
            break;
        }
        else if (onode->name.prefix)
        {
            module_name = onode->name.prefix;
            break;
        }
        node = lyd_parent(node);
    }

    *prefix_p = module_name;
    *prefix_len_p = ly_strlen(module_name);
    return LY_SUCCESS;
}

/**
 * @brief Skip the current CBOR item based on its type.
 *
 * @param[in] cborctx CBOR context.
 * @param[in] item CBOR item to skip.
 * @return LY_ERR value.
 */
static LY_ERR
lydcbor_data_skip(struct lycbor_ctx *cborctx)
{
    (void)cborctx;
    /* In CBOR, items are already parsed, so skipping is implicit */
    return LY_SUCCESS;
}

/**
 * @brief Get schema node corresponding to the input parameters.
 *
 * @param[in] lydctx CBOR data parser context.
 * @param[in] is_attr Flag if the reference to the node is an attribute.
 * @param[in] prefix Requested node's prefix (module name).
 * @param[in] prefix_len Length of the @p prefix.
 * @param[in] name Requested node's name.
 * @param[in] name_len Length of the @p name.
 * @param[in] parent Parent of the node being processed.
 * @param[out] snode Found schema node corresponding to the input parameters.
 * @param[out] ext Extension instance that provided @p snode, if any.
 * @return LY_SUCCESS on success.
 * @return LY_ENOT if the whole object was parsed (skipped or as an extension).
 * @return LY_ERR on error.
 */
static LY_ERR
lydcbor_get_snode(struct lyd_cbor_ctx *lydctx, ly_bool is_attr, const char *prefix, size_t prefix_len, const char *name,
                  size_t name_len, struct lyd_node *parent, const struct lysc_node **snode, struct lysc_ext_instance **ext)
{
    LY_ERR ret = LY_SUCCESS, r;
    struct lys_module *mod = NULL;
    uint32_t getnext_opts = lydctx->int_opts & LYD_INTOPT_REPLY ? LYS_GETNEXT_OUTPUT : 0;

    *snode = NULL;
    *ext = NULL;

    /* get the element module, prefer parent context because of extensions */
    if (prefix_len)
    {
        mod = ly_ctx_get_module_implemented2(parent ? LYD_CTX(parent) : lydctx->cborctx->ctx, prefix, prefix_len);
    }
    else if (parent)
    {
        if (parent->schema)
        {
            mod = parent->schema->module;
        }
    }
    else if (!(lydctx->int_opts & LYD_INTOPT_ANY))
    {
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Top-level CBOR object member \"%.*s\" must be namespace-qualified.",
               (int)(is_attr ? name_len + 1 : name_len), is_attr ? name - 1 : name);
        ret = LY_EVALID;
        goto cleanup;
    }
    if (!mod)
    {
        /* check for extension data */
        r = ly_nested_ext_schema(parent, NULL, prefix, prefix_len, LY_VALUE_CBOR, NULL, name, name_len, snode, ext);
        if (r != LY_ENOT)
        {
            ret = r;
            goto cleanup;
        }

        /* unknown module */
        if (lydctx->parse_opts & LYD_PARSE_STRICT)
        {
            LOGVAL(lydctx->cborctx->ctx, LYVE_REFERENCE, "No module named \"%.*s\" in the context.", (int)prefix_len, prefix);
            ret = LY_EVALID;
            goto cleanup;
        }
    }

    /* get the schema node */
    if (mod && (!parent || parent->schema))
    {
        if (!parent && lydctx->ext)
        {
            *snode = lysc_ext_find_node(lydctx->ext, mod, name, name_len, 0, getnext_opts);
        }
        else
        {
            *snode = lys_find_child(lyd_parser_node_schema(parent), mod, name, name_len, 0, getnext_opts);
        }
        if (!*snode)
        {
            /* check for extension data */
            r = ly_nested_ext_schema(parent, NULL, prefix, prefix_len, LY_VALUE_CBOR, NULL, name, name_len, snode, ext);
            if (r != LY_ENOT)
            {
                ret = r;
                goto cleanup;
            }

            /* unknown data node */
            printf("checkpoint1-web\n");
            if (lydctx->parse_opts & LYD_PARSE_STRICT)
            {
                if (parent)
                {
                    LOGVAL(lydctx->cborctx->ctx, LYVE_REFERENCE, "Node \"%.*s\" not found as a child of \"%s\" node.",
                           (int)name_len, name, LYD_NAME(parent));
                }
                else if (lydctx->ext)
                {
                    if (lydctx->ext->argument)
                    {
                        LOGVAL(lydctx->cborctx->ctx, LYVE_REFERENCE,
                               "Node \"%.*s\" not found in the \"%s\" %s extension instance.",
                               (int)name_len, name, lydctx->ext->argument, lydctx->ext->def->name);
                    }
                    else
                    {
                        LOGVAL(lydctx->cborctx->ctx, LYVE_REFERENCE, "Node \"%.*s\" not found in the %s extension instance.",
                               (int)name_len, name, lydctx->ext->def->name);
                    }
                }
                else
                {
                    LOGVAL(lydctx->cborctx->ctx, LYVE_REFERENCE, "Node \"%.*s\" not found in the \"%s\" module.",
                           (int)name_len, name, mod->name);
                }
                ret = LY_EVALID;
                goto cleanup;
            }
        }
        else
        {
            /* check that schema node is valid and can be used */
            ret = lyd_parser_check_schema((struct lyd_ctx *)lydctx, *snode);
        }
    }

cleanup:
    return ret;
}

/**
 * @brief Check if CBOR item is null/undefined.
 *
 * @param[in] item CBOR item to check.
 * @return 1 if null/undefined, 0 otherwise.
 */
static ly_bool
lydcbor_is_null(const cbor_item_t *item)
{
    if (!item)
    {
        return 1;
    }

    if (cbor_isa_float_ctrl(item))
    {
        if (cbor_float_get_width(item) == CBOR_FLOAT_0)
        {
            uint8_t ctrl = cbor_ctrl_value(item);
            if (ctrl == 22 || ctrl == 23)
            { /* null or undefined */
                return 1;
            }
        }
    }

    return 0;
}

/**
 * @brief Get the hint for the data type parsers according to the current CBOR type.
 *
 * @param[in] lydctx CBOR data parser context.
 * @param[in] item CBOR item.
 * @param[out] type_hint_p Pointer to the variable to store the result.
 * @return LY_SUCCESS in case of success.
 * @return LY_EINVAL in case of invalid CBOR type.
 */
static LY_ERR
lydcbor_value_type_hint(struct lyd_cbor_ctx *lydctx, const cbor_item_t *item, uint32_t *type_hint_p)
{
    enum cbor_type type;

    *type_hint_p = 0;

    if (!item)
    {
        return LY_EINVAL;
    }

    type = cbor_typeof(item);

    if (type == CBOR_TYPE_ARRAY)
    {
        /* check for [null] */
        if (cbor_array_size(item) == 1)
        {
            cbor_item_t **handle = cbor_array_handle(item);
            if (handle && lydcbor_is_null(handle[0]))
            {
                *type_hint_p = LYD_VALHINT_EMPTY;
                return LY_SUCCESS;
            }
        }
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Expected CBOR value or [null], but array found.");
        return LY_EINVAL;
    }
    else if (type == CBOR_TYPE_STRING)
    {
        *type_hint_p = LYD_VALHINT_STRING | LYD_VALHINT_NUM64;
    }
    else if (type == CBOR_TYPE_UINT || type == CBOR_TYPE_NEGINT)
    {
        *type_hint_p = LYD_VALHINT_DECNUM;
    }
    else if (type == CBOR_TYPE_FLOAT_CTRL)
    {
        if (cbor_float_ctrl_is_ctrl(item))
        {
            uint8_t ctrl = cbor_ctrl_value(item);
            if (ctrl == CBOR_CTRL_TRUE || ctrl == CBOR_CTRL_FALSE)
            {
                *type_hint_p = LYD_VALHINT_BOOLEAN;
            }
            else if (ctrl == CBOR_CTRL_NULL)
            {
                *type_hint_p = 0;
            }
            else
            {
                LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Unexpected CBOR control value.");
                return LY_EINVAL;
            }
        }
        else
        {
            *type_hint_p = LYD_VALHINT_DECNUM;
        }
    }
    else
    {
        LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Unexpected CBOR data type.");
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

/**
 * @brief Convert a CBOR item to a string representation.
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

    switch (cbor_typeof(item)) {
    case CBOR_TYPE_UINT: {
        uint64_t val = cbor_get_int(item);
        int len = snprintf(NULL, 0, "%" PRIu64, val);
        if (len < 0) {
            return LY_ESYS;
        }
        *str_val = malloc(len + 1);
        LY_CHECK_ERR_RET(!*str_val, LOGMEM(NULL), LY_EMEM);
        sprintf(*str_val, "%" PRIu64, val);
        *str_len = len;
        break;
    }
    case CBOR_TYPE_NEGINT: {
        int64_t val = -1 - (int64_t)cbor_get_int(item);
        int len = snprintf(NULL, 0, "%" PRId64, val);
        if (len < 0) {
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
        if (cbor_float_ctrl_is_ctrl(item)) {
            switch (cbor_ctrl_value(item)) {
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
        } else {
            double val = cbor_float_get_float(item);
            int len = snprintf(NULL, 0, "%g", val);
            if (len < 0) {
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
 * @brief Check in advance if the input data are parsable according to the provided @p snode.
 *
 * Note that the checks are done only in case the LYD_PARSE_OPAQ is allowed. Otherwise the same checking
 * is naturally done when the data are really parsed.
 *
 * @param[in] lydctx CBOR data parser context.
 * @param[in] snode Schema node corresponding to the member currently being processed.
 * @param[in] cbor_value CBOR value to check.
 * @param[out] type_hint_p Pointer to store detected value type hint.
 * @return LY_SUCCESS if data are parsable.
 * @return LY_ENOT if input data are not sufficient.
 * @return LY_EINVAL in case of invalid encoding.
 */
static LY_ERR
lydcbor_data_check_opaq(struct lyd_cbor_ctx *lydctx, const struct lysc_node *snode, const cbor_item_t *cbor_value,
                        uint32_t *type_hint_p)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t *prev_lo, temp_lo = 0;
    char *str_val = NULL;
    size_t str_len = 0;

    assert(snode);

    if (!(snode->nodetype & (LYD_NODE_TERM | LYS_LIST)))
    {
        return LY_SUCCESS;
    }

    if (lydctx->parse_opts & LYD_PARSE_OPAQ)
    {
        switch (snode->nodetype)
        {
        case LYS_LEAFLIST:
        case LYS_LEAF:
            if ((ret = lydcbor_value_type_hint(lydctx, cbor_value, type_hint_p)))
            {
                break;
            }

            prev_lo = ly_temp_log_options(&temp_lo);
            ret = lydcbor_item_to_string(cbor_value, &str_val, &str_len);
            if (ret == LY_SUCCESS)
            {
                if (ly_value_validate(NULL, snode, str_val, str_len, LY_VALUE_CBOR, NULL, *type_hint_p))
                {
                    ret = LY_ENOT;
                }
            }
            ly_temp_log_options(prev_lo);
            free(str_val);
            break;
        case LYS_LIST:
            /* Lists may not have all keys - handled elsewhere */
            break;
        }
    }
    else if (snode->nodetype & LYD_NODE_TERM)
    {
        ret = lydcbor_value_type_hint(lydctx, cbor_value, type_hint_p);
    }

    return ret;
}

/**
 * @brief Join the forward-referencing metadata with their target data nodes.
 *
 * @param[in] lydctx CBOR data parser context.
 * @param[in,out] first_p Pointer to the first sibling node.
 * @return LY_SUCCESS on success.
 * @return LY_EVALID if there are unresolved metadata.
 */
static LY_ERR
lydcbor_metadata_finish(struct lyd_cbor_ctx *lydctx, struct lyd_node **first_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *node, *attr, *next, *meta_iter;
    struct lysc_ext_instance *ext;
    uint64_t instance = 0;
    const char *prev = NULL;
    uint32_t log_location_items = 0;

    /* finish linking metadata */
    LY_LIST_FOR_SAFE(*first_p, next, attr)
    {
        struct lyd_node_opaq *meta_container = (struct lyd_node_opaq *)attr;
        uint64_t match = 0;
        ly_bool is_attr;
        const char *name, *prefix;
        size_t name_len, prefix_len;
        const struct lysc_node *snode;

        if (attr->schema || (meta_container->name.name[0] != '@'))
        {
            continue;
        }

        LOG_LOCSET(NULL, attr);
        log_location_items++;

        if (prev != meta_container->name.name)
        {
            lydict_remove(lydctx->cborctx->ctx, prev);
            LY_CHECK_GOTO(ret = lydict_insert(lydctx->cborctx->ctx, meta_container->name.name, 0, &prev), cleanup);
            instance = 1;
        }
        else
        {
            instance++;
        }

        /* find the corresponding data node */
        LY_LIST_FOR(*first_p, node)
        {
            if (!node->schema)
            {
                /* opaq node */
                if (strcmp(&meta_container->name.name[1], ((struct lyd_node_opaq *)node)->name.name))
                {
                    continue;
                }

                if (((struct lyd_node_opaq *)node)->hints & LYD_NODEHINT_LIST)
                {
                    LOGVAL(lydctx->cborctx->ctx, LYVE_SYNTAX, "Metadata container references a sibling list node %s.",
                           ((struct lyd_node_opaq *)node)->name.name);
                    ret = LY_EVALID;
                    goto cleanup;
                }

                match++;
                if (match != instance)
                {
                    continue;
                }

                LY_LIST_FOR(meta_container->child, meta_iter)
                {
                    struct lyd_node_opaq *meta = (struct lyd_node_opaq *)meta_iter;

                    ret = lyd_create_attr(node, NULL, lydctx->cborctx->ctx, meta->name.name, strlen(meta->name.name),
                                          meta->name.prefix, ly_strlen(meta->name.prefix), meta->name.module_name,
                                          ly_strlen(meta->name.module_name), meta->value, ly_strlen(meta->value), NULL, LY_VALUE_CBOR,
                                          NULL, meta->hints);
                    LY_CHECK_GOTO(ret, cleanup);
                }
                break;
            }
            else
            {
                lydcbor_parse_name(meta_container->name.name, strlen(meta_container->name.name), &name, &name_len,
                                   &prefix, &prefix_len, &is_attr);
                assert(is_attr);
                lydcbor_get_snode(lydctx, is_attr, prefix, prefix_len, name, name_len, lyd_parent(*first_p), &snode, &ext);

                if (snode != node->schema)
                {
                    continue;
                }

                match++;
                if (match != instance)
                {
                    continue;
                }

                LY_LIST_FOR(meta_container->child, meta_iter)
                {
                    struct lyd_node_opaq *meta = (struct lyd_node_opaq *)meta_iter;
                    struct lys_module *mod = NULL;

                    mod = ly_ctx_get_module_implemented(lydctx->cborctx->ctx, meta->name.prefix);
                    if (mod)
                    {
                        ret = lyd_parser_create_meta((struct lyd_ctx *)lydctx, node, NULL, mod,
                                                     meta->name.name, strlen(meta->name.name), meta->value, ly_strlen(meta->value),
                                                     NULL, LY_VALUE_CBOR, NULL, meta->hints, node->schema);
                        LY_CHECK_GOTO(ret, cleanup);
                    }
                    else if (lydctx->parse_opts & LYD_PARSE_STRICT)
                    {
                        if (meta->name.prefix)
                        {
                            LOGVAL(lydctx->cborctx->ctx, LYVE_REFERENCE,
                                   "Unknown (or not implemented) YANG module \"%s\" of metadata \"%s%s%s\".",
                                   meta->name.prefix, meta->name.prefix, ly_strlen(meta->name.prefix) ? ":" : "",
                                   meta->name.name);
                        }
                        else
                        {
                            LOGVAL(lydctx->cborctx->ctx, LYVE_REFERENCE, "Missing YANG module of metadata \"%s\".",
                                   meta->name.name);
                        }
                        ret = LY_EVALID;
                        goto cleanup;
                    }
                }

                ret = lyd_parser_set_data_flags(node, &node->meta, (struct lyd_ctx *)lydctx, ext);
                LY_CHECK_GOTO(ret, cleanup);
                break;
            }
        }

        if (match != instance)
        {
            if (instance > 1)
            {
                LOGVAL(lydctx->cborctx->ctx, LYVE_REFERENCE,
                       "Missing CBOR data instance #%" PRIu64 " to be coupled with %s metadata.",
                       instance, meta_container->name.name);
            }
            else
            {
                LOGVAL(lydctx->cborctx->ctx, LYVE_REFERENCE, "Missing CBOR data instance to be coupled with %s metadata.",
                       meta_container->name.name);
            }
            ret = LY_EVALID;
        }
        else
        {
            if (attr == (*first_p))
            {
                *first_p = attr->next;
            }
            lyd_free_tree(attr);
        }

        LOG_LOCBACK(0, log_location_items);
        log_location_items = 0;
    }

cleanup:
    lydict_remove(lydctx->cborctx->ctx, prev);
    LOG_LOCBACK(0, log_location_items);
    return ret;
}

/**
 * @brief Parse a metadata member/attribute from CBOR.
 *
 * @param[in] lydctx CBOR data parser context.
 * @param[in] snode Schema node of the metadata parent.
 * @param[in] node Parent node.
 * @param[in] cbor_meta CBOR metadata item.
 * @return LY_ERR value.
 */
static LY_ERR
lydcbor_meta_attr(struct lyd_cbor_ctx *lydctx, const struct lysc_node *snode, struct lyd_node *node,
                  const cbor_item_t *cbor_meta)
{
    LY_ERR rc = LY_SUCCESS, r;
    enum cbor_type type;
    const char *expected;
    ly_bool in_parent = 0;
    const char *name, *prefix = NULL;
    size_t name_len, prefix_len = 0;
    struct lys_module *mod;
    const struct ly_ctx *ctx = lydctx->cborctx->ctx;
    ly_bool is_attr = 0;
    struct lyd_node *prev = node;
    uint32_t instance = 0, val_hints;
    uint16_t nodetype;
    struct cbor_pair *pairs;
    size_t map_size;

    assert(snode || node);

    nodetype = snode ? snode->nodetype : LYS_CONTAINER;
    if (snode)
    {
        LOG_LOCSET(snode, NULL);
    }

    type = cbor_typeof(cbor_meta);

    /* check attribute encoding */
    switch (nodetype)
    {
    case LYS_LEAFLIST:
        expected = "@name/array of objects/nulls";
        LY_CHECK_GOTO(type != CBOR_TYPE_ARRAY, representation_error);

    next_entry:
        instance++;
        if (!node || (node->schema != prev->schema))
        {
            LOGVAL(ctx, LYVE_REFERENCE, "Missing CBOR data instance #%" PRIu32 " of %s:%s to be coupled with metadata.", instance, prev->schema->module->name, prev->schema->name);
            rc = LY_EVALID;
            goto cleanup;
        }

        /* Process array item */
        if (cbor_array_size(cbor_meta) > instance - 1)
        {
            cbor_item_t **handle = cbor_array_handle(cbor_meta);
            const cbor_item_t *item = handle[instance - 1];

            if (lydcbor_is_null(item))
            {
                prev = node;
                node = node->next;
                if (instance < cbor_array_size(cbor_meta))
                {
                    goto next_entry;
                }
                goto cleanup;
            }
        }
        else
        {
            goto cleanup;
        }
        break;
    case LYS_LEAF:
    case LYS_ANYXML:
        expected = "@name/object";
        LY_CHECK_GOTO(type != CBOR_TYPE_MAP, representation_error);
        break;
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_ANYDATA:
    case LYS_NOTIF:
    case LYS_ACTION:
    case LYS_RPC:
        in_parent = 1;
        expected = "@/object";
        LY_CHECK_GOTO(type != CBOR_TYPE_MAP, representation_error);
        break;
    default:
        LOGINT(ctx);
        rc = LY_EINT;
        goto cleanup;
    }

    /* process all members inside metadata object */
    assert(type == CBOR_TYPE_MAP);
    map_size = cbor_map_size(cbor_meta);
    pairs = cbor_map_handle(cbor_meta);

    for (size_t i = 0; i < map_size; ++i)
    {
        const cbor_item_t *key_item = pairs[i].key;
        const cbor_item_t *value_item = pairs[i].value;
        char *key_str = NULL;
        size_t key_len = 0;

        if (!cbor_isa_string(key_item))
        {
            LOGVAL(ctx, LYVE_SYNTAX, "Metadata key must be a string.");
            rc = LY_EVALID;
            goto cleanup;
        }

        LY_CHECK_GOTO(rc = lydcbor_item_to_string(key_item, &key_str, &key_len), cleanup);

        lydcbor_parse_name(key_str, key_len, &name, &name_len, &prefix, &prefix_len, &is_attr);

        if (!name_len)
        {
            LOGVAL(ctx, LYVE_SYNTAX, "Metadata in CBOR found with an empty name.");
            free(key_str);
            rc = LY_EVALID;
            goto cleanup;
        }
        else if (!prefix_len)
        {
            LOGVAL(ctx, LYVE_SYNTAX, "Metadata in CBOR must be namespace-qualified, missing prefix for \"%.*s\".",
                   (int)key_len, key_str);
            free(key_str);
            rc = LY_EVALID;
            goto cleanup;
        }
        else if (is_attr)
        {
            LOGVAL(ctx, LYVE_SYNTAX, "Invalid format of Metadata identifier in CBOR, unexpected '@' in \"%.*s\"",
                   (int)key_len, key_str);
            free(key_str);
            rc = LY_EVALID;
            goto cleanup;
        }

        /* get the element module */
        mod = ly_ctx_get_module_implemented2(ctx, prefix, prefix_len);
        if (!mod)
        {
            if (lydctx->parse_opts & LYD_PARSE_STRICT)
            {
                LOGVAL(ctx, LYVE_REFERENCE, "Prefix \"%.*s\" of the metadata \"%.*s\" does not match any module in the context.",
                       (int)prefix_len, prefix, (int)name_len, name);
                free(key_str);
                rc = LY_EVALID;
                goto cleanup;
            }
            if (node->schema)
            {
                free(key_str);
                continue;
            }
            assert(lydctx->parse_opts & LYD_PARSE_OPAQ);
        }

        /* get value hints */
        LY_CHECK_ERR_GOTO(rc = lydcbor_value_type_hint(lydctx, value_item, &val_hints), free(key_str), cleanup);

        if (node->schema)
        {
            char *str_val = NULL;
            size_t str_len = 0;

            LY_CHECK_ERR_GOTO(rc = lydcbor_item_to_string(value_item, &str_val, &str_len), free(key_str), cleanup);

            /* create metadata */
            rc = lyd_parser_create_meta((struct lyd_ctx *)lydctx, node, NULL, mod, name, name_len, str_val,
                                        str_len, NULL, LY_VALUE_CBOR, NULL, val_hints, node->schema);
            free(str_val);
            LY_CHECK_ERR_GOTO(rc, free(key_str), cleanup);

            /* add/correct flags */
            rc = lyd_parser_set_data_flags(node, &node->meta, (struct lyd_ctx *)lydctx, NULL);
            LY_CHECK_ERR_GOTO(rc, free(key_str), cleanup);
        }
        else
        {
            /* create attribute */
            const char *module_name;
            size_t module_name_len;
            char *str_val = NULL;
            size_t str_len = 0;

            lydcbor_get_node_prefix(node, prefix, prefix_len, &module_name, &module_name_len);

            LY_CHECK_ERR_GOTO(rc = lydcbor_item_to_string(value_item, &str_val, &str_len), free(key_str), cleanup);

            rc = lyd_create_attr(node, NULL, ctx, name, name_len, prefix, prefix_len, module_name,
                                 module_name_len, str_val, str_len, NULL, LY_VALUE_CBOR, NULL, val_hints);
            free(str_val);
            LY_CHECK_ERR_GOTO(rc, free(key_str), cleanup);
        }

        free(key_str);
    }

    if (nodetype == LYS_LEAFLIST && instance < cbor_array_size(cbor_meta))
    {
        prev = node;
        node = node->next;
        goto next_entry;
    }

    goto cleanup;

representation_error:
    LOGVAL(ctx, LYVE_SYNTAX,
           "The attribute(s) of %s \"%s\" is expected to be represented as CBOR %s, but input data contains different type.",
           lys_nodetype2str(nodetype), node ? LYD_NAME(node) : LYD_NAME(prev), expected);
    rc = LY_EVALID;

cleanup:
    if ((rc == LY_EVALID) && (lydctx->val_opts & LYD_VALIDATE_MULTI_ERROR))
    {
        if ((r = lydcbor_data_skip(lydctx->cborctx)))
        {
            rc = r;
        }
    }
    LOG_LOCBACK(snode ? 1 : 0, 0);
    return rc;
}

/**
 * @brief Maintain children - insert node and update first pointer.
 *
 * @param[in] parent Parent node to insert to.
 * @param[in,out] first_p Pointer to the first sibling.
 * @param[in,out] node_p Pointer to the node to insert.
 * @param[in] last If set, insert at the end.
 * @param[in] ext Extension instance.
 */
static void
lydcbor_maintain_children(struct lyd_node *parent, struct lyd_node **first_p, struct lyd_node **node_p, ly_bool last,
                          struct lysc_ext_instance *ext)
{
    if (!*node_p)
    {
        return;
    }

    if (ext)
    {
        lyplg_ext_insert(parent, *node_p);
    }
    else
    {
        lyd_insert_node(parent, first_p, *node_p, last);
    }
    if (first_p)
    {
        if (parent)
        {
            *first_p = lyd_child(parent);
        }
        else
        {
            while ((*first_p)->prev->next)
            {
                *first_p = (*first_p)->prev;
            }
        }
    }
    *node_p = NULL;
}

/**
 * @brief Create an opaq node from CBOR.
 *
 * @param[in] lydctx CBOR data parser context.
 * @param[in] name Name of the opaq node.
 * @param[in] name_len Length of @p name.
 * @param[in] prefix Prefix of the opaq node.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] parent Data parent.
 * @param[in] cbor_value CBOR value item.
 * @param[out] node_p Pointer to the created opaq node.
 * @return LY_ERR value.
 */
static LY_ERR
lydcbor_create_opaq(struct lyd_cbor_ctx *lydctx, const char *name, size_t name_len, const char *prefix, size_t prefix_len,
                    struct lyd_node *parent, const cbor_item_t *cbor_value, struct lyd_node **node_p)
{
    LY_ERR ret = LY_SUCCESS;
    const char *value = NULL, *module_name;
    size_t value_len = 0, module_name_len = 0;
    ly_bool dynamic = 0;
    uint32_t type_hint = 0;
    char *str_val = NULL;

    if (cbor_typeof(cbor_value) != CBOR_TYPE_MAP)
    {
        /* prepare for creating opaq node with a value */
        LY_CHECK_RET(lydcbor_item_to_string(cbor_value, &str_val, &value_len));
        value = str_val;
        dynamic = 1;

        LY_CHECK_GOTO(ret = lydcbor_value_type_hint(lydctx, cbor_value, &type_hint), cleanup);
    }

    /* get the module name */
    lydcbor_get_node_prefix(parent, prefix, prefix_len, &module_name, &module_name_len);
    if (!module_name && !parent && lydctx->any_schema)
    {
        module_name = lydctx->any_schema->module->name;
        module_name_len = strlen(module_name);
    }

    /* create node */
    ret = lyd_create_opaq(lydctx->cborctx->ctx, name, name_len, prefix, prefix_len, module_name, module_name_len, value,
                          value_len, &dynamic, LY_VALUE_CBOR, NULL, type_hint, node_p);

cleanup:
    if (dynamic)
    {
        free((char *)value);
    }
    return ret;
}

static LY_ERR lydcbor_subtree_r(struct lyd_cbor_ctx *lydctx, struct lyd_node *parent, 
        struct lyd_node **first_p, struct ly_set *parsed, const cbor_item_t *cbor_obj);

/**
 * @brief Parse opaq node from CBOR.
 *
 * @param[in] lydctx CBOR data parser context.
 * @param[in] name Name of the opaq node.
 * @param[in] name_len Length of @p name.
 * @param[in] prefix Prefix of the opaq node.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] parent Data parent.
 * @param[in] cbor_value CBOR value item.
 * @param[in,out] first_p First top-level/parent sibling.
 * @param[out] node_p Pointer to the created opaq node.
 * @return LY_ERR value.
 */
static LY_ERR
lydcbor_parse_opaq(struct lyd_cbor_ctx *lydctx, const char *name, size_t name_len, const char *prefix, size_t prefix_len,
                   struct lyd_node *parent, const cbor_item_t *cbor_value, struct lyd_node **first_p, struct lyd_node **node_p)
{
    LY_ERR ret = LY_SUCCESS;
    enum cbor_type type = cbor_typeof(cbor_value);

    LY_CHECK_GOTO(ret = lydcbor_create_opaq(lydctx, name, name_len, prefix, prefix_len, parent, cbor_value, node_p), cleanup);

    assert(*node_p);
    LOG_LOCSET(NULL, *node_p);

    if ((type == CBOR_TYPE_ARRAY) && (cbor_array_size(cbor_value) == 1))
    {
        cbor_item_t **handle = cbor_array_handle(cbor_value);
        if (lydcbor_is_null(handle[0]))
        {
            /* special array null value */
            ((struct lyd_node_opaq *)*node_p)->hints |= LYD_VALHINT_EMPTY;
            goto finish;
        }
    }

    if (type == CBOR_TYPE_ARRAY)
    {
        /* process array */
        size_t array_size = cbor_array_size(cbor_value);
        cbor_item_t **array_handle = cbor_array_handle(cbor_value);

        for (size_t i = 0; i < array_size; ++i)
        {
            const cbor_item_t *item = array_handle[i];

            if (cbor_typeof(item) == CBOR_TYPE_MAP)
            {
                /* array with objects, list */
                ((struct lyd_node_opaq *)*node_p)->hints |= LYD_NODEHINT_LIST;

                /* process children */
                LY_CHECK_GOTO(ret = lydcbor_subtree_r(lydctx, *node_p, lyd_node_child_p(*node_p), NULL, item), cleanup);
            }
            else
            {
                /* array with values, leaf-list */
                ((struct lyd_node_opaq *)*node_p)->hints |= LYD_NODEHINT_LEAFLIST;
            }

            if (i < array_size - 1)
            {
                /* continue with next instance */
                assert(*node_p);
                lydcbor_maintain_children(parent, first_p, node_p,
                                          lydctx->parse_opts & LYD_PARSE_ORDERED ? LYD_INSERT_NODE_LAST : LYD_INSERT_NODE_DEFAULT, NULL);

                LOG_LOCBACK(0, 1);

                LY_CHECK_GOTO(ret = lydcbor_create_opaq(lydctx, name, name_len, prefix, prefix_len, parent, item, node_p), cleanup);

                assert(*node_p);
                LOG_LOCSET(NULL, *node_p);
            }
        }
    }
    else if (type == CBOR_TYPE_MAP)
    {
        ((struct lyd_node_opaq *)*node_p)->hints |= LYD_NODEHINT_CONTAINER;
        /* process children */
        LY_CHECK_GOTO(ret = lydcbor_subtree_r(lydctx, *node_p, lyd_node_child_p(*node_p), NULL, cbor_value), cleanup);
    }

finish:
    /* finish linking metadata */
    ret = lydcbor_metadata_finish(lydctx, lyd_node_child_p(*node_p));

cleanup:
    if (*node_p)
    {
        LOG_LOCBACK(0, 1);
    }
    return ret;
}

/**
 * @brief Process the attribute container (starting by @).
 *
 * @param[in] lydctx CBOR data parser context.
 * @param[in] attr_node The data node referenced by the attribute container.
 * @param[in] snode The schema node of the data node.
 * @param[in] name Name of the node.
 * @param[in] name_len Length of @p name.
 * @param[in] prefix Prefix of the node.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] parent Data parent.
 * @param[in] cbor_value CBOR value item.
 * @param[in,out] first_p First top-level/parent sibling.
 * @param[out] node_p Pointer to the created node.
 * @return LY_ERR value.
 */
static LY_ERR
lydcbor_parse_attribute(struct lyd_cbor_ctx *lydctx, struct lyd_node *attr_node, const struct lysc_node *snode,
                        const char *name, size_t name_len, const char *prefix, size_t prefix_len, struct lyd_node *parent,
                        const cbor_item_t *cbor_value, struct lyd_node **first_p, struct lyd_node **node_p)
{
    LY_ERR r;
    const char *opaq_name, *mod_name, *attr_mod;
    size_t opaq_name_len, attr_mod_len;

    if (!attr_node)
    {
        /* learn the attribute module name */
        if (!snode)
        {
            if (!prefix)
            {
                lydcbor_get_node_prefix(parent, NULL, 0, &attr_mod, &attr_mod_len);
            }
            else
            {
                attr_mod = prefix;
                attr_mod_len = prefix_len;
            }
        }

        /* try to find the instance */
        LY_LIST_FOR(parent ? lyd_child(parent) : *first_p, attr_node)
        {
            if (snode)
            {
                if (attr_node->schema)
                {
                    if (attr_node->schema == snode)
                    {
                        break;
                    }
                }
                else
                {
                    mod_name = ((struct lyd_node_opaq *)attr_node)->name.module_name;
                    if (!strcmp(LYD_NAME(attr_node), snode->name) && mod_name && !strcmp(mod_name, snode->module->name))
                    {
                        break;
                    }
                }
            }
            else
            {
                if (attr_node->schema)
                {
                    mod_name = attr_node->schema->module->name;
                }
                else
                {
                    mod_name = ((struct lyd_node_opaq *)attr_node)->name.module_name;
                }

                if (!ly_strncmp(LYD_NAME(attr_node), name, name_len) && mod_name && attr_mod &&
                    !ly_strncmp(mod_name, attr_mod, attr_mod_len))
                {
                    break;
                }
            }
        }
    }

    if (!attr_node)
    {
        /* parse as an opaq node with @ prefix */
        uint32_t prev_opts;

        prev_opts = lydctx->parse_opts;
        lydctx->parse_opts &= ~LYD_PARSE_STRICT;
        lydctx->parse_opts |= LYD_PARSE_OPAQ;

        opaq_name = prefix ? prefix - 1 : name - 1;
        opaq_name_len = prefix ? prefix_len + name_len + 2 : name_len + 1;
        r = lydcbor_parse_opaq(lydctx, opaq_name, opaq_name_len, NULL, 0, parent, cbor_value, first_p, node_p);

        lydctx->parse_opts = prev_opts;
        LY_CHECK_RET(r);
    }
    else
    {
        LY_CHECK_RET(lydcbor_meta_attr(lydctx, snode, attr_node, cbor_value));
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse a single anydata/anyxml node from CBOR.
 *
 * @param[in] lydctx CBOR data parser context.
 * @param[in] snode Schema node corresponding to the member.
 * @param[in] ext Extension instance of @p snode, if any.
 * @param[in] cbor_value CBOR value item.
 * @param[out] node Parsed data (or opaque) node.
 * @return LY_SUCCESS if a node was successfully parsed.
 * @return LY_ENOT in case of invalid CBOR encoding.
 * @return LY_ERR on other errors.
 */
static LY_ERR
lydcbor_parse_any(struct lyd_cbor_ctx *lydctx, const struct lysc_node *snode, struct lysc_ext_instance *ext,
                  const cbor_item_t *cbor_value, struct lyd_node **node)
{
    LY_ERR r, rc = LY_SUCCESS;
    uint32_t prev_parse_opts = lydctx->parse_opts, prev_int_opts = lydctx->int_opts;
    struct lyd_node *child = NULL;
    ly_bool log_node = 0;
    enum cbor_type type = cbor_typeof(cbor_value);

    assert(snode->nodetype & LYD_NODE_ANY);

    *node = NULL;

    /* status check according to allowed CBOR types */
    if (snode->nodetype == LYS_ANYXML)
    {
        LY_CHECK_RET((type != CBOR_TYPE_MAP) && (type != CBOR_TYPE_ARRAY) && (type != CBOR_TYPE_UINT) &&
                         (type != CBOR_TYPE_NEGINT) && (type != CBOR_TYPE_STRING) && (type != CBOR_TYPE_FLOAT_CTRL),
                     LY_ENOT);
    }
    else
    {
        LY_CHECK_RET(type != CBOR_TYPE_MAP, LY_ENOT);
    }

    /* create any node */
    if (type == CBOR_TYPE_MAP)
    {
        /* create node */
        r = lyd_create_any(snode, NULL, LYD_ANYDATA_DATATREE, 1, node);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        assert(*node);
        LOG_LOCSET(NULL, *node);
        log_node = 1;

        /* parse data tree with correct options */
        lydctx->parse_opts &= ~LYD_PARSE_STRICT;
        lydctx->parse_opts |= LYD_PARSE_OPAQ | (ext ? LYD_PARSE_ONLY : 0);
        lydctx->int_opts |= LYD_INTOPT_ANY | LYD_INTOPT_WITH_SIBLINGS;
        lydctx->any_schema = snode;

        /* process the anydata content */
        r = lydcbor_subtree_r(lydctx, NULL, &child, NULL, cbor_value);
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

        /* finish linking metadata */
        r = lydcbor_metadata_finish(lydctx, &child);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        /* assign the data tree */
        ((struct lyd_node_any *)*node)->value.tree = child;
        child = NULL;
    }
    else
    {
        /* store CBOR value directly */
        r = lyd_create_any(snode, cbor_value, LYD_ANYDATA_CBOR, 0, node);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
    }

cleanup:
    if (log_node)
    {
        LOG_LOCBACK(0, 1);
    }
    lydctx->parse_opts = prev_parse_opts;
    lydctx->int_opts = prev_int_opts;
    lydctx->any_schema = NULL;
    lyd_free_tree(child);
    return rc;
}

/**
 * @brief Parse a single instance of an inner node from CBOR.
 *
 * @param[in] lydctx CBOR data parser context.
 * @param[in] snode Schema node corresponding to the member.
 * @param[in] ext Extension instance of @p snode, if any.
 * @param[in] cbor_value CBOR value item.
 * @param[out] node Parsed data (or opaque) node.
 * @return LY_SUCCESS if a node was successfully parsed.
 * @return LY_ENOT in case of invalid CBOR encoding.
 * @return LY_ERR on other errors.
 */
static LY_ERR
lydcbor_parse_instance_inner(struct lyd_cbor_ctx *lydctx, const struct lysc_node *snode, struct lysc_ext_instance *ext,
                             const cbor_item_t *cbor_value, struct lyd_node **node)
{
    LY_ERR r, rc = LY_SUCCESS;
    uint32_t prev_parse_opts = lydctx->parse_opts;

    LY_CHECK_RET(cbor_typeof(cbor_value) != CBOR_TYPE_MAP, LY_ENOT);

    /* create inner node */
    LY_CHECK_RET(lyd_create_inner(snode, node));

    /* use it for logging */
    LOG_LOCSET(NULL, *node);

    if (ext)
    {
        /* only parse these extension data and validate afterwards */
        lydctx->parse_opts |= LYD_PARSE_ONLY;
    }

    /* process children */
    r = lydcbor_subtree_r(lydctx, *node, lyd_node_child_p(*node), NULL, cbor_value);
    LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

    /* finish linking metadata */
    r = lydcbor_metadata_finish(lydctx, lyd_node_child_p(*node));
    LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

    if (snode->nodetype == LYS_LIST)
    {
        /* check all keys exist */
        r = lyd_parser_check_keys(*node);
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
    }

    if (!(lydctx->parse_opts & LYD_PARSE_ONLY) && !rc)
    {
        /* new node validation */
        r = lyd_parser_validate_new_implicit((struct lyd_ctx *)lydctx, *node);
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
    }

cleanup:
    lydctx->parse_opts = prev_parse_opts;
    LOG_LOCBACK(0, 1);
    if (!(*node)->hash)
    {
        /* list without keys is unusable */
        lyd_free_tree(*node);
        *node = NULL;
    }
    return rc;
}

/**
 * @brief Parse a single instance of a node from CBOR.
 *
 * @param[in] lydctx CBOR data parser context.
 * @param[in] parent Data parent of the subtree.
 * @param[in,out] first_p Pointer to the variable holding the first top-level sibling.
 * @param[in] snode Schema node corresponding to the member.
 * @param[in] ext Extension instance of @p snode, if any.
 * @param[in] name Parsed CBOR node name.
 * @param[in] name_len Length of @p name.
 * @param[in] prefix Parsed CBOR node prefix.
 * @param[in] prefix_len Length of @p prefix.
 * @param[in] cbor_value CBOR value item.
 * @param[out] node Parsed data (or opaque) node.
 * @return LY_SUCCESS if a node was successfully parsed.
 * @return LY_ENOT in case of invalid CBOR encoding.
 * @return LY_ERR on other errors.
 */
static LY_ERR
lydcbor_parse_instance(struct lyd_cbor_ctx *lydctx, struct lyd_node *parent, struct lyd_node **first_p,
                       const struct lysc_node *snode, struct lysc_ext_instance *ext, const char *name, size_t name_len,
                       const char *prefix, size_t prefix_len, const cbor_item_t *cbor_value, struct lyd_node **node)
{
    LY_ERR r, rc = LY_SUCCESS;
    uint32_t type_hints = 0;
    char *str_val = NULL;
    size_t str_len = 0;

    LOG_LOCSET(snode, NULL);

    r = lydcbor_data_check_opaq(lydctx, snode, cbor_value, &type_hints);
    if (r == LY_SUCCESS)
    {
        assert(snode->nodetype & (LYD_NODE_TERM | LYD_NODE_INNER | LYD_NODE_ANY));
        if (lydcbor_is_null(cbor_value))
        {
            /* do not do anything if value is CBOR null */
            goto cleanup;
        }
        else if (snode->nodetype & LYD_NODE_TERM)
        {
            enum cbor_type type = cbor_typeof(cbor_value);

            if ((type == CBOR_TYPE_ARRAY) && (cbor_array_size(cbor_value) == 1))
            {
                cbor_item_t **handle = cbor_array_handle(cbor_value);
                if (lydcbor_is_null(handle[0]))
                {
                    /* [null] case */
                    goto cleanup;
                }
            }

            if ((type != CBOR_TYPE_ARRAY) && (type != CBOR_TYPE_UINT) && (type != CBOR_TYPE_NEGINT) &&
                (type != CBOR_TYPE_STRING) && (type != CBOR_TYPE_FLOAT_CTRL))
            {
                rc = LY_ENOT;
                goto cleanup;
            }

            /* create terminal node */
            r = lydcbor_item_to_string(cbor_value, &str_val, &str_len);
            LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

            r = lyd_parser_create_term((struct lyd_ctx *)lydctx, snode, str_val, str_len, NULL, LY_VALUE_CBOR,
                                       NULL, type_hints, node);
            LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
        }
        else if (snode->nodetype & LYD_NODE_INNER)
        {
            /* create inner node */
            r = lydcbor_parse_instance_inner(lydctx, snode, ext, cbor_value, node);
            LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
        }
        else
        {
            /* create any node */
            r = lydcbor_parse_any(lydctx, snode, ext, cbor_value, node);
            LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
        }
        LY_CHECK_GOTO(!*node, cleanup);

        /* add/correct flags */
        r = lyd_parser_set_data_flags(*node, &(*node)->meta, (struct lyd_ctx *)lydctx, ext);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        if (!(lydctx->parse_opts & LYD_PARSE_ONLY))
        {
            /* store for ext instance node validation, if needed */
            r = lyd_validate_node_ext(*node, &lydctx->ext_node);
            LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
        }
    }
    else if (r == LY_ENOT)
    {
        /* parse it again as an opaq node */
        r = lydcbor_parse_opaq(lydctx, name, name_len, prefix, prefix_len, parent, cbor_value, first_p, node);
        LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

        if (snode->nodetype == LYS_LIST)
        {
            ((struct lyd_node_opaq *)*node)->hints |= LYD_NODEHINT_LIST;
        }
        else if (snode->nodetype == LYS_LEAFLIST)
        {
            ((struct lyd_node_opaq *)*node)->hints |= LYD_NODEHINT_LEAFLIST;
        }
    }
    else
    {
        /* error */
        rc = r;
        goto cleanup;
    }

cleanup:
    free(str_val);
    LOG_LOCBACK(1, 0);
    return rc;
}

/**
 * @brief Parse CBOR subtree. All leaf-list and list instances of a node are considered one subtree.
 *
 * @param[in] lydctx CBOR data parser context.
 * @param[in] parent Data parent of the subtree, must be set if @p first_p is not.
 * @param[in,out] first_p Pointer to the variable holding the first top-level sibling.
 * @param[in,out] parsed Optional set to add all the parsed siblings into.
 * @return LY_ERR value.
 */
static LY_ERR
lydcbor_subtree_r(struct lyd_cbor_ctx *lydctx, struct lyd_node *parent, struct lyd_node **first_p,
    struct ly_set *parsed, const cbor_item_t *cbor_obj)
{
    LY_ERR r, rc = LY_SUCCESS;
    const char *name, *prefix = NULL, *expected = NULL;
    size_t name_len, prefix_len = 0;
    ly_bool is_meta = 0;
    const struct lysc_node *snode = NULL;
    struct lysc_ext_instance *ext = NULL;
    struct lyd_node *node = NULL, *attr_node = NULL;
    const struct ly_ctx *ctx = lydctx->cborctx->ctx;
    struct cbor_pair *pairs;
    size_t map_size;

    assert(parent || first_p);
    assert(cbor_typeof(cbor_obj) == CBOR_TYPE_MAP);

    map_size = cbor_map_size(cbor_obj);
    pairs = cbor_map_handle(cbor_obj);

    if (!pairs && map_size > 0)
    {
        LOGVAL(ctx, LYVE_SYNTAX, "Invalid CBOR map structure");
        return LY_EVALID;
    }

    /* process all members */
    for (size_t i = 0; i < map_size; ++i)
    {
        const cbor_item_t *key_item = pairs[i].key;
        const cbor_item_t *value_item = pairs[i].value;
        char *key_str = NULL;
        size_t key_len = 0;

        if (!key_item || !value_item)
        {
            LOGVAL(ctx, LYVE_SYNTAX, "Null key or value in CBOR map");
            rc = LY_EVALID;
            goto cleanup;
        }

        /* Skip null values */
        if (lydcbor_is_null(value_item))
        {
            continue;
        }

        /* Get key string */
        if (!cbor_isa_string(key_item))
        {
            LOGVAL(ctx, LYVE_SYNTAX, "CBOR map key must be string for named identifier format");
            rc = LY_EVALID;
            goto cleanup;
        }

        LY_CHECK_ERR_GOTO(rc = lydcbor_item_to_string(key_item, &key_str, &key_len), free(key_str), cleanup);

        /* process the node name */
        lydcbor_parse_name(key_str, key_len, &name, &name_len, &prefix, &prefix_len, &is_meta);

        if (!is_meta || name_len || prefix_len)
        {
            /* get the schema node */
            r = lydcbor_get_snode(lydctx, is_meta, prefix, prefix_len, name, name_len, parent, &snode, &ext);
            if (r == LY_ENOT)
            {
                free(key_str);
                continue;
            }
            else if ((r == LY_EVALID) && (lydctx->val_opts & LYD_VALIDATE_MULTI_ERROR))
            {
                rc = r;
                free(key_str);
                continue;
            }
            else if (r)
            {
                rc = r;
                free(key_str);
                goto cleanup;
            }
        }

        if (is_meta)
        {
            /* parse as metadata */
            if (!name_len && !prefix_len && !parent)
            {
                LOGVAL(ctx, LYVE_SYNTAX,
                       "Invalid metadata format - \"@\" can be used only inside anydata, container or list entries.");
                r = LY_EVALID;
                free(key_str);
                LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
            }
            else if (!name_len && !prefix_len)
            {
                /* parent's metadata without a name */
                attr_node = parent;
                snode = attr_node->schema;
            }
            r = lydcbor_parse_attribute(lydctx, attr_node, snode, name, name_len, prefix, prefix_len, parent,
                                        value_item, first_p, &node);
            free(key_str);
            LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
        }
        else if (!snode)
        {
            if (!(lydctx->parse_opts & LYD_PARSE_OPAQ))
            {
                /* skip element */
                free(key_str);
                continue;
            }
            else
            {
                /* parse as an opaq node */
                if (name_len == 0)
                {
                    LOGVAL(ctx, LYVE_SYNTAX, "CBOR object member name cannot be a zero-length string.");
                    r = LY_EVALID;
                    free(key_str);
                    LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
                }

                /* parse opaq */
                r = lydcbor_parse_opaq(lydctx, name, name_len, prefix, prefix_len, parent, value_item, first_p, &node);
                free(key_str);
                LY_CHECK_ERR_GOTO(r, rc = r, cleanup);
            }
        }
        else
        {
            /* parse as a standard lyd_node but it can still turn out to be an opaque node */

            /* set expected representation */
            switch (snode->nodetype)
            {
            case LYS_LEAFLIST:
                expected = "name/array of values";
                break;
            case LYS_LIST:
                expected = "name/array of objects";
                break;
            case LYS_LEAF:
                expected = "name/value";
                break;
            case LYS_CONTAINER:
            case LYS_NOTIF:
            case LYS_ACTION:
            case LYS_RPC:
            case LYS_ANYDATA:
                expected = "name/object";
                break;
            case LYS_ANYXML:
                expected = "name/value";
                break;
            }

            /* check the representation and process */
            enum cbor_type value_type = cbor_typeof(value_item);

            switch (snode->nodetype)
            {
            case LYS_LEAFLIST:
            case LYS_LIST:
                if (value_type != CBOR_TYPE_ARRAY)
                {
                    goto representation_error;
                }

                /* process all values/objects in array */
                size_t array_size = cbor_array_size(value_item);
                cbor_item_t **array_handle = cbor_array_handle(value_item);

                for (size_t j = 0; j < array_size; ++j)
                {
                    const cbor_item_t *item = array_handle[j];

                    r = lydcbor_parse_instance(lydctx, parent, first_p, snode, ext, name, name_len, prefix, prefix_len,
                                               item, &node);
                    if (r == LY_ENOT)
                    {
                        free(key_str);
                        goto representation_error;
                    }
                    LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

                    lydcbor_maintain_children(parent, first_p, &node,
                                              lydctx->parse_opts & LYD_PARSE_ORDERED ? LYD_INSERT_NODE_LAST : LYD_INSERT_NODE_DEFAULT, ext);
                }
                break;
            case LYS_LEAF:
            case LYS_CONTAINER:
            case LYS_NOTIF:
            case LYS_ACTION:
            case LYS_RPC:
            case LYS_ANYDATA:
            case LYS_ANYXML:
                /* process the value/object */
                r = lydcbor_parse_instance(lydctx, parent, first_p, snode, ext, name, name_len, prefix, prefix_len,
                                           value_item, &node);
                if (r == LY_ENOT)
                {
                    free(key_str);
                    goto representation_error;
                }
                LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

                if (snode->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF))
                {
                    /* remember the RPC/action/notification */
                    lydctx->op_node = node;
                }
                break;
            }
        }

        free(key_str);

        /* remember a successfully parsed node */
        if (parsed && node)
        {
            ly_set_add(parsed, node, 1, NULL);
        }

        /* finally connect the parsed node */
        lydcbor_maintain_children(parent, first_p, &node,
                                  lydctx->parse_opts & LYD_PARSE_ORDERED ? LYD_INSERT_NODE_LAST : LYD_INSERT_NODE_DEFAULT, ext);
    }

    /* success */
    goto cleanup;

representation_error:
    LOGVAL(ctx, LYVE_SYNTAX, "Expecting CBOR %s but %s \"%s\" is represented in input data differently.",
           expected, lys_nodetype2str(snode->nodetype), snode->name);
    rc = LY_EVALID;
    if (lydctx->val_opts & LYD_VALIDATE_MULTI_ERROR)
    {
        /* try to skip the invalid data */
        if ((r = lydcbor_data_skip(lydctx->cborctx)))
        {
            rc = r;
        }
    }

cleanup:
    lyd_free_tree(node);
    return rc;
}

/**
 * @brief Common start of CBOR parser processing different types of input data.
 *
 * @param[in] ctx libyang context.
 * @param[in] in Input structure.
 * @param[in] parse_opts Options for parser.
 * @param[in] val_opts Options for validation phase.
 * @param[out] lydctx_p Data parser context to finish validation.
 * @return LY_ERR value.
 */
static LY_ERR
lyd_parse_cbor_init(const struct ly_ctx *ctx, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts,
                    struct lyd_cbor_ctx **lydctx_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_cbor_ctx *lydctx;
    enum cbor_type cbortype;

    assert(lydctx_p);

    /* init context */
    lydctx = calloc(1, sizeof *lydctx);
    LY_CHECK_ERR_RET(!lydctx, LOGMEM(ctx), LY_EMEM);
    lydctx->parse_opts = parse_opts;
    lydctx->val_opts = val_opts;
    lydctx->free = lyd_cbor_ctx_free;

    /* Create low-level CBOR context */
    LY_CHECK_ERR_RET(ret = lycbor_ctx_new(ctx, in, &lydctx->cborctx), free(lydctx), ret);
    cbortype = cbor_typeof(lydctx->cborctx->cbor_data);

    if (!cbor_isa_map(lydctx->cborctx->cbor_data))
    {
        /* expecting top-level map */
        LOGVAL(ctx, LYVE_SYNTAX, "Expected top-level CBOR map, but %d found.", cbortype);
        *lydctx_p = NULL;
        lyd_cbor_ctx_free((struct lyd_ctx *)lydctx);
        return LY_EVALID;
    }

    *lydctx_p = lydctx;
    return ret;
}

LY_ERR
lyd_parse_cbor(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
               struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, uint32_t int_opts,
               struct ly_set *parsed, ly_bool *subtree_sibling, struct lyd_ctx **lydctx_p)
{
    LY_ERR r, rc = LY_SUCCESS;
    struct lyd_cbor_ctx *lydctx = NULL;

    rc = lyd_parse_cbor_init(ctx, in, parse_opts, val_opts, &lydctx);
    LY_CHECK_GOTO(rc, cleanup);

    lydctx->int_opts = int_opts;
    lydctx->ext = ext;

    // DEBUG: print the parsed CBOR data
    printf("DEBUG: Parsed CBOR data:\n");
    print_json(lydctx->cborctx->cbor_data);
    printf("\n");
    // END DEBUG

    /* find the operation node if it exists already */
    LY_CHECK_GOTO(rc = lyd_parser_find_operation(parent, int_opts, &lydctx->op_node), cleanup);

    /* read subtree(s) */
    r = lydcbor_subtree_r(lydctx, parent, first_p, parsed, lydctx->cborctx->cbor_data);
    LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);

    if ((int_opts & LYD_INTOPT_NO_SIBLINGS))
    {
        LOGVAL(ctx, LYVE_SYNTAX, "Unexpected sibling node.");
        r = LY_EVALID;
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
    }
    if ((int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION | LYD_INTOPT_NOTIF | LYD_INTOPT_REPLY)) && !lydctx->op_node)
    {
        LOGVAL(ctx, LYVE_DATA, "Missing the operation node.");
        r = LY_EVALID;
        LY_DPARSER_ERR_GOTO(r, rc = r, lydctx, cleanup);
    }

    /* finish linking metadata */
    r = lydcbor_metadata_finish(lydctx, parent ? lyd_node_child_p(parent) : first_p);
    LY_CHECK_ERR_GOTO(r, rc = r, cleanup);

    if (parse_opts & LYD_PARSE_SUBTREE)
    {
        /* subtree parsing not fully implemented for CBOR */
        if (subtree_sibling)
        {
            *subtree_sibling = 0;
        }
    }

cleanup:
    /* there should be no unresolved types stored */
    assert(!(parse_opts & LYD_PARSE_ONLY) || !lydctx || (!lydctx->node_types.count && !lydctx->meta_types.count && !lydctx->node_when.count));

    if (rc && (!lydctx || !(lydctx->val_opts & LYD_VALIDATE_MULTI_ERROR) || (rc != LY_EVALID)))
    {
        lyd_cbor_ctx_free((struct lyd_ctx *)lydctx);
        lydctx = NULL;
    }
    else
    {
        *lydctx_p = (struct lyd_ctx *)lydctx;
        lycbor_ctx_free(lydctx->cborctx);
        lydctx->cborctx = NULL;
    }
    return rc;
}

#endif /* ENABLE_CBOR_SUPPORT */