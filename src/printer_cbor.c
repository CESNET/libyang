/**
 * @file printer_cbor.c
 * @author Meher Rushi
 * @brief CBOR printer for libyang data tree using libcbor
 *
 * Copyright (c) 2024 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "printer_data.h"

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cbor.h>

#include "context.h"
#include "log.h"
#include "ly_common.h"
#include "out.h"
#include "plugins_exts.h"
#include "printer_internal.h"
#include "set.h"
#include "tree_data.h"
#include "tree_schema.h"

/**
 * @brief CBOR printer context
 */
struct cborpr_ctx {
    struct ly_out *out;           /**< output structure */
    const struct lyd_node *root;  /**< root node of the subtree being printed */
    const struct lyd_node *print_sibling_metadata; /**< node with metadata supposed to be printed */
    ly_bool simple_status;        /**< flag for simple status */

    uint16_t level;               /**< current nesting level */
    uint32_t options;             /**< [printer flags](@ref dataprinterflags) */
    const struct ly_ctx *ctx;     /**< libyang context */

    struct ly_set prefix;         /**< printed module prefixes */
    uint32_t array_index;         /**< index in array if we are printing an array element */
    
    cbor_item_t *root_item;       /**< root CBOR item */
};

/**
 * @brief Check if module needs prefix - ROBUST VERSION
 */
static ly_bool
cbor_module_needs_prefix(struct cborpr_ctx *ctx, const struct lys_module *module)
{
    /* CRITICAL: Add comprehensive null checks */
    if (!ctx) {
        fprintf(stderr, "DEBUG: cbor_module_needs_prefix called with NULL ctx\n");
        return 0;
    }
    
    if (!module) {
        fprintf(stderr, "DEBUG: cbor_module_needs_prefix called with NULL module\n");
        return 0;
    }
    
    if (!module->name) {
        fprintf(stderr, "DEBUG: Module has NULL name\n");
        return 0;
    }
    
    fprintf(stderr, "DEBUG: Checking prefix for module: '%s'\n", module->name);
    
    /* Always add prefix if explicitly requested */
    if (ctx->options & LYD_PRINT_WD_ALL_TAG) {
        fprintf(stderr, "DEBUG: Prefix requested via options\n");
        return 1;
    }
    
    /* Check if it's an internal libyang module */
    if (!strcmp(module->name, "ietf-yang-metadata") ||
        !strcmp(module->name, "yang") ||
        !strcmp(module->name, "ietf-inet-types") ||
        !strcmp(module->name, "ietf-yang-types") ||
        !strcmp(module->name, "ietf-yang-structure-ext")) {
        fprintf(stderr, "DEBUG: Internal module, no prefix needed\n");
        return 0;
    }
    
    /* For now, don't add prefixes unless explicitly requested */
    fprintf(stderr, "DEBUG: No prefix needed for module: '%s'\n", module->name);
    return 0;
}


/**
 * @brief Safe wrapper for cbor_build_string that handles NULL inputs - ENHANCED
 */
static cbor_item_t *
safe_cbor_build_string(const char *str)
{
    if (!str) {
        fprintf(stderr, "DEBUG: NULL string passed to safe_cbor_build_string, using empty string\n");
        return cbor_build_string("");
    }
    
    fprintf(stderr, "DEBUG: Building CBOR string: '%s' (len=%zu)\n", str, strlen(str));
    cbor_item_t *item = cbor_build_string(str);
    if (!item) {
        fprintf(stderr, "DEBUG: cbor_build_string failed for '%s'\n", str);
    }
    return item;
}

/**
 * @brief Convert YANG value to CBOR item
 */
static cbor_item_t *
cbor_value_to_item(const struct lyd_node *node)
{
    const char *str;
    cbor_item_t *item = NULL;
    
    if (!node) {
        fprintf(stderr, "DEBUG: cbor_value_to_item called with NULL node\n");
        return safe_cbor_build_string("");
    }
    
    fprintf(stderr, "DEBUG: cbor_value_to_item - node type: %d\n", node->schema->nodetype);
    
    if (!(node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        fprintf(stderr, "DEBUG: Non-leaf node, returning empty string\n");
        return safe_cbor_build_string(""); /* Empty string for non-leaf nodes */
    }
    
    str = lyd_get_value(node);
    fprintf(stderr, "DEBUG: Node value: '%s'\n", str ? str : "NULL");
    
    if (!str || strlen(str) == 0) {
        fprintf(stderr, "DEBUG: Empty or NULL value, returning empty string\n");
        return safe_cbor_build_string("");
    }
    
    /* FIXED: Add null check for schema before casting */
    if (!node->schema) {
        fprintf(stderr, "DEBUG: Node has NULL schema, using string value\n");
        return cbor_build_string(str);
    }
    
    /* Handle different data types based on YANG type */
    switch (((struct lysc_node_leaf *)node->schema)->type->basetype) {
    case LY_TYPE_BOOL:
        if (strcmp(str, "true") == 0) {
            item = cbor_build_bool(true);
        } else {
            item = cbor_build_bool(false);
        }
        break;
        
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64: {
        char *endptr;
        long long num = strtoll(str, &endptr, 10);
        if (*endptr == '\0') {
            if (num >= 0) {
                item = cbor_build_uint64((uint64_t)num);
            } else {
                item = cbor_build_negint64((uint64_t)(-num - 1));
            }
        } else {
            item = cbor_build_string(str);
        }
        break;
    }
    
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64: {
        char *endptr;
        unsigned long long num = strtoull(str, &endptr, 10);
        if (*endptr == '\0') {
            item = cbor_build_uint64(num);
        } else {
            item = cbor_build_string(str);
        }
        break;
    }
    
    case LY_TYPE_DEC64: {
        char *endptr;
        double num = strtod(str, &endptr);
        if (*endptr == '\0') {
            item = cbor_build_float8(num);
        } else {
            item = cbor_build_string(str);
        }
        break;
    }
    
    case LY_TYPE_EMPTY:
        item = cbor_build_string("");
        break;
        
    default:
        /* String types and others */
        item = cbor_build_string(str);
        break;
    }
    
    return item;
}

/**
 * @brief Count direct children of a node
 */
static size_t
cbor_count_children(const struct lyd_node *node)
{
    size_t count = 0;
    const struct lyd_node *child;
    
    LY_LIST_FOR(lyd_child(node), child) {
        count++;
    }
    return count;
}

/**
 * @brief Count sibling nodes with the same name (for leaf-lists)
 */
static size_t
cbor_count_siblings_same_name(const struct lyd_node *node)
{
    size_t count = 0;
    const struct lyd_node *sibling;
    const struct lysc_node *schema = node->schema;
    
    /* Check if this is a leaf-list */
    if (!(schema->nodetype & LYS_LEAFLIST)) {
        return 1;
    }
    
    /* Count siblings with same schema */
    LY_LIST_FOR(node, sibling) {
        if (sibling->schema == schema) {
            count++;
        } else {
            break; /* leaf-list instances are consecutive */
        }
    }
    
    return count;
}

/**
 * @brief Print a single node recursively
 */
static LY_ERR cbor_print_node(struct cborpr_ctx *ctx, const struct lyd_node *node, cbor_item_t *parent_map);

/**
 * @brief Print container or list node - EXTRA SAFE VERSION
 */
static LY_ERR
cbor_print_container(struct cborpr_ctx *ctx, const struct lyd_node *node, cbor_item_t *parent_map)
{
    cbor_item_t *node_map = NULL;
    cbor_item_t *key_item = NULL;
    char *node_name = NULL;
    const struct lyd_node *child;
    LY_ERR ret = LY_SUCCESS;
    size_t child_count;
    
    fprintf(stderr, "DEBUG: cbor_print_container called for node: %s\n",
            node && node->schema && node->schema->name ? node->schema->name : "NULL");
    
    /* COMPREHENSIVE NULL CHECKS */
    if (!ctx) {
        fprintf(stderr, "DEBUG: Container called with NULL ctx\n");
        return LY_EINVAL;
    }
    
    if (!node) {
        fprintf(stderr, "DEBUG: Container called with NULL node\n");
        return LY_EINVAL;
    }
    
    if (!node->schema) {
        fprintf(stderr, "DEBUG: Container node has NULL schema\n");
        return LY_EINVAL;
    }
    
    if (!parent_map) {
        fprintf(stderr, "DEBUG: Container called with NULL parent_map\n");
        return LY_EINVAL;
    }
    
    /* Get node name - This should now be safe */
    // node_name = cbor_get_node_name(ctx, node);
    node_name = "";    if (!node_name) {
        fprintf(stderr, "DEBUG: Failed to get container node name\n");
        ret = LY_EMEM;
        goto cleanup;
    }
    
    fprintf(stderr, "DEBUG: Container name: '%s'\n", node_name);
    
    /* Count children */
    child_count = cbor_count_children(node);
    fprintf(stderr, "DEBUG: Container has %zu children\n", child_count);
    
    /* Create map for this container/list */
    node_map = cbor_new_definite_map(child_count);
    if (!node_map) {
        fprintf(stderr, "DEBUG: Failed to create CBOR map for container\n");
        ret = LY_EMEM;
        goto cleanup;
    }
    
    /* Add all children to the map */
    LY_LIST_FOR(lyd_child(node), child) {
        if (!child || !child->schema || !child->schema->name) {
            fprintf(stderr, "DEBUG: Skipping invalid child\n");
            continue;
        }
        
        fprintf(stderr, "DEBUG: Processing child: %s\n", child->schema->name);
        ret = cbor_print_node(ctx, child, node_map);
        if (ret != LY_SUCCESS) {
            fprintf(stderr, "DEBUG: Failed to process child node\n");
            goto cleanup;
        }
    }
    
    /* Add this container/list to parent map */
    key_item = safe_cbor_build_string(node_name);
    if (!key_item) {
        fprintf(stderr, "DEBUG: Failed to create key item for container\n");
        ret = LY_EMEM;
        goto cleanup;
    }
    
    if (!cbor_map_add(parent_map, (struct cbor_pair) {
        .key = key_item,
        .value = node_map
    })) {
        fprintf(stderr, "DEBUG: Failed to add container to parent map\n");
        ret = LY_EMEM;
        goto cleanup;
    }
    
    fprintf(stderr, "DEBUG: Container added successfully\n");
    
    /* Items are now owned by the map, don't decref them */
    key_item = NULL;
    node_map = NULL;

cleanup:
    if (key_item) {
        cbor_decref(&key_item);
    }
    if (node_map) {
        cbor_decref(&node_map);
    }
    if (node_name) {
        free(node_name);
    }
    return ret;
}

/**
 * @brief Print leaf or leaf-list node
 */
static LY_ERR
cbor_print_leaf(struct cborpr_ctx *ctx, const struct lyd_node *node, cbor_item_t *parent_map)
{
    cbor_item_t *key_item = NULL;
    cbor_item_t *value_item = NULL;
    cbor_item_t *array_item = NULL;
    char *node_name = NULL;
    const struct lyd_node *sibling;
    LY_ERR ret = LY_SUCCESS;
    size_t sibling_count;
    
    fprintf(stderr, "DEBUG: cbor_print_leaf called for node: %s\n", 
            node && node->schema && node->schema->name ? node->schema->name : "NULL");
    
    /* FIXED: Add null checks */
    if (!node || !node->schema) {
        fprintf(stderr, "DEBUG: Leaf node or schema is NULL\n");
        return LY_EINVAL;
    }
    
    /* Get node name */
    // node_name = cbor_get_node_name(ctx, node);
    node_name = "";
    if (!node_name) {
        fprintf(stderr, "DEBUG: Failed to get node name\n");
        ret = LY_EMEM;
        goto cleanup;
    }
    
    fprintf(stderr, "DEBUG: Got node name: '%s'\n", node_name);
    
    /* Check if this is a leaf-list with multiple values */
    sibling_count = cbor_count_siblings_same_name(node);
    fprintf(stderr, "DEBUG: Sibling count: %zu\n", sibling_count);
    
    if (sibling_count > 1 && (node->schema->nodetype & LYS_LEAFLIST)) {
        fprintf(stderr, "DEBUG: Processing leaf-list with %zu values\n", sibling_count);
        
        /* Create array for leaf-list */
        array_item = cbor_new_definite_array(sibling_count);
        if (!array_item) {
            fprintf(stderr, "DEBUG: Failed to create CBOR array\n");
            ret = LY_EMEM;
            goto cleanup;
        }
        
        /* Add all values to array */
        LY_LIST_FOR(node, sibling) {
            if (sibling->schema != node->schema) {
                break; /* Different schema, stop */
            }
            
            fprintf(stderr, "DEBUG: Adding leaf-list value to array\n");
            value_item = cbor_value_to_item(sibling);
            if (!value_item) {
                fprintf(stderr, "DEBUG: Failed to create CBOR value item\n");
                ret = LY_EMEM;
                goto cleanup;
            }
            
            if (!cbor_array_push(array_item, value_item)) {
                fprintf(stderr, "DEBUG: Failed to add item to CBOR array\n");
                cbor_decref(&value_item);
                ret = LY_EMEM;
                goto cleanup;
            }
            
            value_item = NULL; /* Array owns it now */
        }
        
        /* Add array to parent map */
        key_item = safe_cbor_build_string(node_name);
        if (!key_item) {
            fprintf(stderr, "DEBUG: Failed to create CBOR key item for leaf-list\n");
            ret = LY_EMEM;
            goto cleanup;
        }
        
        if (!cbor_map_add(parent_map, (struct cbor_pair) {
            .key = key_item,
            .value = array_item
        })) {
            fprintf(stderr, "DEBUG: Failed to add leaf-list to parent map\n");
            ret = LY_EMEM;
            goto cleanup;
        }
        
        /* Items are now owned by the map */
        key_item = NULL;
        array_item = NULL;
        
    } else {
        fprintf(stderr, "DEBUG: Processing single leaf value\n");
        
        /* Single leaf value */
        value_item = cbor_value_to_item(node);
        if (!value_item) {
            fprintf(stderr, "DEBUG: Failed to create CBOR value item for leaf\n");
            ret = LY_EMEM;
            goto cleanup;
        }
        
        key_item = safe_cbor_build_string(node_name);
        if (!key_item) {
            fprintf(stderr, "DEBUG: Failed to create CBOR key item for leaf\n");
            ret = LY_EMEM;
            goto cleanup;
        }
        
        if (!cbor_map_add(parent_map, (struct cbor_pair) {
            .key = key_item,
            .value = value_item
        })) {
            fprintf(stderr, "DEBUG: Failed to add leaf to parent map\n");
            ret = LY_EMEM;
            goto cleanup;
        }
        
        /* Items are now owned by the map */
        key_item = NULL;
        value_item = NULL;
    }
    
    fprintf(stderr, "DEBUG: cbor_print_leaf completed successfully\n");

cleanup:
    if (key_item) {
        cbor_decref(&key_item);
    }
    if (value_item) {
        cbor_decref(&value_item);
    }
    if (array_item) {
        cbor_decref(&array_item);
    }
    free(node_name);
    return ret;
}

/**
 * @brief Print anydata/anyxml node
 */
static LY_ERR
cbor_print_any(struct cborpr_ctx *ctx, const struct lyd_node *node, cbor_item_t *parent_map)
{
    cbor_item_t *key_item = NULL;
    cbor_item_t *value_item = NULL;
    char *node_name = NULL;
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node_any *any = (struct lyd_node_any *)node;
    const char *value_str = "";
    
    /* FIXED: Add null checks */
    if (!node || !node->schema) {
        fprintf(stderr, "DEBUG: Any node or schema is NULL\n");
        return LY_EINVAL;
    }
    
    /* Get node name */
    // node_name = cbor_get_node_name(ctx, node);
    node_name = "";    if (!node_name) {
        ret = LY_EMEM;
        goto cleanup;
    }
    
    /* Convert anydata to string representation for now */
    /* TODO: Could be enhanced to preserve the actual data format */
    switch (any->value_type) {
    case LYD_ANYDATA_STRING:
        value_str = any->value.str ? (char *)any->value.str : "";
        break;
    case LYD_ANYDATA_DATATREE:
        /* For now, just indicate it's a data tree */
        value_str = "[DATA TREE]";
        break;
    case LYD_ANYDATA_XML:
        value_str = any->value.str ? (char *)any->value.str : "";
        break;
    case LYD_ANYDATA_JSON:
        value_str = any->value.str ? (char *)any->value.str : "";
        break;
    default:
        value_str = "";
        break;
    }
    
    value_item = cbor_build_string(value_str);
    if (!value_item) {
        ret = LY_EMEM;
        goto cleanup;
    }
    
    key_item = cbor_build_string(node_name);
    if (!key_item) {
        ret = LY_EMEM;
        goto cleanup;
    }
    
    if (!cbor_map_add(parent_map, (struct cbor_pair) {
        .key = key_item,
        .value = value_item
    })) {
        ret = LY_EMEM;
        goto cleanup;
    }
    
    /* Items are now owned by the map */
    key_item = NULL;
    value_item = NULL;

cleanup:
    if (key_item) {
        cbor_decref(&key_item);
    }
    if (value_item) {
        cbor_decref(&value_item);
    }
    free(node_name);
    return ret;
}

/**
 * @brief Print a single node recursively
 */
static LY_ERR
cbor_print_node(struct cborpr_ctx *ctx, const struct lyd_node *node, cbor_item_t *parent_map)
{
    /* FIXED: Add null checks at the beginning */
    if (!node || !node->schema) {
        fprintf(stderr, "DEBUG: cbor_print_node called with NULL node or schema\n");
        return LY_EINVAL;
    }
    
    switch (node->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
        return cbor_print_container(ctx, node, parent_map);
    case LYS_LEAF:
    case LYS_LEAFLIST:
        return cbor_print_leaf(ctx, node, parent_map);
    case LYS_ANYXML:
    case LYS_ANYDATA:
        return cbor_print_any(ctx, node, parent_map);
    default:
        /* Skip unknown node types */
        fprintf(stderr, "DEBUG: Skipping unknown node type: %d\n", node->schema->nodetype);
        return LY_SUCCESS;
    }
}

/**
 * @brief Count root level nodes, handling leaf-lists correctly
 */
static size_t
cbor_count_root_nodes(const struct lyd_node *root)
{
    size_t count = 0;
    const struct lyd_node *node;
    const struct lysc_node *last_schema = NULL;
    
    LY_LIST_FOR(root, node) {
        /* FIXED: Add null check for schema */
        if (!node->schema) {
            continue;
        }
        
        /* For leaf-lists, only count the first occurrence */
        if (node->schema != last_schema) {
            count++;
            last_schema = node->schema;
        } else if (!(node->schema->nodetype & LYS_LEAFLIST)) {
            count++;
        }
    }
    
    return count;
}

/**
 * @brief Main function to print data tree in CBOR format
 */
LY_ERR
cbor_print_data(struct ly_out *out, const struct lyd_node *root, uint32_t options)
{
    LY_ERR ret = LY_SUCCESS;
    struct cborpr_ctx ctx = {0};
    const struct lyd_node *node;
    const struct lysc_node *last_schema = NULL;
    size_t root_count;
    unsigned char *cbor_data = NULL;
    size_t cbor_data_len = 0;
    
    if (!out) {
        return LY_EINVAL;
    }
    
    /* Initialize context */
    ctx.out = out;
    ctx.root = root;
    ctx.options = options;
    ctx.level = 0;
    
    if (root) {
        ctx.ctx = LYD_CTX(root);
        
        /* Count root level nodes */
        root_count = cbor_count_root_nodes(root);
        
        /* Debug: Print what we're processing */
        fprintf(stderr, "DEBUG: Processing %zu root nodes\n", root_count);
        
        /* Create root map */
        ctx.root_item = cbor_new_definite_map(root_count);
        if (!ctx.root_item) {
            fprintf(stderr, "DEBUG: Failed to create root map\n");
            ret = LY_EMEM;
            goto cleanup;
        }
        
        /* Process all root nodes */
        LY_LIST_FOR(root, node) {
            /* FIXED: Add null check for schema */
            if (!node->schema) {
                fprintf(stderr, "DEBUG: Skipping node with NULL schema\n");
                continue;
            }
            
            /* Skip duplicate leaf-list entries (they are handled together) */
            if ((node->schema->nodetype & LYS_LEAFLIST) && (node->schema == last_schema)) {
                continue;
            }
            
            fprintf(stderr, "DEBUG: Processing node: %s\n", node->schema->name);
            
            ctx.root = node;
            ret = cbor_print_node(&ctx, node, ctx.root_item);
            if (ret != LY_SUCCESS) {
                fprintf(stderr, "DEBUG: Failed to print node: %s\n", node->schema->name);
                goto cleanup;
            }
            
            last_schema = node->schema;
            
            /* Break if not printing siblings */
            if (!(options & LYD_PRINT_WITHSIBLINGS)) {
                break;
            }
        }
    } else {
        fprintf(stderr, "DEBUG: Empty data tree\n");
        /* Empty data tree - create empty map */
        ctx.root_item = cbor_new_definite_map(0);
        if (!ctx.root_item) {
            ret = LY_EMEM;
            goto cleanup;
        }
    }
    
    /* Serialize CBOR to bytes */
    cbor_data_len = cbor_serialize_alloc(ctx.root_item, &cbor_data, &cbor_data_len);
    if (cbor_data_len == 0 || !cbor_data) {
        fprintf(stderr, "DEBUG: Failed to serialize CBOR or got 0 bytes\n");
        ret = LY_EMEM;
        goto cleanup;
    }
    
    fprintf(stderr, "DEBUG: Generated %zu bytes of CBOR data\n", cbor_data_len);
    
    /* Write to output using ly_print_ macro */
    ly_print_(out, "%.*s", (int)cbor_data_len, cbor_data);

cleanup:
    if (ctx.root_item) {
        cbor_decref(&ctx.root_item);
    }
    if (cbor_data) {
        free(cbor_data);
    }
    
    return ret;
}

/**
 * @brief Print data subtree in CBOR format
 */
LY_ERR
cbor_print_tree(struct ly_out *out, const struct lyd_node *root, uint32_t options, size_t max_depth)
{
    /* For now, ignore max_depth and use the regular print function */
    /* TODO: Implement depth limiting if needed */
    (void)max_depth;
    return cbor_print_data(out, root, options);
}

/**
 * @brief Print all data trees in CBOR format
 */
LY_ERR
cbor_print_all(struct ly_out *out, const struct lyd_node *root, uint32_t options)
{
    return cbor_print_data(out, root, options);
}