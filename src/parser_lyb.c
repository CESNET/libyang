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
#include "log.h"
#include "parser_data.h"
#include "parser_internal.h"
#include "set.h"
#include "tree.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "validation.h"

/**
 * @brief Read YANG data from LYB input. Metadata are handled transparently and not returned.
 *
 * @param[in] buf Destination buffer.
 * @param[in] count Number of bytes to read.
 * @param[in] lybctx LYB context.
 */
static void
lyb_read(uint8_t *buf, size_t count, struct lyd_lyb_ctx *lybctx)
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
lyb_read_number(void *num, size_t num_size, size_t bytes, struct lyd_lyb_ctx *lybctx)
{
    uint64_t buf = 0;

    lyb_read((uint8_t *)&buf, bytes, lybctx);

    /* correct byte order */
    buf = le64toh(buf);

    switch (num_size) {
    case 1:
        *((uint8_t *)num) = buf;
        break;
    case 2:
        *((uint16_t *)num) = buf;
        break;
    case 4:
        *((uint32_t *)num) = buf;
        break;
    case 8:
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
lyb_read_string(char **str, int with_length, struct lyd_lyb_ctx *lybctx)
{
    int next_chunk = 0;
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
lyb_read_stop_subtree(struct lyd_lyb_ctx *lybctx)
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
lyb_read_start_subtree(struct lyd_lyb_ctx *lybctx)
{
    uint8_t meta_buf[LYB_META_BYTES];
    LY_ARRAY_COUNT_TYPE u;

    if (!lybctx->subtrees) {
        assert(lybctx->subtree_size == 0);
        u = 0;
    } else {
        u = LY_ARRAY_COUNT(lybctx->subtrees);
    }
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
lyb_parse_model(struct lyd_lyb_ctx *lybctx, const struct lys_module **mod)
{
    LY_ERR ret = LY_SUCCESS;
    char *mod_name = NULL, mod_rev[11];
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
        sprintf(mod_rev, "%04u-%02u-%02u", ((rev & 0xFE00) >> 9) + 2000, (rev & 0x01E0) >> 5, rev & 0x001Fu);
        *mod = ly_ctx_get_module(lybctx->ctx, mod_name, mod_rev);
        if ((lybctx->parse_options & LYD_PARSE_LYB_MOD_UPDATE) && !(*mod)) {
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
        if (lybctx->parse_options & LYD_PARSE_STRICT) {
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
 * @param[in] sparent Schema parent node.
 * @param[out] meta Parsed metadata.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_metadata(struct lyd_lyb_ctx *lybctx, const struct lysc_node *sparent, struct lyd_meta **meta)
{
    LY_ERR ret = LY_SUCCESS;
    int dynamic;
    uint8_t i, count = 0;
    char *meta_name = NULL, *meta_value;
    const struct lys_module *mod;

    /* read number of attributes stored */
    lyb_read(&count, 1, lybctx);

    /* read attributes */
    for (i = 0; i < count; ++i) {
        ret = lyb_read_start_subtree(lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* find model */
        ret = lyb_parse_model(lybctx, &mod);
        LY_CHECK_GOTO(ret, cleanup);

        if (!mod) {
            /* skip it */
            do {
                lyb_read(NULL, LYB_LAST_SUBTREE(lybctx).written, lybctx);
            } while (LYB_LAST_SUBTREE(lybctx).written);
            goto stop_subtree;
        }

        /* meta name */
        ret = lyb_read_string(&meta_name, 1, lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* meta value */
        ret = lyb_read_string(&meta_value, 0, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        dynamic = 1;

        /* create metadata */
        ret = lyd_create_meta(NULL, meta, mod, meta_name, strlen(meta_name), meta_value, strlen(meta_value), &dynamic,
                              lydjson_resolve_prefix, NULL, LYD_JSON, sparent);

        /* free strings */
        free(meta_name);
        meta_name = NULL;
        if (dynamic) {
            free(meta_value);
            dynamic = 0;
        }

        if (ret == LY_EINCOMPLETE) {
            ly_set_add(&lybctx->unres_meta_type, *meta, LY_SET_OPT_USEASLIST);
        } else if (ret) {
            goto cleanup;
        }

stop_subtree:
        ret = lyb_read_stop_subtree(lybctx);
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
 * @brief Parse opaque prefixes structure.
 *
 * @param[in] lybctx LYB context.
 * @param[out] prefs Parsed prefixes.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_opaq_prefixes(struct lyd_lyb_ctx *lybctx, struct ly_prefix **prefs)
{
    LY_ERR ret = LY_SUCCESS;
    uint8_t count, i;
    char *str;

    /* read count */
    lyb_read(&count, 1, lybctx);
    if (!count) {
        return LY_SUCCESS;
    }

    LY_ARRAY_CREATE_RET(lybctx->ctx, *prefs, count, LY_EMEM);
    for (i = 0; i < count; ++i) {
        LY_ARRAY_INCREMENT(*prefs);

        /* prefix */
        ret = lyb_read_string(&str, 1, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        (*prefs)[i].pref = lydict_insert_zc(lybctx->ctx, str);

        /* namespace */
        ret = lyb_read_string(&str, 1, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        (*prefs)[i].ns = lydict_insert_zc(lybctx->ctx, str);
    }

cleanup:
    if (ret) {
        ly_free_val_prefs(lybctx->ctx, *prefs);
        *prefs = NULL;
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
lyb_parse_attributes(struct lyd_lyb_ctx *lybctx, struct ly_attr **attr)
{
    LY_ERR ret = LY_SUCCESS;
    uint8_t count, i;
    struct ly_attr *attr2;
    char *prefix = NULL, *ns = NULL, *name = NULL, *value = NULL;
    int dynamic = 0;
    LYD_FORMAT format = 0;
    struct ly_prefix *val_prefs = NULL;

    /* read count */
    lyb_read(&count, 1, lybctx);

    /* read attributes */
    for (i = 0; i < count; ++i) {
        ret = lyb_read_start_subtree(lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* prefix, may be emtpy */
        ret = lyb_read_string(&prefix, 1, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        if (!prefix[0]) {
            free(prefix);
            prefix = NULL;
        }

        /* namespace, may be empty */
        ret = lyb_read_string(&ns, 1, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        if (!ns[0]) {
            free(ns);
            ns = NULL;
        }

        /* name */
        ret = lyb_read_string(&name, 1, lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* value prefixes */
        ret = lyb_parse_opaq_prefixes(lybctx, &val_prefs);
        LY_CHECK_GOTO(ret, cleanup);

        /* format */
        lyb_read((uint8_t *)&format, 1, lybctx);

        /* value */
        ret = lyb_read_string(&value, 0, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        dynamic = 1;

        /* attr2 is always changed to the created attribute */
        ret = ly_create_attr(NULL, &attr2, lybctx->ctx, name, strlen(name), value, strlen(value), &dynamic, format,
                             val_prefs, prefix, prefix ? strlen(prefix) : 0, ns);
        LY_CHECK_GOTO(ret, cleanup);

        free(prefix);
        prefix = NULL;
        free(ns);
        ns = NULL;
        free(name);
        name = NULL;
        val_prefs = NULL;
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
    free(ns);
    free(name);
    if (dynamic) {
        free(value);
    }
    ly_free_val_prefs(lybctx->ctx, val_prefs);
    if (ret) {
        ly_free_attr_siblings(lybctx->ctx, *attr);
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
 * @brief Check that a schema node is suitable based on options.
 *
 * @param[in] lybctx LYB context.
 * @param[in] snode Schema node to check.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_check_schema(struct lyd_lyb_ctx *lybctx, const struct lysc_node *snode)
{
    LY_ERR ret = LY_SUCCESS;

    if ((lybctx->parse_options & LYD_PARSE_NO_STATE) && (snode->flags & LYS_CONFIG_R)) {
        LOGVAL(lybctx->ctx, LY_VLOG_LYSC, snode, LY_VCODE_INNODE, "state", snode->name);
        return LY_EVALID;
    }

    if (snode->nodetype & (LYS_RPC | LYS_ACTION)) {
        if (lybctx->int_opts & LYD_INTOPT_RPC) {
            if (lybctx->op_ntf) {
                LOGVAL(lybctx->ctx, LY_VLOG_LYSC, snode, LYVE_DATA, "Unexpected %s element \"%s\", %s \"%s\" already parsed.",
                       lys_nodetype2str(snode->nodetype), snode->name,
                       lys_nodetype2str(lybctx->op_ntf->schema->nodetype), lybctx->op_ntf->schema->name);
                return LY_EVALID;
            }
        } else {
            LOGVAL(lybctx->ctx, LY_VLOG_LYSC, snode, LYVE_DATA, "Unexpected %s element \"%s\".",
                   lys_nodetype2str(snode->nodetype), snode->name);
            return LY_EVALID;
        }
    } else if (snode->nodetype == LYS_NOTIF) {
        if (lybctx->int_opts & LYD_INTOPT_NOTIF) {
            if (lybctx->op_ntf) {
                LOGVAL(lybctx->ctx, LY_VLOG_LYSC, snode, LYVE_DATA, "Unexpected %s element \"%s\", %s \"%s\" already parsed.",
                       lys_nodetype2str(snode->nodetype), snode->name,
                       lys_nodetype2str(lybctx->op_ntf->schema->nodetype), lybctx->op_ntf->schema->name);
                return LY_EVALID;
            }
        } else {
            LOGVAL(lybctx->ctx, LY_VLOG_LYSC, snode, LYVE_DATA, "Unexpected %s element \"%s\".",
                   lys_nodetype2str(snode->nodetype), snode->name);
            return LY_EVALID;
        }
    }

    return ret;
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
    int getnext_opts;

    *snode = NULL;
    /* leave if-feature check for validation */
    getnext_opts = LYS_GETNEXT_NOSTATECHECK | (lybctx->int_opts & LYD_INTOPT_REPLY ? LYS_GETNEXT_OUTPUT : 0);

    /* read the first hash */
    lyb_read(&hash[0], sizeof *hash, lybctx);

    if (!hash[0]) {
        /* opaque node */
        return LY_SUCCESS;
    }

    /* based on the first hash read all the other ones, if any */
    for (i = 0; !(hash[0] & (LYB_HASH_COLLISION_ID >> i)); ++i) {
        if (i > LYB_HASH_BITS) {
            LOGINT_RET(lybctx->ctx);
        }
    }

    /* move the first hash on its accurate position */
    hash[i] = hash[0];

    /* read the rest of hashes */
    for (j = i; j; --j) {
        lyb_read(&hash[j - 1], sizeof *hash, lybctx);

        /* correct collision ID */
        assert(hash[j - 1] & (LYB_HASH_COLLISION_ID >> (j - 1)));
        /* preceded with zeros */
        assert(!(hash[j - 1] & (LYB_HASH_MASK << (LYB_HASH_BITS - (j - 1)))));
    }

    /* find our node with matching hashes */
    sibling = NULL;
    while ((sibling = lys_getnext(sibling, sparent, mod ? mod->compiled : NULL, getnext_opts))) {
        /* skip schema nodes from models not present during printing */
        if (lyb_has_schema_model(sibling, lybctx->models)
                && lyb_is_schema_hash_match((struct lysc_node *)sibling, hash, i + 1)) {
            /* match found */
            break;
        }
    }

    if (!sibling && (lybctx->parse_options & LYD_PARSE_STRICT)) {
        if (mod) {
            LOGVAL(lybctx->ctx, LY_VLOG_NONE, NULL, LYVE_REFERENCE, "Failed to find matching hash for a top-level node"
                   " from \"%s\".", mod->name);
        } else {
            LOGVAL(lybctx->ctx, LY_VLOG_LYSC, sparent, LYVE_REFERENCE, "Failed to find matching hash for a child node"
                   " of \"%s\".", sparent->name);
        }
        return LY_EVALID;
    } else if (sibling && (ret = lyb_parse_check_schema(lybctx, sibling))) {
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
lyb_skip_subtree(struct lyd_lyb_ctx *lybctx)
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
    struct ly_attr *attr = NULL, *a;
    struct ly_prefix *val_prefs = NULL;
    LYD_ANYDATA_VALUETYPE value_type;
    char *value = NULL, *name = NULL, *prefix = NULL, *ns = NULL;
    int dynamic = 0;
    LYD_FORMAT format = 0;
    int prev_lo;

    /* register a new subtree */
    LY_CHECK_GOTO(ret = lyb_read_start_subtree(lybctx), cleanup);

    if (!parent) {
        /* top-level, read module name */
        ret = lyb_parse_model(lybctx, &mod);
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
        lyb_skip_subtree(lybctx);
        goto stop_subtree;
     }

    /* create metadata/attributes */
    if (snode) {
        ret = lyb_parse_metadata(lybctx, snode, &meta);
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        ret = lyb_parse_attributes(lybctx, &attr);
        LY_CHECK_GOTO(ret, cleanup);
    }

    if (!snode) {
        /* parse prefix */
        ret = lyb_read_string(&prefix, 1, lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* parse namespace */
        ret = lyb_read_string(&ns, 1, lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* parse name */
        ret = lyb_read_string(&name, 1, lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* parse value prefixes */
        ret = lyb_parse_opaq_prefixes(lybctx, &val_prefs);
        LY_CHECK_GOTO(ret, cleanup);

        /* parse format */
        lyb_read((uint8_t *)&format, 1, lybctx);

        /* parse value */
        ret = lyb_read_string(&value, 0, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        dynamic = 1;

        /* create node */
        ret = lyd_create_opaq(lybctx->ctx, name, strlen(name), value, strlen(value), &dynamic, format, val_prefs, prefix,
                              strlen(prefix), ns, &node);
        LY_CHECK_GOTO(ret, cleanup);

        /* process children */
        while (LYB_LAST_SUBTREE(lybctx).written) {
            ret = lyb_parse_subtree_r(lybctx, (struct lyd_node_inner *)node, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }
    } else if (snode->nodetype & LYD_NODE_TERM) {
        /* parse value */
        ret = lyb_read_string(&value, 0, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        dynamic = 1;

        /* create node */
        ret = lyd_create_term(snode, value, strlen(value), &dynamic, lydjson_resolve_prefix, NULL, LYD_JSON, &node);
        if (dynamic) {
            free(value);
            dynamic = 0;
        }
        value = NULL;
        if (ret == LY_EINCOMPLETE) {
            if (!(lybctx->parse_options & LYD_PARSE_ONLY)) {
                ly_set_add(&lybctx->unres_node_type, node, LY_SET_OPT_USEASLIST);
            }
            ret = LY_SUCCESS;
        } else if (ret) {
            goto cleanup;
        }
    } else if (snode->nodetype & LYD_NODE_INNER) {
        /* create node */
        ret = lyd_create_inner(snode, &node);
        LY_CHECK_GOTO(ret, cleanup);

        /* process children */
        while (LYB_LAST_SUBTREE(lybctx).written) {
            ret = lyb_parse_subtree_r(lybctx, (struct lyd_node_inner *)node, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }

        if (!(lybctx->parse_options & LYD_PARSE_ONLY)) {
            /* new node validation, autodelete CANNOT occur, all nodes are new */
            ret = lyd_validate_new(lyd_node_children_p(node), snode, NULL, NULL);
            LY_CHECK_GOTO(ret, cleanup);

            /* add any missing default children */
            ret = lyd_validate_defaults_r(node, lyd_node_children_p(node), NULL, NULL, &lybctx->unres_node_type,
                                          &lybctx->when_check, lybctx->validate_options, NULL);
            LY_CHECK_GOTO(ret, cleanup);
        }

        if (snode->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
            /* rememeber the RPC/action/notification */
            lybctx->op_ntf = node;
        }
    } else if (snode->nodetype & LYD_NODE_ANY) {
        /* parse value type */
        lyb_read((uint8_t *)&value_type, sizeof value_type, lybctx);
        if (value_type == LYD_ANYDATA_DATATREE) {
            /* invalid situation */
            LOGINT(lybctx->ctx);
            goto cleanup;
        }

        /* read anydata content */
        ret = lyb_read_string(&value, 0, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        dynamic = 1;

        if (value_type == LYD_ANYDATA_LYB) {
            /* turn logging off */
            prev_lo = ly_log_options(0);

            /* try to parse LYB into a data tree */
            if (lyd_parse_data_mem((struct ly_ctx *)lybctx->ctx, value, LYD_LYB, LYD_PARSE_ONLY | LYD_PARSE_OPAQ | LYD_PARSE_STRICT, 0, &tree) == LY_SUCCESS) {
                /* successfully parsed */
                free(value);
                value = (char *)tree;
                value_type = LYD_ANYDATA_DATATREE;
            }

            /* turn logging on again */
            ly_log_options(prev_lo);
        }

        /* create node */
        ret = lyd_create_any(snode, value, value_type, &node);
        LY_CHECK_GOTO(ret, cleanup);

        dynamic = 0;
        value = NULL;
    }
    assert(node);

    /* add/correct flags */
    if (snode) {
        lyd_parse_set_data_flags(node, &lybctx->when_check, &meta, lybctx->parse_options);
    }

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
    ret = lyb_read_stop_subtree(lybctx);
    LY_CHECK_GOTO(ret, cleanup);

cleanup:
    free(prefix);
    free(ns);
    free(name);
    if (dynamic) {
        free(value);
    }
    ly_free_val_prefs(lybctx->ctx, val_prefs);

    lyd_free_meta_siblings(meta);
    ly_free_attr_siblings(lybctx->ctx, attr);
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
lyb_parse_data_models(struct lyd_lyb_ctx *lybctx)
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
            ret = lyb_parse_model(lybctx, &lybctx->models[u]);
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
lyb_parse_magic_number(struct lyd_lyb_ctx *lybctx)
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
lyb_parse_header(struct lyd_lyb_ctx *lybctx)
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

LY_ERR
lyd_parse_lyb_data(const struct ly_ctx *ctx, struct ly_in *in, int parse_options, int validate_options,
                   struct lyd_node **tree)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_lyb_ctx lybctx = {0};

    assert(!(parse_options & ~LYD_PARSE_OPTS_MASK));
    assert(!(validate_options & ~LYD_VALIDATE_OPTS_MASK));

    *tree = NULL;

    lybctx.in = in;
    lybctx.ctx = ctx;
    lybctx.parse_options = parse_options;
    lybctx.validate_options = validate_options;

    /* read magic number */
    ret = lyb_parse_magic_number(&lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read header */
    ret = lyb_parse_header(&lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read used models */
    ret = lyb_parse_data_models(&lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read subtree(s) */
    while (lybctx.in->current[0]) {
        ret = lyb_parse_subtree_r(&lybctx, NULL, tree);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* read the last zero, parsing finished */
    ly_in_skip(lybctx.in, 1);

    /* TODO validation */

cleanup:
    LY_ARRAY_FREE(lybctx.subtrees);
    LY_ARRAY_FREE(lybctx.models);
    ly_set_erase(&lybctx.unres_node_type, NULL);
    ly_set_erase(&lybctx.unres_meta_type, NULL);
    ly_set_erase(&lybctx.when_check, NULL);

    if (ret) {
        lyd_free_all(*tree);
        *tree = NULL;
    }
    return ret;
}

LY_ERR
lyd_parse_lyb_rpc(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree, struct lyd_node **op)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_lyb_ctx lybctx = {0};

    lybctx.in = in;
    lybctx.ctx = ctx;
    lybctx.parse_options = LYD_PARSE_ONLY | LYD_PARSE_STRICT;
    lybctx.int_opts = LYD_INTOPT_RPC;

    *tree = NULL;
    if (op) {
        *op = NULL;
    }

    /* read magic number */
    ret = lyb_parse_magic_number(&lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read header */
    ret = lyb_parse_header(&lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read used models */
    ret = lyb_parse_data_models(&lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read subtree(s) */
    while (lybctx.in->current[0]) {
        ret = lyb_parse_subtree_r(&lybctx, NULL, tree);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* read the last zero, parsing finished */
    ly_in_skip(lybctx.in, 1);

    /* make sure we have parsed some operation */
    if (!lybctx.op_ntf) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_DATA, "Missing the \"rpc\"/\"action\" node.");
        ret = LY_EVALID;
        goto cleanup;
    }

    if (op) {
        *op = lybctx.op_ntf;
    }
    assert(*tree);

cleanup:
    LY_ARRAY_FREE(lybctx.subtrees);
    LY_ARRAY_FREE(lybctx.models);
    assert(!lybctx.unres_node_type.count && !lybctx.unres_meta_type.count && !lybctx.when_check.count);

    if (ret) {
        lyd_free_all(*tree);
        *tree = NULL;
    }
    return ret;
}

LY_ERR
lyd_parse_lyb_notif(const struct ly_ctx *ctx, struct ly_in *in, struct lyd_node **tree, struct lyd_node **ntf)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_lyb_ctx lybctx = {0};

    lybctx.in = in;
    lybctx.ctx = ctx;
    lybctx.parse_options = LYD_PARSE_ONLY | LYD_PARSE_STRICT;
    lybctx.int_opts = LYD_INTOPT_NOTIF;

    *tree = NULL;
    if (ntf) {
        *ntf = NULL;
    }

    /* read magic number */
    ret = lyb_parse_magic_number(&lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read header */
    ret = lyb_parse_header(&lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read used models */
    ret = lyb_parse_data_models(&lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read subtree(s) */
    while (lybctx.in->current[0]) {
        ret = lyb_parse_subtree_r(&lybctx, NULL, tree);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* read the last zero, parsing finished */
    ly_in_skip(lybctx.in, 1);

    /* make sure we have parsed some notification */
    if (!lybctx.op_ntf) {
        LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_DATA, "Missing the \"notification\" node.");
        ret = LY_EVALID;
        goto cleanup;
    }

    if (ntf) {
        *ntf = lybctx.op_ntf;
    }
    assert(*tree);

cleanup:
    LY_ARRAY_FREE(lybctx.subtrees);
    LY_ARRAY_FREE(lybctx.models);
    assert(!lybctx.unres_node_type.count && !lybctx.unres_meta_type.count && !lybctx.when_check.count);

    if (ret) {
        lyd_free_all(*tree);
        *tree = NULL;
    }
    return ret;
}

LY_ERR
lyd_parse_lyb_reply(const struct lyd_node *request, struct ly_in *in, struct lyd_node **tree, struct lyd_node **op)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_lyb_ctx lybctx = {0};
    struct lyd_node *iter, *req_op, *rep_op = NULL;

    lybctx.in = in;
    lybctx.ctx = LYD_NODE_CTX(request);
    lybctx.parse_options = LYD_PARSE_ONLY | LYD_PARSE_STRICT;
    lybctx.int_opts = LYD_INTOPT_REPLY;

    *tree = NULL;
    if (op) {
        *op = NULL;
    }

    /* find request OP */
    LYD_TREE_DFS_BEGIN((struct lyd_node *)request, iter, req_op) {
        if (req_op->schema->nodetype & (LYS_RPC | LYS_ACTION)) {
            break;
        }
        LYD_TREE_DFS_END(request, iter, req_op);
    }
    if (!(req_op->schema->nodetype & (LYS_RPC | LYS_ACTION))) {
        LOGERR(LYD_NODE_CTX(request), LY_EINVAL, "No RPC/action in the request found.");
        ret = LY_EINVAL;
        goto cleanup;
    }

    /* duplicate request OP with parents */
    ret = lyd_dup_single(req_op, NULL, LYD_DUP_WITH_PARENTS, &rep_op);
    LY_CHECK_GOTO(ret, cleanup);

    /* read magic number */
    ret = lyb_parse_magic_number(&lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read header */
    ret = lyb_parse_header(&lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read used models */
    ret = lyb_parse_data_models(&lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read subtree(s) */
    while (lybctx.in->current[0]) {
        ret = lyb_parse_subtree_r(&lybctx, (struct lyd_node_inner *)rep_op, NULL);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* read the last zero, parsing finished */
    ly_in_skip(lybctx.in, 1);

    if (op) {
        *op = rep_op;
    }
    for (iter = rep_op; iter->parent; iter = (struct lyd_node *)iter->parent);
    *tree = iter;
    rep_op = NULL;

cleanup:
    lyd_free_all(rep_op);
    LY_ARRAY_FREE(lybctx.subtrees);
    LY_ARRAY_FREE(lybctx.models);
    assert(!lybctx.unres_node_type.count && !lybctx.unres_meta_type.count && !lybctx.when_check.count);

    if (ret) {
        lyd_free_all(*tree);
        *tree = NULL;
    }
    return ret;
}

API int
lyd_lyb_data_length(const char *data)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_lyb_ctx lybctx = {0};
    int count, i;
    size_t len;
    uint8_t buf[LYB_SIZE_MAX];

    if (!data) {
        return -1;
    }

    ret = ly_in_new_memory(data, &lybctx.in);
    LY_CHECK_GOTO(ret, cleanup);

    /* read magic number */
    ret = lyb_parse_magic_number(&lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read header */
    ret = lyb_parse_header(&lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* read model count */
    lyb_read_number(&count, sizeof count, 2, &lybctx);

    /* read all models */
    for (i = 0; i < count; ++i) {
        /* module name length */
        len = 0;
        lyb_read_number(&len, sizeof len, 2, &lybctx);

        /* model name */
        lyb_read(buf, len, &lybctx);

        /* revision */
        lyb_read(buf, 2, &lybctx);
    }

    while (lybctx.in->current[0]) {
        /* register a new subtree */
        ret = lyb_read_start_subtree(&lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* skip it */
        lyb_skip_subtree(&lybctx);

        /* subtree finished */
        ret = lyb_read_stop_subtree(&lybctx);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* read the last zero, parsing finished */
    ly_in_skip(lybctx.in, 1);

cleanup:
    count = lybctx.in->current - lybctx.in->start;

    ly_in_free(lybctx.in, 0);
    LY_ARRAY_FREE(lybctx.subtrees);
    return ret ? -1 : count;
}
