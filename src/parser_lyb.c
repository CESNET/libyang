/**
 * @file parser_lyb.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief LYB data parser for libyang
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "lyb.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "hash_table.h"
#include "in.h"
#include "in_internal.h"
#include "log.h"
#include "parser_data.h"
#include "parser_internal.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "validation.h"
#include "xml.h"

void
lylyb_ctx_free(struct lylyb_ctx *ctx)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FREE(ctx->subtrees);
    LY_ARRAY_FREE(ctx->models);

    LY_ARRAY_FOR(ctx->sib_hts, u) {
        lyht_free(ctx->sib_hts[u].ht);
    }
    LY_ARRAY_FREE(ctx->sib_hts);

    free(ctx);
}

void
lyd_lyb_ctx_free(struct lyd_ctx *lydctx)
{
    struct lyd_lyb_ctx *ctx = (struct lyd_lyb_ctx *)lydctx;

    lyd_ctx_free(lydctx);
    lylyb_ctx_free(ctx->lybctx);
    free(ctx);
}

/**
 * @brief Read YANG data from LYB input. Metadata are handled transparently and not returned.
 *
 * @param[in] buf Destination buffer.
 * @param[in] count Number of bytes to read.
 * @param[in] lybctx LYB context.
 */
static void
lyb_read(uint8_t *buf, size_t count, struct lylyb_ctx *lybctx)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lyd_lyb_subtree *empty;
    size_t to_read;
    uint8_t meta_buf[LYB_META_BYTES];

    assert(lybctx);

    while (1) {
        /* check for fully-read (empty) data chunks */
        to_read = count;
        empty = NULL;
        LY_ARRAY_FOR(lybctx->subtrees, u) {
            /* we want the innermost chunks resolved first, so replace previous empty chunks,
             * also ignore chunks that are completely finished, there is nothing for us to do */
            if ((lybctx->subtrees[u].written <= to_read) && lybctx->subtrees[u].position) {
                /* empty chunk, do not read more */
                to_read = lybctx->subtrees[u].written;
                empty = &lybctx->subtrees[u];
            }
        }

        if (!empty && !count) {
            break;
        }

        /* we are actually reading some data, not just finishing another chunk */
        if (to_read) {
            if (buf) {
                ly_in_read(lybctx->in, buf, to_read);
            } else {
                ly_in_skip(lybctx->in, to_read);
            }

            LY_ARRAY_FOR(lybctx->subtrees, u) {
                /* decrease all written counters */
                lybctx->subtrees[u].written -= to_read;
                assert(lybctx->subtrees[u].written <= LYB_SIZE_MAX);
            }
            /* decrease count/buf */
            count -= to_read;
            if (buf) {
                buf += to_read;
            }
        }

        if (empty) {
            /* read the next chunk meta information */
            ly_in_read(lybctx->in, meta_buf, LYB_META_BYTES);
            empty->written = meta_buf[0];
            empty->inner_chunks = meta_buf[1];

            /* remember whether there is a following chunk or not */
            empty->position = (empty->written == LYB_SIZE_MAX ? 1 : 0);
        }
    }
}

/**
 * @brief Read a number.
 *
 * @param[in] num Destination buffer.
 * @param[in] num_size Size of @p num.
 * @param[in] bytes Number of bytes to read.
 * @param[in] lybctx LYB context.
 */
static void
lyb_read_number(void *num, size_t num_size, size_t bytes, struct lylyb_ctx *lybctx)
{
    uint64_t buf = 0;

    lyb_read((uint8_t *)&buf, bytes, lybctx);

    /* correct byte order */
    buf = le64toh(buf);

    switch (num_size) {
    case sizeof(uint8_t):
        *((uint8_t *)num) = buf;
        break;
    case sizeof(uint16_t):
        *((uint16_t *)num) = buf;
        break;
    case sizeof(uint32_t):
        *((uint32_t *)num) = buf;
        break;
    case sizeof(uint64_t):
        *((uint64_t *)num) = buf;
        break;
    default:
        LOGINT(lybctx->ctx);
    }
}

/**
 * @brief Read a string.
 *
 * @param[in] str Destination buffer, is allocated.
 * @param[in] with_length Whether the string is preceded with its length or it ends at the end of this subtree.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_read_string(char **str, ly_bool with_length, struct lylyb_ctx *lybctx)
{
    ly_bool next_chunk = 0;
    size_t len = 0, cur_len;

    *str = NULL;

    if (with_length) {
        lyb_read_number(&len, sizeof len, 2, lybctx);
    } else {
        /* read until the end of this subtree */
        len = LYB_LAST_SUBTREE(lybctx).written;
        if (LYB_LAST_SUBTREE(lybctx).position) {
            next_chunk = 1;
        }
    }

    *str = malloc((len + 1) * sizeof **str);
    LY_CHECK_ERR_RET(!*str, LOGMEM(lybctx->ctx), LY_EMEM);

    lyb_read((uint8_t *)*str, len, lybctx);

    while (next_chunk) {
        cur_len = LYB_LAST_SUBTREE(lybctx).written;
        if (LYB_LAST_SUBTREE(lybctx).position) {
            next_chunk = 1;
        } else {
            next_chunk = 0;
        }

        *str = ly_realloc(*str, (len + cur_len + 1) * sizeof **str);
        LY_CHECK_ERR_RET(!*str, LOGMEM(lybctx->ctx), LY_EMEM);

        lyb_read(((uint8_t *)*str) + len, cur_len, lybctx);

        len += cur_len;
    }

    ((char *)*str)[len] = '\0';
    return LY_SUCCESS;
}

/**
 * @brief Stop the current subtree - change LYB context state.
 *
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_read_stop_subtree(struct lylyb_ctx *lybctx)
{
    if (LYB_LAST_SUBTREE(lybctx).written) {
        LOGINT_RET(lybctx->ctx);
    }

    LY_ARRAY_DECREMENT(lybctx->subtrees);
    return LY_SUCCESS;
}

/**
 * @brief Start a new subtree - change LYB context state but also read the expected metadata.
 *
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_read_start_subtree(struct lylyb_ctx *lybctx)
{
    uint8_t meta_buf[LYB_META_BYTES];
    LY_ARRAY_COUNT_TYPE u;

    u = LY_ARRAY_COUNT(lybctx->subtrees);
    if (u == lybctx->subtree_size) {
        LY_ARRAY_CREATE_RET(lybctx->ctx, lybctx->subtrees, u + LYB_SUBTREE_STEP, LY_EMEM);
        lybctx->subtree_size = u + LYB_SUBTREE_STEP;
    }

    LY_CHECK_RET(ly_in_read(lybctx->in, meta_buf, LYB_META_BYTES));

    LY_ARRAY_INCREMENT(lybctx->subtrees);
    LYB_LAST_SUBTREE(lybctx).written = meta_buf[0];
    LYB_LAST_SUBTREE(lybctx).inner_chunks = meta_buf[LYB_SIZE_BYTES];
    LYB_LAST_SUBTREE(lybctx).position = (LYB_LAST_SUBTREE(lybctx).written == LYB_SIZE_MAX ? 1 : 0);

    return LY_SUCCESS;
}

/**
 * @brief Parse YANG model info.
 *
 * @param[in] lybctx LYB context.
 * @param[out] mod Parsed module.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_model(struct lylyb_ctx *lybctx, uint32_t parse_options, const struct lys_module **mod)
{
    LY_ERR ret = LY_SUCCESS;
    char *mod_name = NULL, mod_rev[LY_REV_SIZE];
    uint16_t rev;

    /* model name */
    ret = lyb_read_string(&mod_name, 1, lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* revision */
    lyb_read_number(&rev, sizeof rev, 2, lybctx);

    if (!mod_name[0]) {
        /* opaq node, no module */
        *mod = NULL;
        goto cleanup;
    }

    if (rev) {
        sprintf(mod_rev, "%04u-%02u-%02u", ((rev & LYB_REV_YEAR_MASK) >> LYB_REV_YEAR_SHIFT) + LYB_REV_YEAR_OFFSET,
                (rev & LYB_REV_MONTH_MASK) >> LYB_REV_MONTH_SHIFT, rev & LYB_REV_DAY_MASK);
        *mod = ly_ctx_get_module(lybctx->ctx, mod_name, mod_rev);
        if ((parse_options & LYD_PARSE_LYB_MOD_UPDATE) && !(*mod)) {
            /* try to use an updated module */
            *mod = ly_ctx_get_module_implemented(lybctx->ctx, mod_name);
            if (*mod && (!(*mod)->revision || (strcmp((*mod)->revision, mod_rev) < 0))) {
                /* not an implemented module in a newer revision */
                *mod = NULL;
            }
        }
    } else {
        *mod = ly_ctx_get_module_latest(lybctx->ctx, mod_name);
    }
    /* TODO data_clb supported?
    if (lybctx->ctx->data_clb) {
        if (!*mod) {
            *mod = lybctx->ctx->data_clb(lybctx->ctx, mod_name, NULL, 0, lybctx->ctx->data_clb_data);
        } else if (!(*mod)->implemented) {
            *mod = lybctx->ctx->data_clb(lybctx->ctx, mod_name, (*mod)->ns, LY_MODCLB_NOT_IMPLEMENTED, lybctx->ctx->data_clb_data);
        }
    }*/

    if (!*mod || !(*mod)->implemented) {
        if (parse_options & LYD_PARSE_STRICT) {
            if (!*mod) {
                LOGERR(lybctx->ctx, LY_EINVAL, "Invalid context for LYB data parsing, missing module \"%s%s%s\".",
                        mod_name, rev ? "@" : "", rev ? mod_rev : "");
            } else if (!(*mod)->implemented) {
                LOGERR(lybctx->ctx, LY_EINVAL, "Invalid context for LYB data parsing, module \"%s%s%s\" not implemented.",
                        mod_name, rev ? "@" : "", rev ? mod_rev : "");
            }
            ret = LY_EINVAL;
            goto cleanup;
        }

    }

cleanup:
    free(mod_name);
    return ret;
}

/**
 * @brief Parse YANG node metadata.
 *
 * @param[in] lybctx LYB context.
 * @param[out] meta Parsed metadata.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_metadata(struct lyd_lyb_ctx *lybctx, struct lyd_meta **meta)
{
    LY_ERR ret = LY_SUCCESS;
    ly_bool dynamic;
    uint8_t i, count = 0;
    char *meta_name = NULL, *meta_value;
    const struct lys_module *mod;

    /* read number of attributes stored */
    lyb_read(&count, 1, lybctx->lybctx);

    /* read attributes */
    for (i = 0; i < count; ++i) {
        ret = lyb_read_start_subtree(lybctx->lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* find model */
        ret = lyb_parse_model(lybctx->lybctx, lybctx->parse_options, &mod);
        LY_CHECK_GOTO(ret, cleanup);

        if (!mod) {
            /* skip it */
            do {
                lyb_read(NULL, LYB_LAST_SUBTREE(lybctx->lybctx).written, lybctx->lybctx);
            } while (LYB_LAST_SUBTREE(lybctx->lybctx).written);
            goto stop_subtree;
        }

        /* meta name */
        ret = lyb_read_string(&meta_name, 1, lybctx->lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* meta value */
        ret = lyb_read_string(&meta_value, 0, lybctx->lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        dynamic = 1;

        /* create metadata */
        ret = lyd_parser_create_meta((struct lyd_ctx *)lybctx, NULL, meta, mod, meta_name, strlen(meta_name), meta_value,
                ly_strlen(meta_value), &dynamic, LY_PREF_JSON, NULL, LYD_HINT_DATA);

        /* free strings */
        free(meta_name);
        meta_name = NULL;
        if (dynamic) {
            free(meta_value);
            dynamic = 0;
        }

        LY_CHECK_GOTO(ret, cleanup);

stop_subtree:
        ret = lyb_read_stop_subtree(lybctx->lybctx);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    free(meta_name);
    if (ret) {
        lyd_free_meta_siblings(*meta);
        *meta = NULL;
    }
    return ret;
}

/**
 * @brief Parse format-specific prefix data.
 *
 * @param[in] lybctx LYB context.
 * @param[in] format Prefix data format.
 * @param[out] prefix_data Parsed prefix data.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_prefix_data(struct lylyb_ctx *lybctx, LY_PREFIX_FORMAT format, void **prefix_data)
{
    LY_ERR ret = LY_SUCCESS;
    uint8_t count, i;
    struct ly_set *set = NULL;
    struct lyxml_ns *ns = NULL;

    switch (format) {
    case LY_PREF_XML:
        /* read count */
        lyb_read(&count, 1, lybctx);
        if (!count) {
            return LY_SUCCESS;
        }

        /* read all NS elements */
        LY_CHECK_GOTO(ret = ly_set_new(&set), cleanup);

        for (i = 0; i < count; ++i) {
            ns = calloc(1, sizeof *ns);

            /* prefix */
            LY_CHECK_GOTO(ret = lyb_read_string(&ns->prefix, 1, lybctx), cleanup);

            /* namespace */
            LY_CHECK_GOTO(ret = lyb_read_string(&ns->uri, 1, lybctx), cleanup);

            LY_CHECK_GOTO(ret = ly_set_add(set, ns, 1, NULL), cleanup);
            ns = NULL;
        }

        *prefix_data = set;
        break;
    case LY_PREF_JSON:
        /* nothing stored */
        break;
    default:
        LOGINT(lybctx->ctx);
        ret = LY_EINT;
        break;
    }

cleanup:
    if (ret) {
        ly_free_prefix_data(format, set);
        if (ns) {
            free(ns->prefix);
            free(ns->uri);
            free(ns);
        }
    }
    return ret;
}

/**
 * @brief Parse opaque attributes.
 *
 * @param[in] lybctx LYB context.
 * @param[out] attr Parsed attributes.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_attributes(struct lylyb_ctx *lybctx, struct lyd_attr **attr)
{
    LY_ERR ret = LY_SUCCESS;
    uint8_t count, i;
    struct lyd_attr *attr2;
    char *prefix = NULL, *module_name = NULL, *name = NULL, *value = NULL;
    ly_bool dynamic = 0;
    LY_PREFIX_FORMAT format = 0;
    void *val_prefix_data = NULL;

    /* read count */
    lyb_read(&count, 1, lybctx);

    /* read attributes */
    for (i = 0; i < count; ++i) {
        ret = lyb_read_start_subtree(lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* prefix, may be empty */
        ret = lyb_read_string(&prefix, 1, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        if (!prefix[0]) {
            free(prefix);
            prefix = NULL;
        }

        /* namespace, may be empty */
        ret = lyb_read_string(&module_name, 1, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        if (!module_name[0]) {
            free(module_name);
            module_name = NULL;
        }

        /* name */
        ret = lyb_read_string(&name, 1, lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* format */
        lyb_read((uint8_t *)&format, 1, lybctx);

        /* value prefixes */
        ret = lyb_parse_prefix_data(lybctx, format, &val_prefix_data);
        LY_CHECK_GOTO(ret, cleanup);

        /* value */
        ret = lyb_read_string(&value, 0, lybctx);
        LY_CHECK_ERR_GOTO(ret, ly_free_prefix_data(format, val_prefix_data), cleanup);
        dynamic = 1;

        /* attr2 is always changed to the created attribute */
        ret = lyd_create_attr(NULL, &attr2, lybctx->ctx, name, strlen(name), prefix, ly_strlen(prefix), module_name,
                ly_strlen(module_name), value, ly_strlen(value), &dynamic, format, val_prefix_data, 0);
        LY_CHECK_GOTO(ret, cleanup);

        free(prefix);
        prefix = NULL;
        free(module_name);
        module_name = NULL;
        free(name);
        name = NULL;
        assert(!dynamic);
        value = NULL;

        if (!*attr) {
            *attr = attr2;
        }

        ret = lyb_read_stop_subtree(lybctx);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    free(prefix);
    free(module_name);
    free(name);
    if (dynamic) {
        free(value);
    }
    if (ret) {
        lyd_free_attr_siblings(lybctx->ctx, *attr);
        *attr = NULL;
    }
    return ret;
}

/**
 * @brief Check whether a schema node matches a hash(es).
 *
 * @param[in] sibling Schema node to check.
 * @param[in] hash Hash array to check.
 * @param[in] hash_count Number of hashes in @p hash.
 * @return non-zero if matches,
 * @return 0 if not.
 */
static int
lyb_is_schema_hash_match(struct lysc_node *sibling, LYB_HASH *hash, uint8_t hash_count)
{
    LYB_HASH sibling_hash;
    uint8_t i;

    /* compare all the hashes starting from collision ID 0 */
    for (i = 0; i < hash_count; ++i) {
        sibling_hash = lyb_hash(sibling, i);
        if (sibling_hash != hash[i]) {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Parse schema node hash.
 *
 * @param[in] lybctx LYB context.
 * @param[in] sparent Schema parent, must be set if @p mod is not.
 * @param[in] mod Module of the top-level node, must be set if @p sparent is not.
 * @param[out] snode Parsed found schema node, may be NULL if opaque.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_schema_hash(struct lyd_lyb_ctx *lybctx, const struct lysc_node *sparent, const struct lys_module *mod,
        const struct lysc_node **snode)
{
    LY_ERR ret;
    uint8_t i, j;
    const struct lysc_node *sibling;
    LYB_HASH hash[LYB_HASH_BITS - 1];
    uint32_t getnext_opts;

    *snode = NULL;
    getnext_opts = lybctx->int_opts & LYD_INTOPT_REPLY ? LYS_GETNEXT_OUTPUT : 0;

    /* read the first hash */
    lyb_read(&hash[0], sizeof *hash, lybctx->lybctx);

    if (!hash[0]) {
        /* opaque node */
        return LY_SUCCESS;
    }

    /* based on the first hash read all the other ones, if any */
    for (i = 0; !(hash[0] & (LYB_HASH_COLLISION_ID >> i)); ++i) {
        if (i > LYB_HASH_BITS) {
            LOGINT_RET(lybctx->lybctx->ctx);
        }
    }

    /* move the first hash on its accurate position */
    hash[i] = hash[0];

    /* read the rest of hashes */
    for (j = i; j; --j) {
        lyb_read(&hash[j - 1], sizeof *hash, lybctx->lybctx);

        /* correct collision ID */
        assert(hash[j - 1] & (LYB_HASH_COLLISION_ID >> (j - 1)));
        /* preceded with zeros */
        assert(!(hash[j - 1] & (LYB_HASH_MASK << (LYB_HASH_BITS - (j - 1)))));
    }

    /* find our node with matching hashes */
    sibling = NULL;
    while ((sibling = lys_getnext(sibling, sparent, mod ? mod->compiled : NULL, getnext_opts))) {
        /* skip schema nodes from models not present during printing */
        if (lyb_has_schema_model(sibling, lybctx->lybctx->models) &&
                lyb_is_schema_hash_match((struct lysc_node *)sibling, hash, i + 1)) {
            /* match found */
            break;
        }
    }

    if (!sibling && (lybctx->parse_options & LYD_PARSE_STRICT)) {
        if (mod) {
            LOGVAL(lybctx->lybctx->ctx, LYVE_REFERENCE, "Failed to find matching hash for a top-level node"
                    " from \"%s\".", mod->name);
        } else {
            LOGVAL(lybctx->lybctx->ctx, LYVE_REFERENCE, "Failed to find matching hash for a child node"
                    " of \"%s\".", sparent->name);
        }
        return LY_EVALID;
    } else if (sibling && (ret = lyd_parser_check_schema((struct lyd_ctx *)lybctx, sibling))) {
        return ret;
    }

    *snode = sibling;
    return LY_SUCCESS;
}

/**
 * @brief Read until the end of the current subtree.
 *
 * @param[in] lybctx LYB context.
 */
static void
lyb_skip_subtree(struct lylyb_ctx *lybctx)
{
    do {
        /* first skip any meta information inside */
        ly_in_skip(lybctx->in, LYB_LAST_SUBTREE(lybctx).inner_chunks * LYB_META_BYTES);

        /* then read data */
        lyb_read(NULL, LYB_LAST_SUBTREE(lybctx).written, lybctx);
    } while (LYB_LAST_SUBTREE(lybctx).written);
}

/**
 * @brief Parse LYB subtree.
 *
 * @param[in] lybctx LYB context.
 * @param[in] parent Data parent of the subtree, must be set if @p first is not.
 * @param[in,out] first First top-level sibling, must be set if @p parent is not.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_subtree_r(struct lyd_lyb_ctx *lybctx, struct lyd_node_inner *parent, struct lyd_node **first)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *node = NULL, *tree;
    const struct lys_module *mod;
    const struct lysc_node *snode = NULL;
    struct lyd_meta *meta = NULL, *m;
    struct lyd_attr *attr = NULL, *a;
    LYD_ANYDATA_VALUETYPE value_type;
    char *value = NULL, *name = NULL, *prefix = NULL, *module_key = NULL;
    ly_bool dynamic = 0;
    LY_PREFIX_FORMAT format = 0;
    void *val_prefix_data = NULL;
    uint32_t prev_lo, flags;
    const struct ly_ctx *ctx = lybctx->lybctx->ctx;

    /* register a new subtree */
    LY_CHECK_GOTO(ret = lyb_read_start_subtree(lybctx->lybctx), cleanup);

    if (!parent) {
        /* top-level, read module name */
        ret = lyb_parse_model(lybctx->lybctx, lybctx->parse_options, &mod);
        LY_CHECK_GOTO(ret, cleanup);

        /* read hash, find the schema node starting from mod */
        ret = lyb_parse_schema_hash(lybctx, NULL, mod, &snode);
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        /* read hash, find the schema node starting from parent schema */
        ret = lyb_parse_schema_hash(lybctx, parent->schema, NULL, &snode);
        LY_CHECK_GOTO(ret, cleanup);
    }

    if (!snode && !(lybctx->parse_options & LYD_PARSE_OPAQ)) {
        /* unknown data, skip them */
        lyb_skip_subtree(lybctx->lybctx);
        goto stop_subtree;
    }

    /* create metadata/attributes */
    if (snode) {
        ret = lyb_parse_metadata(lybctx, &meta);
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        ret = lyb_parse_attributes(lybctx->lybctx, &attr);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* read flags */
    lyb_read_number(&flags, sizeof flags, sizeof flags, lybctx->lybctx);

    if (!snode) {
        /* parse prefix */
        ret = lyb_read_string(&prefix, 1, lybctx->lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* parse module key */
        ret = lyb_read_string(&module_key, 1, lybctx->lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* parse name */
        ret = lyb_read_string(&name, 1, lybctx->lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* parse format */
        lyb_read((uint8_t *)&format, 1, lybctx->lybctx);

        /* parse value prefixes */
        ret = lyb_parse_prefix_data(lybctx->lybctx, format, &val_prefix_data);
        LY_CHECK_GOTO(ret, cleanup);

        /* parse value */
        ret = lyb_read_string(&value, 0, lybctx->lybctx);
        LY_CHECK_ERR_GOTO(ret, ly_free_prefix_data(format, val_prefix_data), cleanup);
        dynamic = 1;

        /* create node */
        ret = lyd_create_opaq(ctx, name, strlen(name), prefix, ly_strlen(prefix), module_key, ly_strlen(module_key),
                value, strlen(value), &dynamic, format, val_prefix_data, 0, &node);
        LY_CHECK_GOTO(ret, cleanup);

        /* process children */
        while (LYB_LAST_SUBTREE(lybctx->lybctx).written) {
            ret = lyb_parse_subtree_r(lybctx, (struct lyd_node_inner *)node, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }
    } else if (snode->nodetype & LYD_NODE_TERM) {
        /* parse value */
        ret = lyb_read_string(&value, 0, lybctx->lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        dynamic = 1;

        /* create node */
        ret = lyd_parser_create_term((struct lyd_ctx *)lybctx, snode, value, ly_strlen(value), &dynamic, LY_PREF_JSON,
                NULL, LYD_HINT_DATA, &node);
        if (dynamic) {
            free(value);
            dynamic = 0;
        }
        value = NULL;
        LY_CHECK_GOTO(ret, cleanup);
    } else if (snode->nodetype & LYD_NODE_INNER) {
        /* create node */
        ret = lyd_create_inner(snode, &node);
        LY_CHECK_GOTO(ret, cleanup);

        /* process children */
        while (LYB_LAST_SUBTREE(lybctx->lybctx).written) {
            ret = lyb_parse_subtree_r(lybctx, (struct lyd_node_inner *)node, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }

        if (!(lybctx->parse_options & LYD_PARSE_ONLY)) {
            /* new node validation, autodelete CANNOT occur, all nodes are new */
            ret = lyd_validate_new(lyd_node_children_p(node), snode, NULL, NULL);
            LY_CHECK_GOTO(ret, cleanup);

            /* add any missing default children */
            ret = lyd_new_implicit_r(node, lyd_node_children_p(node), NULL, NULL, &lybctx->unres_node_type,
                    &lybctx->when_check, (lybctx->validate_options & LYD_VALIDATE_NO_STATE) ?
                    LYD_IMPLICIT_NO_STATE : 0, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }

        if (snode->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
            /* rememeber the RPC/action/notification */
            lybctx->op_node = node;
        }
    } else if (snode->nodetype & LYD_NODE_ANY) {
        /* parse value type */
        lyb_read((uint8_t *)&value_type, sizeof value_type, lybctx->lybctx);
        if (value_type == LYD_ANYDATA_DATATREE) {
            /* invalid situation */
            LOGINT(ctx);
            goto cleanup;
        }

        /* read anydata content */
        ret = lyb_read_string(&value, 0, lybctx->lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        dynamic = 1;

        if (value_type == LYD_ANYDATA_LYB) {
            /* turn logging off */
            prev_lo = ly_log_options(0);

            /* try to parse LYB into a data tree */
            if (lyd_parse_data_mem(ctx, value, LYD_LYB, LYD_PARSE_ONLY | LYD_PARSE_OPAQ | LYD_PARSE_STRICT, 0, &tree) == LY_SUCCESS) {
                /* successfully parsed */
                free(value);
                value = (char *)tree;
                value_type = LYD_ANYDATA_DATATREE;
            }

            /* turn logging on again */
            ly_log_options(prev_lo);
        }

        /* create node */
        ret = lyd_create_any(snode, value, value_type, 1, &node);
        LY_CHECK_GOTO(ret, cleanup);

        dynamic = 0;
        value = NULL;
    }
    assert(node);

    /* set flags */
    node->flags = flags;

    /* add metadata/attributes */
    if (snode) {
        LY_LIST_FOR(meta, m) {
            m->parent = node;
        }
        node->meta = meta;
        meta = NULL;
    } else {
        assert(!node->schema);
        LY_LIST_FOR(attr, a) {
            a->parent = (struct lyd_node_opaq *)node;
        }
        ((struct lyd_node_opaq *)node)->attr = attr;
        attr = NULL;
    }

    /* insert, keep first pointer correct */
    lyd_insert_node((struct lyd_node *)parent, first, node);
    while (!parent && (*first)->prev->next) {
        *first = (*first)->prev;
    }
    node = NULL;

stop_subtree:
    /* end the subtree */
    ret = lyb_read_stop_subtree(lybctx->lybctx);
    LY_CHECK_GOTO(ret, cleanup);

cleanup:
    free(prefix);
    free(module_key);
    free(name);
    if (dynamic) {
        free(value);
    }

    lyd_free_meta_siblings(meta);
    lyd_free_attr_siblings(ctx, attr);
    lyd_free_tree(node);
    return ret;
}

/**
 * @brief Parse used YANG data models.
 *
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_data_models(struct lylyb_ctx *lybctx, uint32_t parse_options)
{
    LY_ERR ret;
    uint32_t count;
    LY_ARRAY_COUNT_TYPE u;

    /* read model count */
    lyb_read_number(&count, sizeof count, 2, lybctx);

    if (count) {
        LY_ARRAY_CREATE_RET(lybctx->ctx, lybctx->models, count, LY_EMEM);

        /* read modules */
        for (u = 0; u < count; ++u) {
            ret = lyb_parse_model(lybctx, parse_options, &lybctx->models[u]);
            LY_CHECK_RET(ret);
            LY_ARRAY_INCREMENT(lybctx->models);
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse LYB magic number.
 *
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_magic_number(struct lylyb_ctx *lybctx)
{
    char magic_byte = 0;

    lyb_read((uint8_t *)&magic_byte, 1, lybctx);
    if (magic_byte != 'l') {
        LOGERR(lybctx->ctx, LY_EINVAL, "Invalid first magic number byte \"0x%02x\".", magic_byte);
        return LY_EINVAL;
    }

    lyb_read((uint8_t *)&magic_byte, 1, lybctx);
    if (magic_byte != 'y') {
        LOGERR(lybctx->ctx, LY_EINVAL, "Invalid second magic number byte \"0x%02x\".", magic_byte);
        return LY_EINVAL;
    }

    lyb_read((uint8_t *)&magic_byte, 1, lybctx);
    if (magic_byte != 'b') {
        LOGERR(lybctx->ctx, LY_EINVAL, "Invalid third magic number byte \"0x%02x\".", magic_byte);
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse LYB header.
 *
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_header(struct lylyb_ctx *lybctx)
{
    uint8_t byte = 0;

    /* version, future flags */
    lyb_read((uint8_t *)&byte, sizeof byte, lybctx);

    if ((byte & LYB_VERSION_MASK) != LYB_VERSION_NUM) {
        LOGERR(lybctx->ctx, LY_EINVAL, "Invalid LYB format version \"0x%02x\", expected \"0x%02x\".",
                byte & LYB_VERSION_MASK, LYB_VERSION_NUM);
        return LY_EINVAL;
    }

    return LY_SUCCESS;
}

/**
 * @param[in] ctx libyang context for logging
 * @param[in] parent Parent node where to connect the parsed data, required for reply where the reply data are connected
 * with the request operation
 * @param[in] in Input structure.
 * @param[in] parse_options Options for parser, see @ref dataparseroptions.
 * @param[in] validate_options Options for the validation phase, see @ref datavalidationoptions.
 * @param[in] data_type Internal data parser flag to distnguish type of the data to parse (RPC/Reply/Notification/regular data].
 * @param[out] tree_p Parsed data tree. Note that NULL can be a valid result.
 * @param[out] op_p Optional pointer to the actual operation. Useful for action and inner notifications.
 * @param[out] lydctx_p Data parser context to finish validation.
 */
static LY_ERR
lyd_parse_lyb_(const struct ly_ctx *ctx, struct lyd_node_inner **parent, struct ly_in *in, uint32_t parse_options,
        uint32_t validate_options, uint32_t data_type, struct lyd_node **tree_p, struct lyd_node **op_p,
        struct lyd_ctx **lydctx_p)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_lyb_ctx *lybctx;
    struct lyd_node *tree = NULL;

    assert(!(parse_options & ~LYD_PARSE_OPTS_MASK));
    assert(!(validate_options & ~LYD_VALIDATE_OPTS_MASK));

    lybctx = calloc(1, sizeof *lybctx);
    LY_CHECK_ERR_RET(!lybctx, LOGMEM(ctx), LY_EMEM);
    lybctx->lybctx = calloc(1, sizeof *lybctx->lybctx);
    LY_CHECK_ERR_GOTO(!lybctx->lybctx, LOGMEM(ctx); ret = LY_EMEM, cleanup);

    lybctx->lybctx->in = in;
    lybctx->lybctx->ctx = ctx;
    lybctx->parse_options = parse_options;
    lybctx->validate_options = validate_options;
    lybctx->int_opts = data_type;
    lybctx->free = lyd_lyb_ctx_free;

    /* read magic number */
    ret = lyb_parse_magic_number(lybctx->lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read header */
    ret = lyb_parse_header(lybctx->lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read used models */
    ret = lyb_parse_data_models(lybctx->lybctx, lybctx->parse_options);
    LY_CHECK_GOTO(ret, cleanup);

    /* read subtree(s) */
    while (lybctx->lybctx->in->current[0]) {
        ret = lyb_parse_subtree_r(lybctx, parent ? *parent : NULL, &tree);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* read the last zero, parsing finished */
    ly_in_skip(lybctx->lybctx->in, 1);

    if (data_type == (LYD_INTOPT_RPC | LYD_INTOPT_REPLY)) {
        /* make sure we have parsed some operation */
        if (!lybctx->op_node) {
            LOGVAL(ctx, LYVE_DATA, "Missing the \"rpc\"/\"action\" node.");
            ret = LY_EVALID;
            goto cleanup;
        }

        if (op_p) {
            *op_p = lybctx->op_node;
        }
        assert(tree);

    } else if (data_type == LYD_INTOPT_NOTIF) {
        /* make sure we have parsed some notification */
        if (!lybctx->op_node) {
            LOGVAL(ctx, LYVE_DATA, "Missing the \"notification\" node.");
            ret = LY_EVALID;
            goto cleanup;
        }

        if (op_p) {
            *op_p = lybctx->op_node;
        }
        assert(tree);
    }

cleanup:
    if (ret) {
        lyd_lyb_ctx_free((struct lyd_ctx *)lybctx);
        lyd_free_all(tree);
    } else {
        if (lydctx_p) {
            *lydctx_p = (struct lyd_ctx *)lybctx;
        } else {
            lyd_lyb_ctx_free((struct lyd_ctx *)lybctx);
        }
        if (tree_p) {
            *tree_p = tree;
        }
    }
    return ret;
}

LY_ERR
lyd_parse_lyb_data(const struct ly_ctx *ctx, struct ly_in *in, uint32_t parse_options, uint32_t validate_options,
        struct lyd_node **tree_p, struct lyd_ctx **lydctx_p)
{
    return lyd_parse_lyb_(ctx, NULL, in, parse_options, validate_options, 0, tree_p, NULL, lydctx_p);
}

LY_ERR
lyd_parse_lyb_rpc(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree_p, struct lyd_node **op_p)
{
    return lyd_parse_lyb_(ctx, NULL, in, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, LYD_INTOPT_RPC, tree_p, op_p, NULL);
}

LY_ERR
lyd_parse_lyb_notif(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree_p, struct lyd_node **ntf_p)
{
    return lyd_parse_lyb_(ctx, NULL, in, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, LYD_INTOPT_NOTIF, tree_p, ntf_p, NULL);
}

LY_ERR
lyd_parse_lyb_reply(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree_p, struct lyd_node **op_p)
{
    return lyd_parse_lyb_(ctx, NULL, in, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, LYD_INTOPT_REPLY, tree_p, op_p, NULL);
}

API int
lyd_lyb_data_length(const char *data)
{
    LY_ERR ret = LY_SUCCESS;
    struct lylyb_ctx *lybctx;
    int count, i;
    size_t len;
    uint8_t buf[LYB_SIZE_MAX];

    if (!data) {
        return -1;
    }

    lybctx = calloc(1, sizeof *lybctx);
    LY_CHECK_ERR_RET(!lybctx, LOGMEM(NULL), LY_EMEM);
    ret = ly_in_new_memory(data, &lybctx->in);
    LY_CHECK_GOTO(ret, cleanup);

    /* read magic number */
    ret = lyb_parse_magic_number(lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read header */
    ret = lyb_parse_header(lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read model count */
    lyb_read_number(&count, sizeof count, 2, lybctx);

    /* read all models */
    for (i = 0; i < count; ++i) {
        /* module name length */
        len = 0;
        lyb_read_number(&len, sizeof len, 2, lybctx);

        /* model name */
        lyb_read(buf, len, lybctx);

        /* revision */
        lyb_read(buf, 2, lybctx);
    }

    while (lybctx->in->current[0]) {
        /* register a new subtree */
        ret = lyb_read_start_subtree(lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* skip it */
        lyb_skip_subtree(lybctx);

        /* subtree finished */
        ret = lyb_read_stop_subtree(lybctx);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* read the last zero, parsing finished */
    ly_in_skip(lybctx->in, 1);

cleanup:
    count = lybctx->in->current - lybctx->in->start;

    ly_in_free(lybctx->in, 0);
    lylyb_ctx_free(lybctx);

    return ret ? -1 : count;
}
