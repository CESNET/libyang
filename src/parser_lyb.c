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
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "validation.h"
#include "xml.h"

static LY_ERR _lyd_parse_lyb(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
        struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, uint32_t int_opts,
        struct ly_set *parsed, struct lyd_ctx **lydctx_p);

static LY_ERR lyb_parse_siblings(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, struct lyd_node **first_p, struct ly_set *parsed);

void
lylyb_ctx_free(struct lylyb_ctx *ctx)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FREE(ctx->siblings);
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
 * @brief Read metadata about siblings.
 *
 * @param[out] sib Structure in which the metadata will be stored.
 * @param[in] lybctx LYB context.
 */
static void
lyb_read_sibling_meta(struct lyd_lyb_sibling *sib, struct lylyb_ctx *lybctx)
{
    uint8_t meta_buf[LYB_META_BYTES];
    uint64_t num = 0;

    ly_in_read(lybctx->in, meta_buf, LYB_META_BYTES);

    memcpy(&num, meta_buf, LYB_SIZE_BYTES);
    sib->written = le64toh(num);
    memcpy(&num, meta_buf + LYB_SIZE_BYTES, LYB_INCHUNK_BYTES);
    sib->inner_chunks = le64toh(num);

    /* remember whether there is a following chunk or not */
    sib->position = (sib->written == LYB_SIZE_MAX ? 1 : 0);
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
    struct lyd_lyb_sibling *empty;
    size_t to_read;

    assert(lybctx);

    while (1) {
        /* check for fully-read (empty) data chunks */
        to_read = count;
        empty = NULL;
        LY_ARRAY_FOR(lybctx->siblings, u) {
            /* we want the innermost chunks resolved first, so replace previous empty chunks,
             * also ignore chunks that are completely finished, there is nothing for us to do */
            if ((lybctx->siblings[u].written <= to_read) && lybctx->siblings[u].position) {
                /* empty chunk, do not read more */
                to_read = lybctx->siblings[u].written;
                empty = &lybctx->siblings[u];
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

            LY_ARRAY_FOR(lybctx->siblings, u) {
                /* decrease all written counters */
                lybctx->siblings[u].written -= to_read;
                assert(lybctx->siblings[u].written <= LYB_SIZE_MAX);
            }
            /* decrease count/buf */
            count -= to_read;
            if (buf) {
                buf += to_read;
            }
        }

        if (empty) {
            /* read the next chunk meta information */
            lyb_read_sibling_meta(empty, lybctx);
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
 * @param[in] with_length Whether the string is preceded with its length or it ends at the end of this "sibling".
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
        /* read until the end of this "sibling" */
        len = LYB_LAST_SIBLING(lybctx).written;
        if (LYB_LAST_SIBLING(lybctx).position) {
            next_chunk = 1;
        }
    }

    *str = malloc((len + 1) * sizeof **str);
    LY_CHECK_ERR_RET(!*str, LOGMEM(lybctx->ctx), LY_EMEM);

    lyb_read((uint8_t *)*str, len, lybctx);

    while (next_chunk) {
        cur_len = LYB_LAST_SIBLING(lybctx).written;
        if (LYB_LAST_SIBLING(lybctx).position) {
            next_chunk = 1;
        } else {
            next_chunk = 0;
        }

        *str = ly_realloc(*str, (len + cur_len + 1) * sizeof **str);
        LY_CHECK_ERR_RET(!*str, LOGMEM(lybctx->ctx), LY_EMEM);

        lyb_read(((uint8_t *)*str) + len, cur_len, lybctx);

        len += cur_len;
    }

    (*str)[len] = '\0';
    return LY_SUCCESS;
}

/**
 * @brief Read value of term node.
 *
 * @param[in] term Compiled term node.
 * @param[out] term_value Set to term node value in dynamically
 * allocated memory. The caller must release it.
 * @param[out] term_value_len Value length in bytes. The zero byte is
 * always included and is not counted.
 * @param[in,out] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_read_term_value(const struct lysc_node_leaf *term, uint8_t **term_value, uint32_t *term_value_len,
        struct lylyb_ctx *lybctx)
{
    uint32_t allocated_size;
    int32_t lyb_data_len;
    struct lysc_type_leafref *type_lf;

    assert(term && term_value && term_value_len && lybctx);

    /*  Find out the size from @ref howtoDataLYB. */
    if (term->type->basetype == LY_TYPE_LEAFREF) {
        /* Leafref itself is ignored, the target is loaded directly. */
        type_lf = (struct lysc_type_leafref *)term->type;
        lyb_data_len = type_lf->realtype->plugin->lyb_data_len;
    } else {
        lyb_data_len = term->type->plugin->lyb_data_len;
    }

    if (lyb_data_len < 0) {
        /* Parse value size. */
        lyb_read_number(term_value_len, sizeof *term_value_len,
                sizeof *term_value_len, lybctx);
    } else {
        /* Data size is fixed. */
        *term_value_len = lyb_data_len;
    }

    /* Allocate memory. */
    allocated_size = *term_value_len + 1;
    *term_value = malloc(allocated_size * sizeof **term_value);
    LY_CHECK_ERR_RET(!*term_value, LOGMEM(lybctx->ctx), LY_EMEM);

    if (*term_value_len > 0) {
        /* Parse value. */
        lyb_read(*term_value, *term_value_len, lybctx);
    }

    /* Add extra zero byte regardless of whether it is string or not. */
    (*term_value)[allocated_size - 1] = 0;

    return LY_SUCCESS;
}

/**
 * @brief Stop the current "siblings" - change LYB context state.
 *
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_read_stop_siblings(struct lylyb_ctx *lybctx)
{
    if (LYB_LAST_SIBLING(lybctx).written) {
        LOGINT_RET(lybctx->ctx);
    }

    LY_ARRAY_DECREMENT(lybctx->siblings);
    return LY_SUCCESS;
}

/**
 * @brief Start a new "siblings" - change LYB context state but also read the expected metadata.
 *
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_read_start_siblings(struct lylyb_ctx *lybctx)
{
    LY_ARRAY_COUNT_TYPE u;

    u = LY_ARRAY_COUNT(lybctx->siblings);
    if (u == lybctx->sibling_size) {
        LY_ARRAY_CREATE_RET(lybctx->ctx, lybctx->siblings, u + LYB_SIBLING_STEP, LY_EMEM);
        lybctx->sibling_size = u + LYB_SIBLING_STEP;
    }

    LY_ARRAY_INCREMENT(lybctx->siblings);
    lyb_read_sibling_meta(&LYB_LAST_SIBLING(lybctx), lybctx);

    return LY_SUCCESS;
}

/**
 * @brief Parse YANG model info.
 *
 * @param[in] lybctx LYB context.
 * @param[in] parse_options Flag with options for parsing.
 * @param[out] model Parsed module.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_model(struct lylyb_ctx *lybctx, uint32_t parse_options, const struct lys_module **model)
{
    LY_ERR ret = LY_SUCCESS;
    const struct lys_module *mod = NULL;
    char *mod_name = NULL, mod_rev[LY_REV_SIZE];
    uint16_t rev, length;

    lyb_read_number(&length, 2, 2, lybctx);

    if (length) {
        mod_name = malloc((length + 1) * sizeof *mod_name);
        LY_CHECK_ERR_RET(!mod_name, LOGMEM(lybctx->ctx), LY_EMEM);
        lyb_read(((uint8_t *)mod_name), length, lybctx);
        mod_name[length] = '\0';
    } else {
        goto cleanup;
    }

    /* revision */
    lyb_read_number(&rev, sizeof rev, 2, lybctx);

    if (rev) {
        sprintf(mod_rev, "%04u-%02u-%02u", ((rev & LYB_REV_YEAR_MASK) >> LYB_REV_YEAR_SHIFT) + LYB_REV_YEAR_OFFSET,
                (rev & LYB_REV_MONTH_MASK) >> LYB_REV_MONTH_SHIFT, rev & LYB_REV_DAY_MASK);
        mod = ly_ctx_get_module(lybctx->ctx, mod_name, mod_rev);
        if ((parse_options & LYD_PARSE_LYB_MOD_UPDATE) && !mod) {
            /* try to use an updated module */
            mod = ly_ctx_get_module_implemented(lybctx->ctx, mod_name);
            if (mod && (!mod->revision || (strcmp(mod->revision, mod_rev) < 0))) {
                /* not an implemented module in a newer revision */
                mod = NULL;
            }
        }
    } else {
        mod = ly_ctx_get_module_latest(lybctx->ctx, mod_name);
    }
    /* TODO data_clb supported?
    if (lybctx->ctx->data_clb) {
        if (!*mod) {
            *mod = lybctx->ctx->data_clb(lybctx->ctx, mod_name, NULL, 0, lybctx->ctx->data_clb_data);
        } else if (!(*mod)->implemented) {
            *mod = lybctx->ctx->data_clb(lybctx->ctx, mod_name, (*mod)->ns, LY_MODCLB_NOT_IMPLEMENTED, lybctx->ctx->data_clb_data);
        }
    }*/

    if (!mod || !mod->implemented) {
        if (parse_options & LYD_PARSE_STRICT) {
            if (!mod) {
                LOGERR(lybctx->ctx, LY_EINVAL, "Invalid context for LYB data parsing, missing module \"%s%s%s\".",
                        mod_name, rev ? "@" : "", rev ? mod_rev : "");
            } else if (!mod->implemented) {
                LOGERR(lybctx->ctx, LY_EINVAL, "Invalid context for LYB data parsing, module \"%s%s%s\" not implemented.",
                        mod_name, rev ? "@" : "", rev ? mod_rev : "");
            }
            ret = LY_EINVAL;
            goto cleanup;
        }

    }

    if (mod) {
        /* fill cached hashes, if not already */
        lyb_cache_module_hash(mod);
    }

cleanup:
    *model = mod;
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
        ret = lyb_read_start_siblings(lybctx->lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* find model */
        ret = lyb_parse_model(lybctx->lybctx, lybctx->parse_opts, &mod);
        LY_CHECK_GOTO(ret, cleanup);

        if (!mod) {
            /* skip it */
            do {
                lyb_read(NULL, LYB_LAST_SIBLING(lybctx->lybctx).written, lybctx->lybctx);
            } while (LYB_LAST_SIBLING(lybctx->lybctx).written);
            goto stop_sibling;
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
                ly_strlen(meta_value), &dynamic, LY_VALUE_JSON, NULL, LYD_HINT_DATA);

        /* free strings */
        free(meta_name);
        meta_name = NULL;
        if (dynamic) {
            free(meta_value);
            dynamic = 0;
        }

        LY_CHECK_GOTO(ret, cleanup);

stop_sibling:
        ret = lyb_read_stop_siblings(lybctx->lybctx);
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
lyb_parse_prefix_data(struct lylyb_ctx *lybctx, LY_VALUE_FORMAT format, void **prefix_data)
{
    LY_ERR ret = LY_SUCCESS;
    uint8_t count, i;
    struct ly_set *set = NULL;
    struct lyxml_ns *ns = NULL;

    switch (format) {
    case LY_VALUE_XML:
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
    case LY_VALUE_JSON:
    case LY_VALUE_LYB:
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
    struct lyd_attr *attr2 = NULL;
    char *prefix = NULL, *module_name = NULL, *name = NULL, *value = NULL;
    ly_bool dynamic = 0;
    LY_VALUE_FORMAT format = 0;
    void *val_prefix_data = NULL;

    /* read count */
    lyb_read(&count, 1, lybctx);

    /* read attributes */
    for (i = 0; i < count; ++i) {
        ret = lyb_read_start_siblings(lybctx);
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
        lyb_read_number(&format, sizeof format, 1, lybctx);

        /* value prefixes */
        ret = lyb_parse_prefix_data(lybctx, format, &val_prefix_data);
        LY_CHECK_GOTO(ret, cleanup);

        /* value */
        ret = lyb_read_string(&value, 0, lybctx);
        LY_CHECK_ERR_GOTO(ret, ly_free_prefix_data(format, val_prefix_data), cleanup);
        dynamic = 1;

        /* attr2 is always changed to the created attribute */
        ret = lyd_create_attr(NULL, &attr2, lybctx->ctx, name, strlen(name), prefix, ly_strlen(prefix), module_name,
                ly_strlen(module_name), value, ly_strlen(value), &dynamic, format, val_prefix_data, LYD_HINT_DATA);
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

        ret = lyb_read_stop_siblings(lybctx);
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
 * @brief Fill @p hash with hash values.
 *
 * @param[in] lybctx LYB context.
 * @param[in,out] hash Pointer to the array in which the hash values are to be written.
 * @param[out] hash_count Number of hashes in @p hash.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_read_hashes(struct lylyb_ctx *lybctx, LYB_HASH *hash, uint8_t *hash_count)
{
    uint8_t i = 0, j;

    /* read the first hash */
    lyb_read(&hash[0], sizeof *hash, lybctx);

    if (!hash[0]) {
        *hash_count = i + 1;
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

    *hash_count = i + 1;

    return LY_SUCCESS;
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
        sibling_hash = lyb_get_hash(sibling, i);
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
    const struct lysc_node *sibling;
    LYB_HASH hash[LYB_HASH_BITS - 1];
    uint32_t getnext_opts;
    uint8_t hash_count;

    *snode = NULL;

    ret = lyb_read_hashes(lybctx->lybctx, hash, &hash_count);
    LY_CHECK_RET(ret);

    if (!hash[0]) {
        /* opaque node */
        return LY_SUCCESS;
    }

    getnext_opts = lybctx->int_opts & LYD_INTOPT_REPLY ? LYS_GETNEXT_OUTPUT : 0;

    /* find our node with matching hashes */
    sibling = NULL;
    while (1) {
        if (!sparent && lybctx->ext) {
            sibling = lys_getnext_ext(sibling, sparent, lybctx->ext, getnext_opts);
        } else {
            sibling = lys_getnext(sibling, sparent, mod ? mod->compiled : NULL, getnext_opts);
        }
        if (!sibling) {
            break;
        }
        /* skip schema nodes from models not present during printing */
        if (lyb_has_schema_model(sibling, lybctx->lybctx->models) &&
                lyb_is_schema_hash_match((struct lysc_node *)sibling, hash, hash_count)) {
            /* match found */
            break;
        }
    }

    if (!sibling && (lybctx->parse_opts & LYD_PARSE_STRICT)) {
        if (lybctx->ext) {
            LOGVAL(lybctx->lybctx->ctx, LYVE_REFERENCE, "Failed to find matching hash for a node from \"%s\" extension instance node.",
                    lybctx->ext->def->name);
        } else if (mod) {
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
 * @brief Read until the end of the current siblings.
 *
 * @param[in] lybctx LYB context.
 */
static void
lyb_skip_siblings(struct lylyb_ctx *lybctx)
{
    do {
        /* first skip any meta information inside */
        ly_in_skip(lybctx->in, LYB_LAST_SIBLING(lybctx).inner_chunks * LYB_META_BYTES);

        /* then read data */
        lyb_read(NULL, LYB_LAST_SIBLING(lybctx).written, lybctx);
    } while (LYB_LAST_SIBLING(lybctx).written);
}

/**
 * @brief Parse the context of anydata/anyxml node.
 *
 * @param[in] ctx libyang context.
 * @param[in] data LYB data to parse.
 * @param[out] tree Parsed tree.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_any_content(const struct ly_ctx *ctx, const char *data, struct lyd_node **tree)
{
    LY_ERR ret;
    uint32_t prev_lo;
    struct ly_in *in;
    struct lyd_ctx *lydctx = NULL;

    *tree = NULL;

    LY_CHECK_RET(ly_in_new_memory(data, &in));

    /* turn logging off */
    prev_lo = ly_log_options(0);

    ret = _lyd_parse_lyb(ctx, NULL, NULL, tree, in, LYD_PARSE_ONLY | LYD_PARSE_OPAQ | LYD_PARSE_STRICT, 0,
            LYD_INTOPT_ANY | LYD_INTOPT_WITH_SIBLINGS, NULL, &lydctx);

    /* turn logging on again */
    ly_log_options(prev_lo);

    ly_in_free(in, 0);
    if (lydctx) {
        lydctx->free(lydctx);
    }
    if (ret) {
        lyd_free_siblings(*tree);
        *tree = NULL;
    }
    return ret;
}

/**
 * @brief Insert new node to @p parsed set.
 *
 * Also if needed, correct @p first_p.
 *
 * @param[in] lybctx LYB context.
 * @param[in] parent Data parent of the sibling, must be set if @p first_p is not.
 * @param[in,out] node Parsed node to insertion.
 * @param[in,out] first_p First top-level sibling, must be set if @p parent is not.
 * @param[out] parsed Set of all successfully parsed nodes.
 * @return LY_ERR value.
 */
static void
lyb_insert_node(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, struct lyd_node *node, struct lyd_node **first_p,
        struct ly_set *parsed)
{
    /* insert, keep first pointer correct */
    lyd_insert_node(parent, first_p, node, lybctx->parse_opts & LYD_PARSE_ORDERED ? 1 : 0);
    while (!parent && (*first_p)->prev->next) {
        *first_p = (*first_p)->prev;
    }

    /* rememeber a successfully parsed node */
    if (parsed) {
        ly_set_add(parsed, node, 1, NULL);
    }
}

/**
 * @brief Finish parsing the opaq node.
 *
 * @param[in] lybctx LYB context.
 * @param[in] parent Data parent of the sibling, must be set if @p first_p is not.
 * @param[in] flags Node flags to set.
 * @param[in,out] attr Attributes to be attached. Finally set to NULL.
 * @param[in,out] node Parsed opaq node to finish.
 * @param[in,out] first_p First top-level sibling, must be set if @p parent is not.
 * @param[out] parsed Set of all successfully parsed nodes.
 * @return LY_ERR value.
 */
static void
lyb_finish_opaq(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, uint32_t flags, struct lyd_attr **attr,
        struct lyd_node **node, struct lyd_node **first_p, struct ly_set *parsed)
{
    struct lyd_attr *iter;

    /* set flags */
    (*node)->flags = flags;

    /* add attributes */
    assert(!(*node)->schema);
    LY_LIST_FOR(*attr, iter) {
        iter->parent = (struct lyd_node_opaq *)*node;
    }
    ((struct lyd_node_opaq *)*node)->attr = *attr;
    *attr = NULL;

    lyb_insert_node(lybctx, parent, *node, first_p, parsed);
    *node = NULL;
}

/**
 * @brief Finish parsing the node.
 *
 * @param[in] lybctx LYB context.
 * @param[in] parent Data parent of the sibling, must be set if @p first_p is not.
 * @param[in] flags Node flags to set.
 * @param[in,out] meta Metadata to be attached. Finally set to NULL.
 * @param[in,out] node Parsed node to finish.
 * @param[in,out] first_p First top-level sibling, must be set if @p parent is not.
 * @param[out] parsed Set of all successfully parsed nodes.
 * @return LY_ERR value.
 */
static void
lyb_finish_node(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, uint32_t flags, struct lyd_meta **meta,
        struct lyd_node **node, struct lyd_node **first_p, struct ly_set *parsed)
{
    struct lyd_meta *m;

    /* set flags */
    (*node)->flags = flags;

    /* add metadata */
    LY_LIST_FOR(*meta, m) {
        m->parent = *node;
    }
    (*node)->meta = *meta;
    *meta = NULL;

    lyb_insert_node(lybctx, parent, *node, first_p, parsed);
    *node = NULL;
}

/**
 * @brief Parse header for non-opaq node.
 *
 * @param[in] lybctx LYB context.
 * @param[out] flags Parsed node flags.
 * @param[out] meta Parsed metadata of the node.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_node_header(struct lyd_lyb_ctx *lybctx, uint32_t *flags, struct lyd_meta **meta)
{
    LY_ERR ret;

    /* create and read metadata */
    ret = lyb_parse_metadata(lybctx, meta);
    LY_CHECK_RET(ret);

    /* read flags */
    lyb_read_number(flags, sizeof *flags, sizeof *flags, lybctx->lybctx);

    return ret;
}

/**
 * @brief Create term node and fill it with value.
 *
 * @param[in] lybctx LYB context.
 * @param[in] snode Schema of the term node.
 * @param[out] node Created term node.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_create_term(struct lyd_lyb_ctx *lybctx, const struct lysc_node *snode, struct lyd_node **node)
{
    LY_ERR ret;
    ly_bool dynamic;
    uint8_t *term_value;
    uint32_t term_value_len;

    ret = lyb_read_term_value((struct lysc_node_leaf *)snode, &term_value, &term_value_len, lybctx->lybctx);
    LY_CHECK_RET(ret);

    dynamic = 1;
    /* create node */
    ret = lyd_parser_create_term((struct lyd_ctx *)lybctx, snode,
            term_value, term_value_len, &dynamic, LY_VALUE_LYB,
            NULL, LYD_HINT_DATA, node);
    if (dynamic) {
        free(term_value);
    }
    if (ret) {
        lyd_free_tree(*node);
        *node = NULL;
    }

    return ret;
}

/**
 * @brief Validate inner node, autodelete default values nad create implicit nodes.
 *
 * @param[in,out] lybctx LYB context.
 * @param[in] snode Schema of the inner node.
 * @param[in] node Parsed inner node.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_validate_node_inner(struct lyd_lyb_ctx *lybctx, const struct lysc_node *snode, struct lyd_node *node)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t impl_opts;

    if (!(lybctx->parse_opts & LYD_PARSE_ONLY)) {
        /* new node validation, autodelete CANNOT occur, all nodes are new */
        ret = lyd_validate_new(lyd_node_child_p(node), snode, NULL, NULL);
        LY_CHECK_RET(ret);

        /* add any missing default children */
        impl_opts = (lybctx->val_opts & LYD_VALIDATE_NO_STATE) ? LYD_IMPLICIT_NO_STATE : 0;
        ret = lyd_new_implicit_r(node, lyd_node_child_p(node), NULL,
                NULL, &lybctx->node_when, &lybctx->node_exts,
                &lybctx->node_types, impl_opts, NULL);
        LY_CHECK_RET(ret);
    }

    return ret;
}

/**
 * @brief Parse opaq node.
 *
 * @param[in] lybctx LYB context.
 * @param[in] parent Data parent of the sibling.
 * @param[in,out] first_p First top-level sibling.
 * @param[out] parsed Set of all successfully parsed nodes.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_node_opaq(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, struct lyd_node **first_p, struct ly_set *parsed)
{
    LY_ERR ret;
    struct lyd_node *node = NULL;
    struct lyd_attr *attr = NULL;
    char *value = NULL, *name = NULL, *prefix = NULL, *module_key = NULL;
    ly_bool dynamic = 0;
    LY_VALUE_FORMAT format = 0;
    void *val_prefix_data = NULL;
    const struct ly_ctx *ctx = lybctx->lybctx->ctx;
    uint32_t flags;
    uint8_t zero[LYB_SIZE_BYTES] = {0};

    /* parse opaq node attributes */
    ret = lyb_parse_attributes(lybctx->lybctx, &attr);
    LY_CHECK_GOTO(ret, cleanup);

    /* read flags */
    lyb_read_number(&flags, sizeof flags, sizeof flags, lybctx->lybctx);

    /* parse prefix */
    ret = lyb_read_string(&prefix, 1, lybctx->lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* parse module key */
    ret = lyb_read_string(&module_key, 1, lybctx->lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* parse name */
    ret = lyb_read_string(&name, 1, lybctx->lybctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* parse value */
    ret = lyb_read_string(&value, 1, lybctx->lybctx);
    LY_CHECK_ERR_GOTO(ret, ly_free_prefix_data(format, val_prefix_data), cleanup);
    dynamic = 1;

    /* parse format */
    lyb_read_number(&format, sizeof format, 1, lybctx->lybctx);

    /* parse value prefixes */
    ret = lyb_parse_prefix_data(lybctx->lybctx, format, &val_prefix_data);
    LY_CHECK_GOTO(ret, cleanup);

    if (!(lybctx->parse_opts & LYD_PARSE_OPAQ)) {
        if (memcmp(zero, lybctx->lybctx->in->current, LYB_SIZE_BYTES)) {
            /* skip children */
            ret = lyb_read_start_siblings(lybctx->lybctx);
            LY_CHECK_RET(ret);
            lyb_skip_siblings(lybctx->lybctx);
            ret = lyb_read_stop_siblings(lybctx->lybctx);
            LY_CHECK_RET(ret);
        } else {
            /* opaq node has no children */
            lyb_read(NULL, LYB_SIZE_BYTES, lybctx->lybctx);
        }

        goto cleanup;
    }

    /* create node */
    ret = lyd_create_opaq(ctx, name, strlen(name), prefix, ly_strlen(prefix), module_key, ly_strlen(module_key),
            value, strlen(value), &dynamic, format, val_prefix_data, 0, &node);
    LY_CHECK_GOTO(ret, cleanup);

    /* process children */
    ret = lyb_parse_siblings(lybctx, node, NULL, NULL);
    LY_CHECK_GOTO(ret, cleanup);

    /* register parsed opaq node */
    lyb_finish_opaq(lybctx, parent, flags, &attr, &node, first_p, parsed);
    assert(!attr && !node);

cleanup:
    free(prefix);
    free(module_key);
    free(name);
    if (dynamic) {
        free(value);
    }
    lyd_free_attr_siblings(ctx, attr);
    lyd_free_tree(node);

    return ret;
}

/**
 * @brief Parse anydata or anyxml node.
 *
 * @param[in] lybctx LYB context.
 * @param[in] parent Data parent of the sibling.
 * @param[in] snode Schema of the node to be parsed.
 * @param[in,out] first_p First top-level sibling.
 * @param[out] parsed Set of all successfully parsed nodes.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_node_any(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, const struct lysc_node *snode,
        struct lyd_node **first_p, struct ly_set *parsed)
{
    LY_ERR ret;
    struct lyd_node *node = NULL, *tree;
    struct lyd_meta *meta = NULL;
    LYD_ANYDATA_VALUETYPE value_type;
    char *value = NULL;
    const char *val_dict;
    uint32_t flags;
    const struct ly_ctx *ctx = lybctx->lybctx->ctx;

    /* read necessary basic data */
    ret = lyb_parse_node_header(lybctx, &flags, &meta);
    LY_CHECK_GOTO(ret, error);

    /* parse value type */
    lyb_read_number(&value_type, sizeof value_type, sizeof value_type, lybctx->lybctx);
    if (value_type == LYD_ANYDATA_DATATREE) {
        /* invalid situation */
        LOGINT(ctx);
        ret = LY_EINT;
        goto error;
    }

    /* read anydata content */
    ret = lyb_read_string(&value, 1, lybctx->lybctx);
    LY_CHECK_GOTO(ret, error);

    if (value_type == LYD_ANYDATA_LYB) {
        /* try to parse LYB into a data tree */
        if (!lyb_parse_any_content(ctx, value, &tree)) {
            /* successfully parsed */
            free(value);
            value = (char *)tree;
            value_type = LYD_ANYDATA_DATATREE;
        }
    }

    /* create the node */
    switch (value_type) {
    case LYD_ANYDATA_LYB:
    case LYD_ANYDATA_DATATREE:
        /* use the value directly */
        ret = lyd_create_any(snode, value, value_type, 1, &node);
        LY_CHECK_GOTO(ret, error);

        break;
    case LYD_ANYDATA_STRING:
    case LYD_ANYDATA_XML:
    case LYD_ANYDATA_JSON:
        /* value is expected to be in the dictionary */
        ret = lydict_insert_zc(ctx, value, &val_dict);
        LY_CHECK_GOTO(ret, error);

        /* use the value in the dictionary */
        ret = lyd_create_any(snode, val_dict, value_type, 1, &node);
        if (ret) {
            lydict_remove(ctx, val_dict);
            goto error;
        }
        break;
    default:
        LOGINT(ctx);
        ret = LY_EINT;
        goto error;
    }

    /* register parsed anydata node */
    lyb_finish_node(lybctx, parent, flags, &meta, &node, first_p, parsed);

    return LY_SUCCESS;

error:
    free(value);
    lyd_free_meta_siblings(meta);
    lyd_free_tree(node);
    return ret;
}

/**
 * @brief Parse inner node.
 *
 * @param[in] lybctx LYB context.
 * @param[in] parent Data parent of the sibling, must be set if @p first is not.
 * @param[in] snode Schema of the node to be parsed.
 * @param[in,out] first_p First top-level sibling, must be set if @p parent is not.
 * @param[out] parsed Set of all successfully parsed nodes.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_node_inner(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, const struct lysc_node *snode,
        struct lyd_node **first_p, struct ly_set *parsed)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *node = NULL;
    struct lyd_meta *meta = NULL;
    uint32_t flags;

    /* read necessary basic data */
    ret = lyb_parse_node_header(lybctx, &flags, &meta);
    LY_CHECK_GOTO(ret, error);

    /* create node */
    ret = lyd_create_inner(snode, &node);
    LY_CHECK_GOTO(ret, error);

    /* process children */
    ret = lyb_parse_siblings(lybctx, node, NULL, NULL);
    LY_CHECK_GOTO(ret, error);

    /* additional procedure for inner node */
    ret = lyb_validate_node_inner(lybctx, snode, node);
    LY_CHECK_GOTO(ret, error);

    if (snode->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
        /* rememeber the RPC/action/notification */
        lybctx->op_node = node;
    }

    /* register parsed node */
    lyb_finish_node(lybctx, parent, flags, &meta, &node, first_p, parsed);

    return LY_SUCCESS;

error:
    lyd_free_meta_siblings(meta);
    lyd_free_tree(node);
    return ret;
}

/**
 * @brief Parse leaf node.
 *
 * @param[in] lybctx LYB context.
 * @param[in] parent Data parent of the sibling.
 * @param[in] snode Schema of the node to be parsed.
 * @param[in,out] first_p First top-level sibling.
 * @param[out] parsed Set of all successfully parsed nodes.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_node_leaf(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, const struct lysc_node *snode,
        struct lyd_node **first_p, struct ly_set *parsed)
{
    LY_ERR ret;
    struct lyd_node *node = NULL;
    struct lyd_meta *meta = NULL;
    uint32_t flags;

    /* read necessary basic data */
    ret = lyb_parse_node_header(lybctx, &flags, &meta);
    LY_CHECK_GOTO(ret, error);

    /* read value of term node and create it */
    ret = lyb_create_term(lybctx, snode, &node);
    LY_CHECK_GOTO(ret, error);

    lyb_finish_node(lybctx, parent, flags, &meta, &node, first_p, parsed);

    return LY_SUCCESS;

error:
    lyd_free_meta_siblings(meta);
    lyd_free_tree(node);
    return ret;
}

/**
 * @brief Parse all leaflist nodes which belong to same schema.
 *
 * @param[in] lybctx LYB context.
 * @param[in] parent Data parent of the sibling.
 * @param[in] snode Schema of the nodes to be parsed.
 * @param[in,out] first_p First top-level sibling.
 * @param[out] parsed Set of all successfully parsed nodes.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_node_leaflist(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, const struct lysc_node *snode,
        struct lyd_node **first_p, struct ly_set *parsed)
{
    LY_ERR ret;

    /* register a new sibling */
    ret = lyb_read_start_siblings(lybctx->lybctx);
    LY_CHECK_RET(ret);

    /* process all siblings */
    while (LYB_LAST_SIBLING(lybctx->lybctx).written) {
        ret = lyb_parse_node_leaf(lybctx, parent, snode, first_p, parsed);
        LY_CHECK_RET(ret);
    }

    /* end the sibling */
    ret = lyb_read_stop_siblings(lybctx->lybctx);
    LY_CHECK_RET(ret);

    return ret;
}

/**
 * @brief Parse all list nodes which belong to same schema.
 *
 * @param[in] lybctx LYB context.
 * @param[in] parent Data parent of the sibling.
 * @param[in] snode Schema of the nodes to be parsed.
 * @param[in,out] first_p First top-level sibling.
 * @param[out] parsed Set of all successfully parsed nodes.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_node_list(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, const struct lysc_node *snode,
        struct lyd_node **first_p, struct ly_set *parsed)
{
    LY_ERR ret;
    struct lyd_node *node = NULL;
    struct lyd_meta *meta = NULL;
    uint32_t flags;

    /* register a new sibling */
    ret = lyb_read_start_siblings(lybctx->lybctx);
    LY_CHECK_RET(ret);

    while (LYB_LAST_SIBLING(lybctx->lybctx).written) {
        /* read necessary basic data */
        ret = lyb_parse_node_header(lybctx, &flags, &meta);
        LY_CHECK_GOTO(ret, error);

        /* create list node */
        ret = lyd_create_inner(snode, &node);
        LY_CHECK_GOTO(ret, error);

        /* process children */
        ret = lyb_parse_siblings(lybctx, node, NULL, NULL);
        LY_CHECK_GOTO(ret, error);

        /* additional procedure for inner node */
        ret = lyb_validate_node_inner(lybctx, snode, node);
        LY_CHECK_GOTO(ret, error);

        if (snode->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
            /* rememeber the RPC/action/notification */
            lybctx->op_node = node;
        }

        /* register parsed list node */
        lyb_finish_node(lybctx, parent, flags, &meta, &node, first_p, parsed);
    }

    /* end the sibling */
    ret = lyb_read_stop_siblings(lybctx->lybctx);
    LY_CHECK_RET(ret);

    return LY_SUCCESS;

error:
    lyd_free_meta_siblings(meta);
    lyd_free_tree(node);
    return ret;
}

/**
 * @brief Parse node.
 *
 * @param[in] out Out structure.
 * @param[in,out] printed_node Current data node to print. Sets to the last printed node.
 * @param[in,out] sibling_ht Cached hash table for these siblings, created if NULL.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_node(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, struct lyd_node **first_p,
        struct ly_set *parsed)
{
    LY_ERR ret;
    const struct lysc_node *snode;
    const struct lys_module *mod;

    if (!parent || !parent->schema) {
        /* top-level or opaque, read module name */
        ret = lyb_parse_model(lybctx->lybctx, lybctx->parse_opts, &mod);
        LY_CHECK_RET(ret);

        /* read hash, find the schema node starting from mod */
        ret = lyb_parse_schema_hash(lybctx, NULL, mod, &snode);
    } else {
        /* read hash, find the schema node starting from parent schema */
        ret = lyb_parse_schema_hash(lybctx, parent->schema, NULL, &snode);
    }
    LY_CHECK_RET(ret);

    if (!snode) {
        ret = lyb_parse_node_opaq(lybctx, parent, first_p, parsed);
    } else if (snode->nodetype & LYS_LEAFLIST) {
        ret = lyb_parse_node_leaflist(lybctx, parent, snode, first_p, parsed);
    } else if (snode->nodetype == LYS_LIST) {
        ret = lyb_parse_node_list(lybctx, parent, snode, first_p, parsed);
    } else if (snode->nodetype & LYD_NODE_ANY) {
        ret = lyb_parse_node_any(lybctx, parent, snode, first_p, parsed);
    } else if (snode->nodetype & LYD_NODE_INNER) {
        ret = lyb_parse_node_inner(lybctx, parent, snode, first_p, parsed);
    } else {
        ret = lyb_parse_node_leaf(lybctx, parent, snode, first_p, parsed);
    }
    LY_CHECK_RET(ret);

    return ret;
}

/**
 * @brief Parse siblings (@ref lyb_print_siblings()).
 *
 * @param[in] lybctx LYB context.
 * @param[in] parent Data parent of the sibling, must be set if @p first_p is not.
 * @param[in,out] first_p First top-level sibling, must be set if @p parent is not.
 * @param[out] parsed Set of all successfully parsed nodes.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_siblings(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, struct lyd_node **first_p,
        struct ly_set *parsed)
{
    LY_ERR ret;
    ly_bool top_level;
    uint8_t zero[LYB_SIZE_BYTES] = {0};

    if (!memcmp(zero, lybctx->lybctx->in->current, LYB_SIZE_BYTES)) {
        lyb_read(NULL, LYB_SIZE_BYTES, lybctx->lybctx);
        return LY_SUCCESS;
    }

    top_level = !LY_ARRAY_COUNT(lybctx->lybctx->siblings);

    /* register a new siblings */
    ret = lyb_read_start_siblings(lybctx->lybctx);
    LY_CHECK_RET(ret);

    while (LYB_LAST_SIBLING(lybctx->lybctx).written) {
        ret = lyb_parse_node(lybctx, parent, first_p, parsed);
        LY_CHECK_RET(ret);

        if (top_level && !(lybctx->int_opts & LYD_INTOPT_WITH_SIBLINGS)) {
            break;
        }
    }

    /* end the siblings */
    ret = lyb_read_stop_siblings(lybctx->lybctx);
    LY_CHECK_RET(ret);

    return ret;
}

/**
 * @brief Parse used YANG data models.
 *
 * @param[in] lybctx LYB context.
 * @param[in] parse_options Flag with options for parsing.
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

static LY_ERR
_lyd_parse_lyb(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
        struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, uint32_t int_opts,
        struct ly_set *parsed, struct lyd_ctx **lydctx_p)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_lyb_ctx *lybctx;

    assert(!(parse_opts & ~LYD_PARSE_OPTS_MASK));
    assert(!(val_opts & ~LYD_VALIDATE_OPTS_MASK));

    lybctx = calloc(1, sizeof *lybctx);
    LY_CHECK_ERR_RET(!lybctx, LOGMEM(ctx), LY_EMEM);
    lybctx->lybctx = calloc(1, sizeof *lybctx->lybctx);
    LY_CHECK_ERR_GOTO(!lybctx->lybctx, LOGMEM(ctx); rc = LY_EMEM, cleanup);

    lybctx->lybctx->in = in;
    lybctx->lybctx->ctx = ctx;
    lybctx->parse_opts = parse_opts;
    lybctx->val_opts = val_opts;
    lybctx->int_opts = int_opts;
    lybctx->free = lyd_lyb_ctx_free;
    lybctx->ext = ext;

    /* find the operation node if it exists already */
    LY_CHECK_GOTO(rc = lyd_parser_find_operation(parent, int_opts, &lybctx->op_node), cleanup);

    /* read magic number */
    rc = lyb_parse_magic_number(lybctx->lybctx);
    LY_CHECK_GOTO(rc, cleanup);

    /* read header */
    rc = lyb_parse_header(lybctx->lybctx);
    LY_CHECK_GOTO(rc, cleanup);

    /* read used models */
    rc = lyb_parse_data_models(lybctx->lybctx, lybctx->parse_opts);
    LY_CHECK_GOTO(rc, cleanup);

    /* read sibling(s) */
    rc = lyb_parse_siblings(lybctx, parent, first_p, parsed);
    LY_CHECK_GOTO(rc, cleanup);

    if ((int_opts & LYD_INTOPT_NO_SIBLINGS) && lybctx->lybctx->in->current[0]) {
        LOGVAL(ctx, LYVE_SYNTAX, "Unexpected sibling node.");
        rc = LY_EVALID;
        goto cleanup;
    }
    if ((int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION | LYD_INTOPT_NOTIF | LYD_INTOPT_REPLY)) && !lybctx->op_node) {
        LOGVAL(ctx, LYVE_DATA, "Missing the operation node.");
        rc = LY_EVALID;
        goto cleanup;
    }

    /* read the last zero, parsing finished */
    ly_in_skip(lybctx->lybctx->in, 1);

cleanup:
    /* there should be no unres stored if validation should be skipped */
    assert(!(parse_opts & LYD_PARSE_ONLY) || (!lybctx->node_types.count && !lybctx->meta_types.count &&
            !lybctx->node_when.count));

    if (rc) {
        lyd_lyb_ctx_free((struct lyd_ctx *)lybctx);
    } else {
        *lydctx_p = (struct lyd_ctx *)lybctx;
    }
    return rc;
}

LY_ERR
lyd_parse_lyb(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
        struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, enum lyd_type data_type,
        struct ly_set *parsed, struct lyd_ctx **lydctx_p)
{
    uint32_t int_opts;

    assert(!(parse_opts & ~LYD_PARSE_OPTS_MASK));
    assert(!(val_opts & ~LYD_VALIDATE_OPTS_MASK));

    switch (data_type) {
    case LYD_TYPE_DATA_YANG:
        int_opts = LYD_INTOPT_WITH_SIBLINGS;
        break;
    case LYD_TYPE_RPC_YANG:
        int_opts = LYD_INTOPT_RPC | LYD_INTOPT_ACTION | LYD_INTOPT_NO_SIBLINGS;
        break;
    case LYD_TYPE_NOTIF_YANG:
        int_opts = LYD_INTOPT_NOTIF | LYD_INTOPT_NO_SIBLINGS;
        break;
    case LYD_TYPE_REPLY_YANG:
        int_opts = LYD_INTOPT_REPLY | LYD_INTOPT_NO_SIBLINGS;
        break;
    default:
        LOGINT(ctx);
        return LY_EINT;
    }

    return _lyd_parse_lyb(ctx, ext, parent, first_p, in, parse_opts, val_opts, int_opts, parsed, lydctx_p);
}

API int
lyd_lyb_data_length(const char *data)
{
    LY_ERR ret = LY_SUCCESS;
    struct lylyb_ctx *lybctx;
    int count, i;
    size_t len;
    uint8_t buf[LYB_SIZE_MAX];
    uint8_t zero[LYB_SIZE_BYTES] = {0};

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

    if (memcmp(zero, lybctx->in->current, LYB_SIZE_BYTES)) {
        /* register a new sibling */
        ret = lyb_read_start_siblings(lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* skip it */
        lyb_skip_siblings(lybctx);

        /* sibling finished */
        ret = lyb_read_stop_siblings(lybctx);
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        lyb_read(NULL, LYB_SIZE_BYTES, lybctx);
    }

    /* read the last zero, parsing finished */
    ly_in_skip(lybctx->in, 1);

cleanup:
    count = lybctx->in->current - lybctx->in->start;

    ly_in_free(lybctx->in, 0);
    lylyb_ctx_free(lybctx);

    return ret ? -1 : count;
}
