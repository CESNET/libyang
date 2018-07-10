/**
 * @file parser_lyb.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief LYB data parser for libyang
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"
#include "common.h"
#include "context.h"
#include "parser.h"
#include "tree_internal.h"

#define LYB_HAVE_READ_GOTO(r, d, go) if (r < 0) goto go; d += r;
#define LYB_HAVE_READ_RETURN(r, d, ret) if (r < 0) return ret; d += r;

static int
lyb_read(const char *data, uint8_t *buf, size_t count, struct lyb_state *lybs)
{
    int ret = 0, i, empty_chunk_i;
    size_t to_read;

    assert(data && lybs);

    while (1) {
        /* check for fully-read (empty) data chunks */
        to_read = count;
        empty_chunk_i = -1;
        for (i = 0; i < lybs->used; ++i) {
            /* we want the innermost chunks resolved first, so replace previous empty chunks,
             * also ignore chunks that are completely finished, there is nothing for us to do */
            if ((lybs->written[i] <= count) && lybs->position[i]) {
                /* empty chunk, do not read more */
                to_read = lybs->written[i];
                empty_chunk_i = i;
            }
        }

        if ((empty_chunk_i == -1) && !count) {
            break;
        }

        /* we are actually reading some data, not just finishing another chunk */
        if (to_read) {
            if (buf) {
                memcpy(buf, data + ret, to_read);
            }

            for (i = 0; i < lybs->used; ++i) {
                /* decrease all written counters */
                lybs->written[i] -= to_read;
            }
            /* decrease count/buf */
            count -= to_read;
            if (buf) {
                buf += to_read;
            }

            ret += to_read;
        }

        if (empty_chunk_i > -1) {
            /* read the next chunk size */
            memcpy(&lybs->written[empty_chunk_i], data + ret, LYB_SIZE_BYTES);
            /* remember whether there is a following chunk or not */
            lybs->position[empty_chunk_i] = (lybs->written[empty_chunk_i] == LYB_SIZE_MAX ? 1 : 0);

            ret += LYB_SIZE_BYTES;
        }
    }

    return ret;
}

static int
lyb_read_number(uint64_t *num, uint64_t max_num, const char *data, struct lyb_state *lybs)
{
    int max_bits, max_bytes, i, r, ret = 0;
    uint8_t byte;

    for (max_bits = 0; max_num; max_num >>= 1, ++max_bits);
    max_bytes = max_bits / 8 + (max_bits % 8 ? 1 : 0);

    for (i = 0; i < max_bytes; ++i) {
        ret += (r = lyb_read(data, &byte, 1, lybs));
        LYB_HAVE_READ_RETURN(r, data, -1);

        *(((uint8_t *)num) + i) = byte;
    }

    return ret;
}

static int
lyb_read_string(struct ly_ctx *ctx, const char **str, const char *data, struct lyb_state *lybs)
{
    int ret;
    size_t len;

    /* read until the end of this subtree */
    len = lybs->written[lybs->used - 1];

    *str = malloc((len + 1) * sizeof **str);
    LY_CHECK_ERR_RETURN(!*str, LOGMEM(ctx), -1);

    ret = lyb_read(data, (uint8_t *)*str, len, lybs);
    LYB_HAVE_READ_GOTO(ret, data, error);
    ((char *)*str)[len] = '\0';

    /* store in the dictionary */
    *str = lydict_insert_zc(ctx, (char *)*str);

    return ret;

error:
    free((char *)*str);
    *str = NULL;
    return -1;
}

static void
lyb_read_stop_subtree(struct lyb_state *lybs)
{
    if (lybs->written[lybs->used - 1]) {
        LOGINT(NULL);
    }

    --lybs->used;
}

static int
lyb_read_start_subtree(const char *data, struct lyb_state *lybs)
{
    uint64_t num = 0;

    if (lybs->used == lybs->size) {
        lybs->size += LYB_STATE_STEP;
        lybs->written = ly_realloc(lybs->written, lybs->size * sizeof *lybs->written);
        lybs->position = ly_realloc(lybs->position, lybs->size * sizeof *lybs->position);
        LY_CHECK_ERR_RETURN(!lybs->written || !lybs->position, LOGMEM(NULL), -1);
    }

    memcpy(&num, data, LYB_SIZE_BYTES);

    ++lybs->used;
    lybs->written[lybs->used - 1] = num;
    lybs->position[lybs->used - 1] = (num == LYB_SIZE_MAX ? 1 : 0);

    return LYB_SIZE_BYTES;
}

static int
lyb_parse_model(struct ly_ctx *ctx, const char *data, const struct lys_module **mod, struct lyb_state *lybs)
{
    int r, ret = 0;
    uint16_t num = 0;
    char *mod_name = NULL, mod_rev[11];

    /* model name length */
    ret += (r = lyb_read(data, (uint8_t *)&num, sizeof(uint16_t), lybs));
    LYB_HAVE_READ_GOTO(r, data, error);

    mod_name = malloc(num + 1);
    LY_CHECK_ERR_GOTO(!mod_name, LOGMEM(ctx), error);

    /* model name */
    ret += (r = lyb_read(data, (uint8_t *)mod_name, num, lybs));
    LYB_HAVE_READ_GOTO(r, data, error);
    mod_name[num] = '\0';

    /* revision */
    ret += (r = lyb_read(data, (uint8_t *)&num, sizeof(uint16_t), lybs));
    LYB_HAVE_READ_GOTO(r, data, error);

    if (num) {
        sprintf(mod_rev, "%04u-%02u-%02u", ((num & 0xFE00) >> 9) + 2000, (num & 0x01E0) >> 5, (num & 0x001F));
        *mod = ly_ctx_get_module(ctx, mod_name, mod_rev, 0);
    } else {
        *mod = ly_ctx_get_module(ctx, mod_name, NULL, 0);
    }
    if (!*mod) {
        LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Module \"%s@%s\" not found in the context.", mod_name, (num ? mod_rev : "<none>"));
        goto error;
    }

    free(mod_name);
    return ret;

error:
    free(mod_name);
    return -1;
}

static struct lyd_node *
lyb_new_node(const struct lys_node *schema)
{
    struct lyd_node *node;

    switch (schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_NOTIF:
    case LYS_RPC:
    case LYS_ACTION:
        node = calloc(sizeof(struct lyd_node), 1);
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        node = calloc(sizeof(struct lyd_node_leaf_list), 1);
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        node = calloc(sizeof(struct lyd_node_anydata), 1);
        break;
    default:
        return NULL;
    }
    LY_CHECK_ERR_RETURN(!node, LOGMEM(schema->module->ctx), NULL);

    /* fill basic info */
    node->schema = (struct lys_node *)schema;
    node->validity = ly_new_node_validity(schema);
    if (resolve_applies_when(schema, 0, NULL)) {
        node->when_status = LYD_WHEN;
    }
    node->prev = node;

    return node;
}

static int
lyb_parse_anydata(struct lyd_node *node, const char *data, struct lyb_state *lybs)
{
    int r, ret = 0;
    struct lyd_node_anydata *any = (struct lyd_node_anydata *)node;

    /* read value type */
    ret += (r = lyb_read(data, (uint8_t *)&any->value_type, sizeof any->value_type, lybs));
    LYB_HAVE_READ_RETURN(r, data, -1);

    /* read anydata content */
    if (any->value_type == LYD_ANYDATA_DATATREE) {
        any->value.tree = lyd_parse_lyb(node->schema->module->ctx, data, 0, NULL, NULL, &r);
        ret += r;
        LYB_HAVE_READ_RETURN(r, data, -1);
    } else {
        ret += (r = lyb_read_string(node->schema->module->ctx, &any->value.str, data, lybs));
        LYB_HAVE_READ_RETURN(r, data, -1);
    }

    return ret;
}

static int
lyb_parse_val(struct lyd_node_leaf_list *node, const char *data, struct lyb_state *lybs)
{
    int r, ret;
    size_t i;
    uint8_t byte;
    uint64_t num;
    struct ly_ctx *ctx = node->schema->module->ctx;
    struct lys_type *type = &((struct lys_node_leaf *)node->schema)->type;

    if (node->value_flags & (LY_VALUE_USER || LY_VALUE_UNRES)) {
        /* just read value_str */
        return lyb_read_string(ctx, &node->value_str, data, lybs);
    }

    switch (node->value_type) {
    case LY_TYPE_INST:
    case LY_TYPE_IDENT:
    case LY_TYPE_UNION:
        /* we do not actually fill value now, but value_str */
        ret = lyb_read_string(ctx, &node->value_str, data, lybs);
        break;
    case LY_TYPE_BINARY:
    case LY_TYPE_STRING:
    case LY_TYPE_UNKNOWN:
        /* read string */
        ret = lyb_read_string(ctx, &node->value.string, data, lybs);
        break;
    case LY_TYPE_BITS:
        /* find the correct structure */
        for (; !type->info.bits.count; type = &type->der->type);

        node->value.bit = calloc(type->info.bits.count, sizeof *node->value.bit);
        LY_CHECK_ERR_RETURN(!node->value.bit, LOGMEM(ctx), -1);

        /* read values */
        ret = 0;
        for (i = 0; i < type->info.bits.count; ++i) {
            if (i % 8 == 0) {
                /* read another byte */
                ret += (r = lyb_read(data, &byte, sizeof byte, lybs));
                if (r < 0) {
                    return -1;
                }
            }

            if (byte & (0x01 << (i % 8))) {
                /* bit is set */
                node->value.bit[i] = &type->info.bits.bit[i];
            }
        }
        break;
    case LY_TYPE_BOOL:
        /* read byte */
        ret = lyb_read(data, &byte, sizeof byte, lybs);
        if ((ret > 0) && byte) {
            node->value.bln = 1;
        }
        break;
    case LY_TYPE_EMPTY:
        /* nothing to read */
        ret = 0;
        break;
    case LY_TYPE_ENUM:
        /* find the correct structure */
        for (; !type->info.enums.count; type = &type->der->type);

        num = 0;
        ret = lyb_read_number(&num, type->info.enums.enm[type->info.enums.count - 1].value, data, lybs);
        if ((ret > 0) && (num < type->info.enums.count)) {
            node->value.enm = &type->info.enums.enm[num];
        }
        break;
    case LY_TYPE_INT8:
    case LY_TYPE_UINT8:
        ret = lyb_read_number((uint64_t *)&node->value.uint8, UINT8_MAX, data, lybs);
        break;
    case LY_TYPE_INT16:
    case LY_TYPE_UINT16:
        ret = lyb_read_number((uint64_t *)&node->value.uint16, UINT16_MAX, data, lybs);
        break;
    case LY_TYPE_INT32:
    case LY_TYPE_UINT32:
        ret = lyb_read_number((uint64_t *)&node->value.uint32, UINT32_MAX, data, lybs);
        break;
    case LY_TYPE_DEC64:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
        ret = lyb_read_number((uint64_t *)&node->value.uint64, UINT64_MAX, data, lybs);
        break;
    default:
        return -1;
    }

    return ret;
}

static int
lyb_parse_val_str(struct lyd_node_leaf_list *node, struct unres_data *unres)
{
    struct ly_ctx *ctx = node->schema->module->ctx;
    struct lys_type *type = &((struct lys_node_leaf *)node->schema)->type;
    char num_str[22], *str;
    uint32_t i, str_len;

    if (node->value_flags & LY_VALUE_UNRES) {
        /* nothing to do */
        return 0;
    }

    if (node->value_flags & LY_VALUE_USER) {
        /* unfortunately, we need to also fill the value properly, so just parse it again */
        node->value_flags &= ~LY_VALUE_USER;
        if (!lyp_parse_value(type, &node->value_str, NULL, node, NULL,
                             lyd_node_module((struct lyd_node *)node), 1, node->dflt, 1)) {
            return -1;
        }

        if (!(node->value_flags & LY_VALUE_USER)) {
            LOGWRN(ctx, "Node \"%s\" value was stored as a user type, but it is not in the current context.", node->schema->name);
        }
        return 0;
    }

    switch (node->value_type) {
    case LY_TYPE_INST:
        /* try to fill the instance-identifier target now */
        if (unres_data_add(unres, (struct lyd_node *)node, UNRES_INSTID)) {
            return -1;
        }
        break;
    case LY_TYPE_IDENT:
        /* fill the identity pointer now */
        node->value.ident = resolve_identref(type, node->value_str, (struct lyd_node *)node, node->schema->module, node->dflt);
        if (!node->value.ident) {
            return -1;
        }
        break;
    case LY_TYPE_BINARY:
    case LY_TYPE_STRING:
    case LY_TYPE_UNKNOWN:
        /* just re-assign it */
        node->value_str = node->value.string;
        break;
    case LY_TYPE_BITS:
        for (; !type->info.bits.count; type = &type->der->type);

        /* print the set bits */
        str = malloc(1);
        LY_CHECK_ERR_RETURN(!str, LOGMEM(ctx), -1);
        str[0] = '\0';
        str_len = 1;
        for (i = 0; i < type->info.bits.count; ++i) {
            if (node->value.bit[i]) {
                str = ly_realloc(str, str_len + strlen(node->value.bit[i]->name) + 1);
                LY_CHECK_ERR_RETURN(!str, LOGMEM(ctx), -1);

                sprintf(str + str_len, "%s%s", str_len == 1 ? "" : " ", node->value.bit[i]->name);

                str_len += strlen(node->value.bit[i]->name) + 1;
            }
        }

        node->value_str = lydict_insert_zc(ctx, str);
        break;
    case LY_TYPE_BOOL:
        node->value_str = lydict_insert(ctx, (node->value.bln ? "true" : "false"), 0);
        break;
    case LY_TYPE_EMPTY:
    case LY_TYPE_UNION:
        /* leave value empty */
        break;
    case LY_TYPE_ENUM:
        /* print the value */
        node->value_str = lydict_insert(ctx, node->value.enm->name, 0);
        break;
    case LY_TYPE_INT8:
        sprintf(num_str, "%d", node->value.int8);
        node->value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_UINT8:
        sprintf(num_str, "%u", node->value.uint8);
        node->value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_INT16:
        sprintf(num_str, "%d", node->value.int16);
        node->value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_UINT16:
        sprintf(num_str, "%u", node->value.uint16);
        node->value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_INT32:
        sprintf(num_str, "%d", node->value.int32);
        node->value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_UINT32:
        sprintf(num_str, "%u", node->value.uint32);
        node->value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_INT64:
        sprintf(num_str, "%ld", node->value.int64);
        node->value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_UINT64:
        sprintf(num_str, "%lu", node->value.uint64);
        node->value_str = lydict_insert(ctx, num_str, 0);
        break;
    case LY_TYPE_DEC64:
        sprintf(num_str, "%ld.%ld", node->value.dec64 / (type->info.dec64.dig * 10), node->value.dec64 % (type->info.dec64.dig * 10));
        node->value_str = lydict_insert(ctx, num_str, 0);
        break;
    default:
        return -1;
    }

    return 0;
}

static int
lyb_parse_leaf(struct lyd_node *node, const char *data, struct unres_data *unres, struct lyb_state *lybs)
{
    int r, ret = 0;
    uint8_t start_byte;
    struct lyd_node_leaf_list *leaf = (struct lyd_node_leaf_list *)node;

    /* read value type and flags on the first byte */
    ret += (r = lyb_read(data, &start_byte, sizeof start_byte, lybs));
    LYB_HAVE_READ_RETURN(r, data, -1);

    /* fill value type, flags */
    leaf->value_type = start_byte & 0x1F;
    if (start_byte & 0x80) {
        leaf->dflt = 1;
    }
    if (start_byte & 0x40) {
        leaf->value_flags |= LY_VALUE_USER;
    }
    if (start_byte & 0x20) {
        leaf->value_flags |= LY_VALUE_UNRES;
    }

    ret += (r = lyb_parse_val(leaf, data, lybs));
    LYB_HAVE_READ_RETURN(r, data, -1);

    ret += (r = lyb_parse_val_str(leaf, unres));
    LYB_HAVE_READ_RETURN(r, data, -1);

    return ret;
}

static int
lyb_is_schema_hash_match(struct lys_node *sibling, LYB_HASH hash)
{
    LYB_HASH sibling_hash;
    uint8_t collision_id;

    /* get collision ID */
    for (collision_id = 0; !(hash & (LYB_HASH_COLLISION_ID >> collision_id)); ++collision_id);

    /* get correct collision ID node hash */
    sibling_hash = lyb_hash(sibling, collision_id);

    return hash == sibling_hash;
}

static int
lyb_parse_schema_hash(const struct lys_node *sparent, const struct lys_module *mod, const char *data, const char *yang_data_name,
                      int options, struct lys_node **snode, struct lyb_state *lybs)
{
    int r, ret = 0;
    struct lys_node *sibling = NULL;
    LYB_HASH hash = 0;
    struct ly_ctx *ctx;

    assert((sparent || mod) && (!sparent || !mod));
    ctx = (sparent ? sparent->module->ctx : mod->ctx);

    /* read the hash */
    ret += (r = lyb_read(data, &hash, sizeof hash, lybs));
    LYB_HAVE_READ_RETURN(r, data, -1);
    if (!hash) {
        return -1;
    }

    while ((sibling = (struct lys_node *)lys_getnext(sibling, sparent, mod, 0))) {
        /* skip schema nodes from models not present during printing */
        if (lyb_has_schema_model(sibling, lybs->models, lybs->mod_count) && lyb_is_schema_hash_match(sibling, hash)) {
            /* match found */
            break;
        }
    }

    *snode = sibling;
    if (!sibling && (options & LYD_OPT_STRICT)) {
        if (mod) {
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_NONE, NULL, "Failed to find matching hash for a top-level node from \"%s\".", mod->name);
        } else {
            LOGVAL(ctx, LYE_SPEC, LY_VLOG_LYS, sparent, "Failed to find matching hash for a child of \"%s\".", sparent->name);
        }
        return -1;
    }

    return ret;
}

static int
lyb_parse_subtree(struct ly_ctx *ctx, const char *data, struct lyd_node *parent, struct lyd_node **first_sibling,
                  const char *yang_data_name, int options, struct unres_data *unres, struct lyb_state *lybs)
{
    int r, ret = 0;
    struct lyd_node *node = NULL, *iter;
    const struct lys_module *mod;
    struct lys_node *sparent, *snode;

    assert((parent && !first_sibling) || (!parent && first_sibling));

    /* register a new subtree */
    ret += (r = lyb_read_start_subtree(data, lybs));
    LYB_HAVE_READ_GOTO(r, data, error);

    if (!parent) {
        /* top-level, read module name */
        ret += (r = lyb_parse_model(ctx, data, &mod, lybs));
        LYB_HAVE_READ_GOTO(r, data, error);

        sparent = NULL;
    } else {
        mod = NULL;
        sparent = parent->schema;
    }

    /* read hash, find the schema node */
    ret += (r = lyb_parse_schema_hash(sparent, mod, data, yang_data_name, options, &snode, lybs));
    LYB_HAVE_READ_GOTO(r, data, error);

    if (!snode) {
        /* unknown data subtree, skip it */
        ret += (r = lyb_read(data, NULL, lybs->written[lybs->used - 1], lybs));
        goto stop_subtree;
    }

    /*
     * read the node
     */
    node = lyb_new_node(snode);
    if (!node) {
        goto error;
    }

    /* TODO read attributes */

    /* read node content */
    switch (snode->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
        /* nothing to read */
        break;
    case LYS_LEAF:
    case LYS_LEAFLIST:
        ret += (r = lyb_parse_leaf(node, data, unres, lybs));
        LYB_HAVE_READ_GOTO(r, data, error);
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        ret += (r = lyb_parse_anydata(node, data, lybs));
        LYB_HAVE_READ_GOTO(r, data, error);
        break;
    default:
        goto error;
    }

    /* insert into data tree, manually */
    if (parent) {
        if (!parent->child) {
            /* only child */
            parent->child = node;
        } else {
            /* last child */
            parent->child->prev->next = node;
            node->prev = parent->child->prev;
            parent->child->prev = node;
        }
        node->parent = parent;
    } else if (*first_sibling) {
        /* last sibling */
        (*first_sibling)->prev->next = node;
        node->prev = (*first_sibling)->prev;
        (*first_sibling)->prev = node;
    } else {
        /* only sibling */
        *first_sibling = node;
    }

    /* read all descendants */
    while (lybs->written[lybs->used - 1]) {
        ret += (r = lyb_parse_subtree(ctx, data, node, NULL, NULL, options, unres, lybs));
        LYB_HAVE_READ_GOTO(r, data, error);
    }

    /* make containers default if should be */
    if (node->schema->nodetype == LYS_CONTAINER) {
        LY_TREE_FOR(node->child, iter) {
            if (!iter->dflt) {
                break;
            }
        }

        if (!iter) {
            node->dflt = 1;
        }
    }

stop_subtree:
    /* end the subtree */
    lyb_read_stop_subtree(lybs);

    return ret;

error:
    lyd_free(node);
    if (*first_sibling == node) {
        *first_sibling = NULL;
    }
    return -1;
}

static int
lyb_parse_data_models(struct ly_ctx *ctx, const char *data, struct lyb_state *lybs)
{
    int i, r, ret = 0;

    /* read model count */
    ret += (r = lyb_read(data, (uint8_t *)&lybs->mod_count, 2, lybs));
    LYB_HAVE_READ_RETURN(r, data, -1);

    lybs->models = malloc(lybs->mod_count * sizeof *lybs->models);
    LY_CHECK_ERR_RETURN(!lybs->models, LOGMEM(NULL), -1);

    /* read modules */
    for (i = 0; i < lybs->mod_count; ++i) {
        ret += (r = lyb_parse_model(ctx, data, &lybs->models[i], lybs));
        LYB_HAVE_READ_RETURN(r, data, -1);
    }

    return ret;
}

static int
lyb_parse_header(const char *data, struct lyb_state *lybs)
{
    int ret = 0;
    uint8_t byte = 0;

    /* TODO version, any flags? */
    ret += lyb_read(data, (uint8_t *)&byte, sizeof byte, lybs);

    return ret;
}

struct lyd_node *
lyd_parse_lyb(struct ly_ctx *ctx, const char *data, int options, const struct lyd_node *data_tree,
              const char *yang_data_name, int *parsed)
{
    int r, ret = 0;
    struct lyd_node *node = NULL, *next, *act_notif = NULL;
    struct unres_data *unres = NULL;
    struct lyb_state lybs;

    if (!ctx || !data) {
        LOGARG;
        return NULL;
    }

    lybs.written = malloc(LYB_STATE_STEP * sizeof *lybs.written);
    lybs.position = malloc(LYB_STATE_STEP * sizeof *lybs.position);
    LY_CHECK_ERR_GOTO(!lybs.written || !lybs.position, LOGMEM(ctx), finish);
    lybs.used = 0;
    lybs.size = LYB_STATE_STEP;
    lybs.models = NULL;
    lybs.mod_count = 0;

    unres = calloc(1, sizeof *unres);
    LY_CHECK_ERR_GOTO(!unres, LOGMEM(ctx), finish);

    /* read header */
    ret += (r = lyb_parse_header(data, &lybs));
    LYB_HAVE_READ_GOTO(r, data, finish);

    /* read used models */
    ret += (r = lyb_parse_data_models(ctx, data, &lybs));
    LYB_HAVE_READ_GOTO(r, data, finish);

    /* read subtree(s) */
    while (data[0]) {
        ret += (r = lyb_parse_subtree(ctx, data, NULL, &node, yang_data_name, options, unres, &lybs));
        if (r < 0) {
            lyd_free_withsiblings(node);
            node = NULL;
            goto finish;
        }
        data += r;
    }

    /* read the last zero, parsing finished */
    ++ret;
    r = ret;

    if (options & LYD_OPT_DATA_ADD_YANGLIB) {
        if (lyd_merge(node, ly_ctx_info(ctx), LYD_OPT_DESTRUCT | LYD_OPT_EXPLICIT)) {
            LOGERR(ctx, LY_EINT, "Adding ietf-yang-library data failed.");
            lyd_free_withsiblings(node);
            node = NULL;
            goto finish;
        }
    }

    /* resolve any unresolved instance-identifiers */
    if (unres->count) {
        if (options & (LYD_OPT_RPC | LYD_OPT_RPCREPLY | LYD_OPT_NOTIF)) {
            LY_TREE_DFS_BEGIN(node, next, act_notif) {
                if (act_notif->schema->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
                    break;
                }
                LY_TREE_DFS_END(node, next, act_notif);
            }
        }
        if (lyd_defaults_add_unres(&node, options, ctx, data_tree, act_notif, unres, 0)) {
            lyd_free_withsiblings(node);
            node = NULL;
            goto finish;
        }
    }

finish:
    free(lybs.written);
    free(lybs.position);
    free(lybs.models);
    if (unres) {
        free(unres->node);
        free(unres->type);
        free(unres);
    }

    if (parsed) {
        *parsed = r;
    }
    return node;
}
