/**
 * @file parser_lyb.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief LYB data parser for libyang
 *
 * Copyright (c) 2020 - 2025 CESNET, z.s.p.o.
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

#include "compat.h"
#include "context.h"
#include "dict.h"
#include "hash_table.h"
#include "in.h"
#include "in_internal.h"
#include "log.h"
#include "ly_common.h"
#include "parser_data.h"
#include "parser_internal.h"
#include "plugins_exts.h"
#include "plugins_exts/metadata.h"
#include "plugins_internal.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "validation.h"
#include "xml.h"

static LY_ERR lyb_parse_siblings(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, ly_bool is_root,
        struct lyd_node **first_p, struct ly_set *parsed);

/**
 * @brief Free a LYB data parser context.
 *
 * @param[in] lydctx Parser context to free.
 */
static void
lyb_parse_ctx_free(struct lyd_ctx *lydctx)
{
    struct lyd_lyb_ctx *ctx = (struct lyd_lyb_ctx *)lydctx;

    if (!ctx) {
        return;
    }

    lyd_ctx_free(lydctx);

    /* nothing parser-specific to free */

    free(ctx->parse_ctx);
    free(ctx);
}

/**
 * @brief Read data from the input.
 *
 * @param[in,out] buf Destination buffer, @p count_bits rightmost bits are written to, rest are shifted left.
 * @param[in] count_bits Number of bits to read.
 * @param[in] lybctx LYB context.
 */
static void
lyb_read(void *buf, uint64_t count_bits, struct lylyb_parse_ctx *lybctx)
{
    uint8_t count_buf_bits = 0, count_bits_remainder, new_buf;
    uint64_t count_bytes;

    assert(lybctx);

    if (!count_bits) {
        return;
    }

    if (lybctx->buf_bits) {
        /* will spent buffered bits */
        count_buf_bits = (count_bits > lybctx->buf_bits) ? lybctx->buf_bits : count_bits;
        count_bits -= count_buf_bits;
    }

    count_bits_remainder = count_bits % 8;

    count_bytes = count_bits / 8 + (count_bits_remainder ? 1 : 0);
    if (count_bytes) {
        if (buf) {
            /* read the bytes */
            ly_in_read(lybctx->in, buf, count_bytes);
            if (count_bits_remainder) {
                /* prepare new buffered bits */
                new_buf = ((uint8_t *)buf)[count_bytes - 1];

                /* zero the unused bits by shifting the last byte */
                ((uint8_t *)buf)[count_bytes - 1] &= lyb_right_bit_mask(count_bits_remainder);
            }
        } else {
            if (count_bits_remainder) {
                /* skip the bytes and prepare the new buffered bits */
                ly_in_skip(lybctx->in, count_bytes - 1);
                ly_in_read(lybctx->in, &new_buf, 1);
            } else {
                /* just skip the bytes */
                ly_in_skip(lybctx->in, count_bytes);
            }
        }
    }

    if (count_buf_bits) {
        if (buf) {
            /* prepend the buffered bits */
            lyb_prepend_bits(buf, LYPLG_BITS2BYTES(count_buf_bits + count_bits), lybctx->buf, count_buf_bits);
        }

        /* remove buffered bits */
        lybctx->buf_bits -= count_buf_bits;
        lybctx->buf >>= count_buf_bits;
    }

    if (count_bits_remainder) {
        assert(!lybctx->buf_bits);

        /* store new buffered rightmost bits */
        lybctx->buf = new_buf >> count_bits_remainder;
        lybctx->buf_bits = 8 - count_bits_remainder;
    }
}

/**
 * @brief Peek data from the input.
 *
 * @param[in,out] buf Destination buffer, @p count_bits rightmost bits are written to, rest are shifted left.
 * @param[in] count_bits Number of bits to peek.
 * @param[in] lybctx LYB context.
 */
static void
lyb_peek(void *buf, uint64_t count_bits, struct lylyb_parse_ctx *lybctx)
{
    uint8_t count_buf_bits = 0, peek;

    assert(lybctx && (count_bits < 9));

    if (!count_bits) {
        return;
    }

    if (lybctx->buf_bits) {
        /* peek the buffered bits */
        count_buf_bits = (lybctx->buf_bits > count_bits) ? count_bits : lybctx->buf_bits;

        /* keep the original leftmost bits and write new peeked rightmost bits */
        ((uint8_t *)buf)[0] = (((uint8_t *)buf)[0] << count_bits) | (lybctx->buf & lyb_right_bit_mask(count_buf_bits));
        count_bits -= count_buf_bits;
    }

    if (!count_bits) {
        return;
    }

    /* need to peek input */
    ly_in_peek(lybctx->in, &peek);

    /* prepend the newly peeked bits in front of the previously peeked bits */
    ((uint8_t *)buf)[0] |= (peek & lyb_right_bit_mask(count_bits)) << count_buf_bits;
}

/**
 * @brief Read a count.
 *
 * @param[in,out] count Destination count buffer, must be zeroed because only relevant bits will be used.
 * @param[in] lybctx LYB context.
 */
static void
lyb_read_count(uint32_t *count, struct lylyb_parse_ctx *lybctx)
{
    uint8_t prefix = 0, pref_len = 0;

    /* read all prefix bits */
    do {
        ++pref_len;
        prefix <<= 1;

        lyb_read(&prefix, 1, lybctx);
    } while (prefix & 0x1);

    switch (pref_len) {
    case 1:
        /* 0 */
        break;
    case 2:
        /* 10 */
        lyb_read(count, 4, lybctx);
        break;
    case 3:
        /* 110 */
        lyb_read(count, 5, lybctx);
        break;
    case 4:
        /* 1110 */
        lyb_read(count, 7, lybctx);
        break;
    case 5:
        /* 11110 */
        lyb_read(count, 11, lybctx);
        break;
    case 6:
        /* 111110 */
        lyb_read(count, 26, lybctx);
        break;
    default:
        /* invalid */
        LOGINT(lybctx->ctx);
        return;
    }

    /* correct byte order */
    *count = le32toh(*count);
}

/**
 * @brief Read a size, may be bits or bytes.
 *
 * @param[in,out] size Destination size buffer, must be zeroed because only relevant bits will be used.
 * @param[in] lybctx LYB context.
 */
static void
lyb_read_size(uint32_t *size, struct lylyb_parse_ctx *lybctx)
{
    uint8_t prefix = 0, pref_len = 0;

    /* read all prefix bits */
    do {
        ++pref_len;
        prefix <<= 1;

        lyb_read(&prefix, 1, lybctx);
    } while (prefix & 0x1);

    switch (pref_len) {
    case 1:
        /* 0 */
        lyb_read(size, 4, lybctx);
        break;
    case 2:
        /* 10 */
        lyb_read(size, 6, lybctx);
        break;
    case 3:
        /* 110 */
        lyb_read(size, 12, lybctx);
        break;
    case 4:
        /* 1110 */
        lyb_read(size, 32, lybctx);
        break;
    default:
        /* invalid */
        LOGINT(lybctx->ctx);
        return;
    }

    /* correct byte order */
    *size = le32toh(*size);
}

/**
 * @brief Read a string.
 *
 * @param[in] str Destination buffer, is allocated.
 * @param[in] len_size Number of bytes on which the length of the string is written.
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_read_string(char **str, struct lylyb_parse_ctx *lybctx)
{
    uint32_t str_len = 0;

    *str = NULL;

    /* read length in bytes */
    lyb_read_size(&str_len, lybctx);

    /* allocate mem */
    *str = malloc((str_len + 1) * sizeof **str);
    LY_CHECK_ERR_RET(!*str, LOGMEM(lybctx->ctx), LY_EMEM);

    if (str_len) {
        /* read the string */
        lyb_read(*str, str_len * 8, lybctx);
    }

    (*str)[str_len] = '\0';
    return LY_SUCCESS;
}

/**
 * @brief Read a value.
 *
 * @param[in] type Type of the value.
 * @param[out] val Value buffer to read into, always terminated by 0.
 * @param[out] val_size_bits Read @p val size in bits.
 * @param[in,out] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_read_value(const struct lysc_type *type, uint8_t **val, uint32_t *val_size_bits, struct lylyb_parse_ctx *lybctx)
{
    enum lyplg_lyb_size_type size_type;
    uint32_t fixed_size_bits;
    struct lysc_type_leafref *type_lf;

    assert(type && val && val_size_bits && lybctx);

    *val = NULL;
    *val_size_bits = 0;

    /* learn the size from @ref howtoDataLYB */
    if (type->basetype == LY_TYPE_LEAFREF) {
        /* leafref itself is ignored, the target is loaded directly */
        type_lf = (struct lysc_type_leafref *)type;
        LYSC_GET_TYPE_PLG(type_lf->realtype->plugin_ref)->lyb_size(type_lf->realtype, &size_type, &fixed_size_bits);
    } else {
        LYSC_GET_TYPE_PLG(type->plugin_ref)->lyb_size(type, &size_type, &fixed_size_bits);
    }

    if (size_type == LYPLG_LYB_SIZE_FIXED_BITS) {
        /* data size is fixed */
        *val_size_bits = fixed_size_bits;
    } else {
        /* parse value size in bits or bytes */
        lyb_read_size(val_size_bits, lybctx);
        if (size_type == LYPLG_LYB_SIZE_VARIABLE_BYTES) {
            *val_size_bits *= 8;
        }
    }

    /* allocate zeroed memory with an addition zero byte */
    *val = calloc(LYPLG_BITS2BYTES(*val_size_bits) + 1, sizeof **val);
    LY_CHECK_ERR_RET(!*val, LOGMEM(lybctx->ctx), LY_EMEM);

    if (*val_size_bits > 0) {
        /* parse value */
        lyb_read(*val, *val_size_bits, lybctx);
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse YANG module info stored separately as module name and its revision.
 *
 * @param[in] lybctx LYB context.
 * @param[out] mod_name Module name, if any.
 * @param[out] mod_rev Module revision, "" if none.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_module(struct lylyb_parse_ctx *lybctx, char **mod_name, char mod_rev[])
{
    uint16_t rev;

    *mod_name = NULL;
    mod_rev[0] = '\0';

    /* module name */
    lyb_read_string(mod_name, lybctx);
    if (!(*mod_name)[0]) {
        return LY_SUCCESS;
    }

    /* module revision */
    lyb_read(&rev, 2 * 8, lybctx);
    if (rev) {
        sprintf(mod_rev, "%04u-%02u-%02u", ((rev & LYB_REV_YEAR_MASK) >> LYB_REV_YEAR_SHIFT) + LYB_REV_YEAR_OFFSET,
                (rev & LYB_REV_MONTH_MASK) >> LYB_REV_MONTH_SHIFT, rev & LYB_REV_DAY_MASK);
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse YANG module info as only the index of the module in the context.
 *
 * @param[in] lybctx LYB context.
 * @param[out] mod Parsed module.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_module_idx(struct lylyb_parse_ctx *lybctx, const struct lys_module **mod)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lys_module *m = NULL;
    uint32_t idx = 0;

    /* read module index */
    lyb_read_count(&idx, lybctx);

    /* get the module */
    assert(idx < lybctx->ctx->modules.count);
    m = lybctx->ctx->modules.objs[idx];
    if (!m->implemented) {
        LOGERR(lybctx->ctx, LY_EINT, "Invalid context for LYB data parsing, module \"%s%s%s\" not implemented.",
                m->name, m->revision ? "@" : "", m->revision ? m->revision : "");
        rc = LY_EINT;
        goto cleanup;
    }

cleanup:
    *mod = m;
    return rc;
}

/**
 * @brief Parse YANG node metadata.
 *
 * @param[in] lybctx LYB context.
 * @param[in] sparent Schema parent node of the metadata.
 * @param[out] meta Parsed metadata.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_metadata(struct lyd_lyb_ctx *lybctx, const struct lysc_node *sparent, struct lyd_meta **meta)
{
    LY_ERR rc = LY_SUCCESS;
    ly_bool dynamic;
    uint32_t i, count = 0, value_size_bits;
    char *meta_name = NULL;
    uint8_t *value;
    const struct lys_module *mod;
    struct lysc_ext_instance *ant;
    const struct lysc_type *ant_type;

    /* read number of metadata stored */
    lyb_read_count(&count, lybctx->parse_ctx);

    for (i = 0; i < count; ++i) {
        /* find module */
        rc = lyb_parse_module_idx(lybctx->parse_ctx, &mod);
        LY_CHECK_GOTO(rc, cleanup);

        /* meta name */
        rc = lyb_read_string(&meta_name, lybctx->parse_ctx);
        LY_CHECK_GOTO(rc, cleanup);

        /* get metadata type */
        ant = lyd_get_meta_annotation(mod, meta_name, strlen(meta_name));
        if (!ant) {
            LOGVAL(lybctx->parse_ctx->ctx, LYVE_REFERENCE, "Annotation definition for metadata \"%s:%s\" not found.",
                    mod->name, meta_name);
            rc = LY_EINT;
            goto cleanup;
        }
        lyplg_ext_get_storage(ant, LY_STMT_TYPE, sizeof ant_type, (const void **)&ant_type);

        /* meta value */
        rc = lyb_read_value(ant_type, &value, &value_size_bits, lybctx->parse_ctx);
        LY_CHECK_GOTO(rc, cleanup);
        dynamic = 1;

        /* create metadata */
        rc = lyd_parser_create_meta((struct lyd_ctx *)lybctx, NULL, meta, mod, meta_name, strlen(meta_name), value,
                value_size_bits, &dynamic, LY_VALUE_LYB, NULL, LYD_HINT_DATA, sparent);

        /* free strings */
        free(meta_name);
        meta_name = NULL;
        if (dynamic) {
            free(value);
            dynamic = 0;
        }

        LY_CHECK_GOTO(rc, cleanup);
    }

cleanup:
    free(meta_name);
    if (rc) {
        lyd_free_meta_siblings(*meta);
        *meta = NULL;
    }
    return rc;
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
lyb_parse_prefix_data(struct lylyb_parse_ctx *lybctx, LY_VALUE_FORMAT format, void **prefix_data)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t count = 0, i;
    struct ly_set *set = NULL;
    struct lyxml_ns *ns = NULL;

    switch (format) {
    case LY_VALUE_XML:
        /* read count */
        lyb_read_count(&count, lybctx);

        /* read all NS elements */
        LY_CHECK_GOTO(ret = ly_set_new(&set), cleanup);

        for (i = 0; i < count; ++i) {
            ns = calloc(1, sizeof *ns);

            /* prefix */
            LY_CHECK_GOTO(ret = lyb_read_string(&ns->prefix, lybctx), cleanup);
            if (!strlen(ns->prefix)) {
                free(ns->prefix);
                ns->prefix = NULL;
            }

            /* namespace */
            LY_CHECK_GOTO(ret = lyb_read_string(&ns->uri, lybctx), cleanup);

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
lyb_parse_attributes(struct lylyb_parse_ctx *lybctx, struct lyd_attr **attr)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t count = 0, i;
    struct lyd_attr *attr2 = NULL;
    char *prefix = NULL, *module_name = NULL, *name = NULL, *value = NULL;
    ly_bool dynamic = 0;
    LY_VALUE_FORMAT format = 0;
    void *val_prefix_data = NULL;
    uint8_t byte = 0;

    /* read count */
    lyb_read_count(&count, lybctx);

    /* read attributes */
    for (i = 0; i < count; ++i) {
        /* prefix, may be empty */
        ret = lyb_read_string(&prefix, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        if (!prefix[0]) {
            free(prefix);
            prefix = NULL;
        }

        /* namespace, may be empty */
        ret = lyb_read_string(&module_name, lybctx);
        LY_CHECK_GOTO(ret, cleanup);
        if (!module_name[0]) {
            free(module_name);
            module_name = NULL;
        }

        /* name */
        ret = lyb_read_string(&name, lybctx);
        LY_CHECK_GOTO(ret, cleanup);

        /* format */
        lyb_read(&byte, LYB_OPAQ_FORMAT_BITS, lybctx);
        if (byte == LYB_OPAQ_FORMAT_XML) {
            format = LY_VALUE_XML;
        } else {
            assert(byte == LYB_OPAQ_FORMAT_JSON);
            format = LY_VALUE_JSON;
        }

        /* value prefixes */
        ret = lyb_parse_prefix_data(lybctx, format, &val_prefix_data);
        LY_CHECK_GOTO(ret, cleanup);

        /* value */
        ret = lyb_read_string(&value, lybctx);
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
lyb_read_hashes(struct lylyb_parse_ctx *lybctx, LYB_HASH *hash, uint8_t *hash_count)
{
    uint8_t i = 0, j;

    /* read the first hash */
    lyb_read(&hash[0], sizeof *hash * 8, lybctx);

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
        lyb_read(&hash[j - 1], sizeof *hash * 8, lybctx);

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
    LY_ERR r;
    const struct lysc_node *sibling;
    LYB_HASH hash[LYB_HASH_BITS - 1];
    uint32_t getnext_opts;
    uint8_t hash_count;

    *snode = NULL;

    LY_CHECK_RET(lyb_read_hashes(lybctx->parse_ctx, hash, &hash_count));

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

        if (lyb_is_schema_hash_match((struct lysc_node *)sibling, hash, hash_count)) {
            /* match found */
            break;
        }
    }

    if (!sibling) {
        if (lybctx->ext) {
            LOGERR(lybctx->parse_ctx->ctx, LY_EINT, "Failed to find matching hash for a node from \"%s\" extension instance node.",
                    lybctx->ext->def->name);
        } else if (mod) {
            LOGERR(lybctx->parse_ctx->ctx, LY_EINT, "Failed to find matching hash for a top-level node from \"%s\".",
                    mod->name);
        } else {
            assert(sparent);
            LOGERR(lybctx->parse_ctx->ctx, LY_EINT, "Failed to find matching hash for a child node of \"%s\".",
                    sparent->name);
        }
        return LY_EINT;
    } else if (sibling && (r = lyd_parser_check_schema((struct lyd_ctx *)lybctx, sibling))) {
        return r;
    }

    *snode = sibling;
    return LY_SUCCESS;
}

/**
 * @brief Parse schema node name of a nested extension data node.
 *
 * @param[in] lybctx LYB context.
 * @param[in] parent Data parent.
 * @param[in] mod_name Module name of the node.
 * @param[out] snode Parsed found schema node of a nested extension.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_schema_nested_ext(struct lyd_lyb_ctx *lybctx, const struct lyd_node *parent, const char *mod_name,
        const struct lysc_node **snode)
{
    LY_ERR rc = LY_SUCCESS, r;
    char *name = NULL;
    struct lysc_ext_instance *ext;

    assert(parent);

    /* read schema node name */
    LY_CHECK_GOTO(rc = lyb_read_string(&name, lybctx->parse_ctx), cleanup);

    /* check for extension data */
    r = ly_nested_ext_schema(parent, NULL, mod_name, mod_name ? strlen(mod_name) : 0, LY_VALUE_JSON, NULL, name,
            strlen(name), snode, &ext);
    if (r == LY_ENOT) {
        /* failed to parse */
        LOGERR(lybctx->parse_ctx->ctx, LY_EINVAL, "Failed to parse node \"%s\" as nested extension instance data.", name);
        rc = LY_EINVAL;
        goto cleanup;
    } else if (r) {
        /* error */
        rc = r;
        goto cleanup;
    }

    if (!((*snode)->module->ctx->opts & LY_CTX_LYB_HASHES)) {
        /* generate LYB hashes */
        LY_CHECK_GOTO(rc = ly_ctx_set_options((*snode)->module->ctx, LY_CTX_LYB_HASHES), cleanup);
    }

cleanup:
    free(name);
    return rc;
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
    if (parent && (LYD_CTX(parent) != LYD_CTX(node))) {
        lyplg_ext_insert(parent, node);
    } else {
        lyd_insert_node(parent, first_p, node,
                lybctx->parse_opts & LYD_PARSE_ORDERED ? LYD_INSERT_NODE_LAST : LYD_INSERT_NODE_DEFAULT);
    }
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
 * @param[in,out] attr Attributes to be attached. Finally set to NULL.
 * @param[in,out] node Parsed opaq node to finish.
 * @param[in,out] first_p First top-level sibling, must be set if @p parent is not.
 * @param[out] parsed Set of all successfully parsed nodes.
 * @return LY_ERR value.
 */
static void
lyb_finish_opaq(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, struct lyd_attr **attr, struct lyd_node **node,
        struct lyd_node **first_p, struct ly_set *parsed)
{
    struct lyd_attr *iter;

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

    /* insert into parent */
    lyb_insert_node(lybctx, parent, *node, first_p, parsed);

    if (!(lybctx->parse_opts & LYD_PARSE_ONLY)) {
        /* store for ext instance node validation, if needed */
        (void)lyd_validate_node_ext(*node, &lybctx->ext_node);
    }

    *node = NULL;
}

/**
 * @brief Parse header for non-opaq node.
 *
 * @param[in] lybctx LYB context.
 * @param[in] sparent Schema parent node of the metadata.
 * @param[out] flags Parsed node flags.
 * @param[out] meta Parsed metadata of the node.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_node_header(struct lyd_lyb_ctx *lybctx, const struct lysc_node *sparent, uint32_t *flags, struct lyd_meta **meta)
{
    /* create and read metadata */
    LY_CHECK_RET(lyb_parse_metadata(lybctx, sparent, meta));

    if (!lybctx->parse_ctx->shrink) {
        /* read flags, fixed bits */
        lyb_read(flags, LYB_DATA_NODE_FLAG_BITS, lybctx->parse_ctx);
    } else {
        *flags = LYD_NEW;
    }

    return LY_SUCCESS;
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
    LY_ERR rc;
    ly_bool dynamic;
    uint8_t *value;
    uint32_t value_size_bits;

    /* parse the value */
    LY_CHECK_RET(lyb_read_value(((struct lysc_node_leaf *)snode)->type, &value, &value_size_bits, lybctx->parse_ctx));
    dynamic = 1;

    /* create node */
    rc = lyd_parser_create_term((struct lyd_ctx *)lybctx, snode, value, value_size_bits, &dynamic, LY_VALUE_LYB,
            NULL, LYD_HINT_DATA, node);
    if (dynamic) {
        free(value);
    }

    if (rc) {
        lyd_free_tree(*node);
        *node = NULL;
    }
    return rc;
}

/**
 * @brief Validate inner node, autodelete default values nad create implicit nodes.
 *
 * @param[in,out] lybctx LYB context.
 * @param[in] node Parsed inner node.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_validate_node_inner(struct lyd_lyb_ctx *lybctx, struct lyd_node *node)
{
    LY_ERR rc = LY_SUCCESS;

    if (!(lybctx->parse_opts & LYD_PARSE_ONLY)) {
        /* new node validation */
        rc = lyd_parser_validate_new_implicit((struct lyd_ctx *)lybctx, node);
    }

    return rc;
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
    uint8_t byte = 0;
    LY_VALUE_FORMAT format;
    void *val_prefix_data = NULL;
    const struct ly_ctx *ctx = lybctx->parse_ctx->ctx;

    /* parse opaq node attributes */
    ret = lyb_parse_attributes(lybctx->parse_ctx, &attr);
    LY_CHECK_GOTO(ret, cleanup);

    /* parse prefix */
    ret = lyb_read_string(&prefix, lybctx->parse_ctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* parse module key */
    ret = lyb_read_string(&module_key, lybctx->parse_ctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* parse name */
    ret = lyb_read_string(&name, lybctx->parse_ctx);
    LY_CHECK_GOTO(ret, cleanup);

    /* parse value */
    ret = lyb_read_string(&value, lybctx->parse_ctx);
    LY_CHECK_GOTO(ret, cleanup);
    dynamic = 1;

    /* parse format */
    lyb_read(&byte, LYB_OPAQ_FORMAT_BITS, lybctx->parse_ctx);
    if (byte == LYB_OPAQ_FORMAT_XML) {
        format = LY_VALUE_XML;
    } else {
        assert(byte == LYB_OPAQ_FORMAT_JSON);
        format = LY_VALUE_JSON;
    }

    /* parse value prefixes */
    ret = lyb_parse_prefix_data(lybctx->parse_ctx, format, &val_prefix_data);
    LY_CHECK_GOTO(ret, cleanup);

    /* create node */
    ret = lyd_create_opaq(ctx, name, strlen(name), prefix, ly_strlen(prefix), module_key, ly_strlen(module_key),
            value, strlen(value), &dynamic, format, val_prefix_data, LYD_HINT_DATA, &node);
    LY_CHECK_ERR_GOTO(ret, ly_free_prefix_data(format, val_prefix_data), cleanup);

    assert(node);
    LOG_LOCSET(NULL, node);

    /* process children */
    ret = lyb_parse_siblings(lybctx, node, 0, NULL, NULL);
    LY_CHECK_GOTO(ret, cleanup);

    if (lybctx->parse_opts & LYD_PARSE_OPAQ) {
        /* register parsed opaq node */
        lyb_finish_opaq(lybctx, parent, &attr, &node, first_p, parsed);
        assert(!attr && !node);
        LOG_LOCBACK(0, 1);
    } /* else is freed */

cleanup:
    if (node) {
        LOG_LOCBACK(0, 1);
    }
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
    LY_ERR rc;
    struct lyd_node *node = NULL, *tree = NULL;
    struct lyd_meta *meta = NULL;
    LYD_ANYDATA_VALUETYPE value_type = 0;
    char *value = NULL;
    uint32_t flags = 0, prev_parse_opts, prev_int_opts;
    const struct ly_ctx *ctx = lybctx->parse_ctx->ctx;

    /* read necessary basic data */
    rc = lyb_parse_node_header(lybctx, snode, &flags, &meta);
    LY_CHECK_GOTO(rc, error);

    /* parse value type */
    lyb_read_count((uint32_t *)&value_type, lybctx->parse_ctx);

    /* create the node */
    switch (value_type) {
    case LYD_ANYDATA_DATATREE:
        /* backup original options and use specific ones */
        prev_parse_opts = lybctx->parse_opts;
        prev_int_opts = lybctx->int_opts;
        lybctx->parse_opts = LYD_PARSE_ONLY | LYD_PARSE_OPAQ;
        lybctx->int_opts = LYD_INTOPT_ANY | LYD_INTOPT_WITH_SIBLINGS;

        /* parse LYB siblings, restore options */
        rc = lyb_parse_siblings(lybctx, NULL, 0, &tree, NULL);
        lybctx->parse_opts = prev_parse_opts;
        lybctx->int_opts = prev_int_opts;
        LY_CHECK_GOTO(rc, error);

        /* use the data tree value */
        rc = lyd_create_any(snode, tree, value_type, 1, &node);
        LY_CHECK_GOTO(rc, error);
        tree = NULL;
        break;
    case LYD_ANYDATA_STRING:
    case LYD_ANYDATA_XML:
    case LYD_ANYDATA_JSON:
        /* string value */
        rc = lyb_read_string(&value, lybctx->parse_ctx);
        LY_CHECK_GOTO(rc, error);

        /* use the string value */
        rc = lyd_create_any(snode, value, value_type, 1, &node);
        LY_CHECK_GOTO(rc, error);
        value = NULL;
        break;
    default:
        LOGINT(ctx);
        rc = LY_EINT;
        goto error;
    }

    assert(node);
    LOG_LOCSET(NULL, node);

    /* register parsed anydata node */
    lyb_finish_node(lybctx, parent, flags, &meta, &node, first_p, parsed);

    LOG_LOCBACK(0, 1);
    return LY_SUCCESS;

error:
    lyd_free_meta_siblings(meta);
    lyd_free_siblings(tree);
    free(value);
    lyd_free_tree(node);
    return rc;
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
    LY_ERR rc;
    struct lyd_node *node = NULL;
    struct lyd_meta *meta = NULL;
    uint32_t flags = 0;

    /* read necessary basic data */
    rc = lyb_parse_node_header(lybctx, snode, &flags, &meta);
    LY_CHECK_GOTO(rc, error);

    /* create node */
    rc = lyd_create_inner(snode, &node);
    LY_CHECK_GOTO(rc, error);

    assert(node);
    LOG_LOCSET(NULL, node);

    /* process children */
    rc = lyb_parse_siblings(lybctx, node, 0, NULL, NULL);
    LY_CHECK_GOTO(rc, error);

    /* additional procedure for inner node */
    rc = lyb_validate_node_inner(lybctx, node);
    LY_CHECK_GOTO(rc, error);

    if (snode->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
        /* rememeber the RPC/action/notification */
        lybctx->op_node = node;
    }

    /* register parsed node */
    lyb_finish_node(lybctx, parent, flags, &meta, &node, first_p, parsed);

    LOG_LOCBACK(0, 1);
    return LY_SUCCESS;

error:
    if (node) {
        LOG_LOCBACK(0, 1);
    }
    lyd_free_meta_siblings(meta);
    lyd_free_tree(node);
    return rc;
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
    uint32_t flags = 0;

    /* read necessary basic data */
    ret = lyb_parse_node_header(lybctx, snode, &flags, &meta);
    LY_CHECK_GOTO(ret, error);

    /* read value of term node and create it */
    ret = lyb_create_term(lybctx, snode, &node);
    LY_CHECK_GOTO(ret, error);

    assert(node);
    LOG_LOCSET(NULL, node);

    lyb_finish_node(lybctx, parent, flags, &meta, &node, first_p, parsed);

    LOG_LOCBACK(0, 1);
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
    uint8_t peek;

    while (1) {
        /* peek for the end instance flag (special metadata count) */
        peek = 0;
        lyb_peek(&peek, LYB_METADATA_END_BITS, lybctx->parse_ctx);
        if (peek == LYB_METADATA_END) {
            /* all the instances parsed, read the end flag */
            lyb_read(&peek, LYB_METADATA_END_BITS, lybctx->parse_ctx);
            break;
        }

        /* next instance */
        LY_CHECK_RET(lyb_parse_node_leaf(lybctx, parent, snode, first_p, parsed));
    }

    return LY_SUCCESS;
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
    LY_ERR rc;
    uint8_t peek;
    struct lyd_node *node = NULL;
    struct lyd_meta *meta = NULL;
    uint32_t flags = 0;
    ly_bool log_node = 0;

    while (1) {
        /* peek for the end instance flag (special metadata count) */
        peek = 0;
        lyb_peek(&peek, LYB_METADATA_END_BITS, lybctx->parse_ctx);
        if (peek == LYB_METADATA_END) {
            /* all the instances parsed, read the end flag */
            lyb_read(&peek, LYB_METADATA_END_BITS, lybctx->parse_ctx);
            break;
        }

        /* read necessary basic data */
        rc = lyb_parse_node_header(lybctx, snode, &flags, &meta);
        LY_CHECK_GOTO(rc, error);

        /* create list node */
        rc = lyd_create_inner(snode, &node);
        LY_CHECK_GOTO(rc, error);

        assert(node);
        LOG_LOCSET(NULL, node);
        log_node = 1;

        /* process children */
        rc = lyb_parse_siblings(lybctx, node, 0, NULL, NULL);
        LY_CHECK_GOTO(rc, error);

        /* additional procedure for inner node */
        rc = lyb_validate_node_inner(lybctx, node);
        LY_CHECK_GOTO(rc, error);

        if (snode->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)) {
            /* rememeber the RPC/action/notification */
            lybctx->op_node = node;
        }

        /* register parsed list node */
        lyb_finish_node(lybctx, parent, flags, &meta, &node, first_p, parsed);

        LOG_LOCBACK(0, 1);
        log_node = 0;
    }

    return LY_SUCCESS;

error:
    if (log_node) {
        LOG_LOCBACK(0, 1);
    }
    lyd_free_meta_siblings(meta);
    lyd_free_tree(node);
    return rc;
}

/**
 * @brief Parse a node.
 *
 * @param[in] lybctx LYB context.
 * @param[in] parent Data parent of the sibling, must be set if @p first_p is not.
 * @param[in,out] first_p First top-level sibling, must be set if @p parent is not.
 * @param[in,out] parsed Set of all successfully parsed nodes to add to.
 * @return LY_ENOT if the last sibling has been parsed;
 * @return LY_ERR value on error.
 */
static LY_ERR
lyb_parse_node(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, struct lyd_node **first_p,
        struct ly_set *parsed)
{
    LY_ERR rc = LY_SUCCESS;
    const struct lysc_node *snode;
    const struct lys_module *mod;
    enum lylyb_node_type lyb_type = 0;
    char *mod_name = NULL, mod_rev[LY_REV_SIZE];

    /* read node type */
    lyb_read((uint64_t *)&lyb_type, LYB_NODE_TYPE_BITS, lybctx->parse_ctx);

    switch (lyb_type) {
    case LYB_NODE_END:
        /* no more siblings */
        rc = LY_ENOT;
        goto cleanup;
    case LYB_NODE_TOP:
        /* top-level, read module */
        LY_CHECK_GOTO(rc = lyb_parse_module_idx(lybctx->parse_ctx, &mod), cleanup);

        /* read hash, find the schema node starting from mod */
        LY_CHECK_GOTO(rc = lyb_parse_schema_hash(lybctx, NULL, mod, &snode), cleanup);
        break;
    case LYB_NODE_CHILD:
        /* read hash, find the schema node starting from parent schema, if any */
        LY_CHECK_GOTO(rc = lyb_parse_schema_hash(lybctx, lyd_parser_node_schema(parent), NULL, &snode), cleanup);
        break;
    case LYB_NODE_EXT:
        /* ext, read module name */
        LY_CHECK_GOTO(rc = lyb_parse_module(lybctx->parse_ctx, &mod_name, mod_rev), cleanup);

        /* read schema node name, find the nested ext schema node */
        LY_CHECK_GOTO(rc = lyb_parse_schema_nested_ext(lybctx, parent, mod_name, &snode), cleanup);
        break;
    }

    if (!snode) {
        rc = lyb_parse_node_opaq(lybctx, parent, first_p, parsed);
    } else if (snode->nodetype & LYS_LEAFLIST) {
        rc = lyb_parse_node_leaflist(lybctx, parent, snode, first_p, parsed);
    } else if (snode->nodetype == LYS_LIST) {
        rc = lyb_parse_node_list(lybctx, parent, snode, first_p, parsed);
    } else if (snode->nodetype & LYD_NODE_ANY) {
        rc = lyb_parse_node_any(lybctx, parent, snode, first_p, parsed);
    } else if (snode->nodetype & LYD_NODE_INNER) {
        rc = lyb_parse_node_inner(lybctx, parent, snode, first_p, parsed);
    } else {
        rc = lyb_parse_node_leaf(lybctx, parent, snode, first_p, parsed);
    }
    LY_CHECK_GOTO(rc, cleanup);

cleanup:
    free(mod_name);
    return rc;
}

/**
 * @brief Parse siblings (@ref lyb_print_siblings()).
 *
 * @param[in] lybctx LYB context.
 * @param[in] parent Data parent of the sibling, must be set if @p first_p is not.
 * @param[in] is_root Whether we are parsing the root node or not.
 * @param[in,out] first_p First top-level sibling, must be set if @p parent is not.
 * @param[out] parsed Set of all successfully parsed nodes.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_siblings(struct lyd_lyb_ctx *lybctx, struct lyd_node *parent, ly_bool is_root, struct lyd_node **first_p,
        struct ly_set *parsed)
{
    LY_ERR r;

    while (!(r = lyb_parse_node(lybctx, parent, first_p, parsed))) {
        /* parsing some data, hash must be set */
        assert(!lybctx->parse_ctx->empty_hash);

        if (is_root && !(lybctx->int_opts & LYD_INTOPT_WITH_SIBLINGS)) {
            break;
        }
    }
    LY_CHECK_RET(r != LY_ENOT, r);

    return LY_SUCCESS;
}

/**
 * @brief Parse LYB header.
 *
 * @param[in] lybctx LYB context.
 * @return LY_ERR value.
 */
static LY_ERR
lyb_parse_header(struct lylyb_parse_ctx *lybctx)
{
    uint8_t byte;
    uint32_t data_hash, cur_hash;

    /* version */
    byte = 0;
    lyb_read(&byte, LYB_HEADER_VERSION_BITS, lybctx);
    if (byte != LYB_HEADER_VERSION_NUM) {
        LOGERR(lybctx->ctx, LY_EINVAL, "Invalid LYB format version \"0x%x\", expected \"0x%x\".",
                byte, LYB_HEADER_VERSION_NUM);
        return LY_EINVAL;
    }

    /* hash algorithm */
    byte = 0;
    lyb_read(&byte, LYB_HEADER_HASH_ALG_BITS, lybctx);
    if (byte != LYB_HEADER_HASH_ALG_NUM) {
        LOGERR(lybctx->ctx, LY_EINVAL, "Different LYB format hash algorithm \"0x%x\" used, expected \"0x%x\".",
                byte, LYB_HEADER_HASH_ALG_NUM);
        return LY_EINVAL;
    }

    /* shrink */
    byte = 0;
    lyb_read(&byte, LYB_HEADER_SHRINK_FLAG_BITS, lybctx);
    if (byte) {
        lybctx->shrink = 1;
    }

    /* context hash */
    data_hash = 0;
    lyb_read((uint8_t *)&data_hash, LYB_HEADER_CTX_HASH_BITS, lybctx);

    if (!data_hash) {
        /* fine for no data */
        lybctx->empty_hash = 1;
    } else {
        /* truncate context hash to the same bit size */
        cur_hash = ly_ctx_get_modules_hash(lybctx->ctx);
        cur_hash <<= (sizeof cur_hash * 8) - LYB_HEADER_CTX_HASH_BITS;
        cur_hash >>= (sizeof cur_hash * 8) - LYB_HEADER_CTX_HASH_BITS;

        if (data_hash != cur_hash) {
            LOGERR(lybctx->ctx, LY_EINVAL, "Different current LYB context modules hash compared to the one stored in the "
                    "LYB data (0x%x != 0x%x).", data_hash, cur_hash);
            return LY_EINVAL;
        }
    }

    return LY_SUCCESS;
}

LY_ERR
lyd_parse_lyb(const struct ly_ctx *ctx, const struct lysc_ext_instance *ext, struct lyd_node *parent,
        struct lyd_node **first_p, struct ly_in *in, uint32_t parse_opts, uint32_t val_opts, uint32_t int_opts,
        struct ly_set *parsed, ly_bool *subtree_sibling, struct lyd_ctx **lydctx_p)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyd_lyb_ctx *lybctx;

    assert(!(parse_opts & ~LYD_PARSE_OPTS_MASK));
    assert(!(val_opts & ~LYD_VALIDATE_OPTS_MASK));

    LY_CHECK_ARG_RET(ctx, !(parse_opts & LYD_PARSE_SUBTREE), LY_EINVAL);

    if (!(ctx->opts & LY_CTX_LYB_HASHES)) {
        /* generate LYB hashes */
        LY_CHECK_GOTO(rc = ly_ctx_set_options((struct ly_ctx *)ctx, LY_CTX_LYB_HASHES), cleanup);
    }

    if (subtree_sibling) {
        *subtree_sibling = 0;
    }

    lybctx = calloc(1, sizeof *lybctx);
    LY_CHECK_ERR_RET(!lybctx, LOGMEM(ctx), LY_EMEM);
    lybctx->parse_ctx = calloc(1, sizeof *lybctx->parse_ctx);
    LY_CHECK_ERR_RET(!lybctx->parse_ctx, LOGMEM(ctx), LY_EMEM);

    lybctx->parse_ctx->in = in;
    lybctx->parse_ctx->ctx = ctx;
    lybctx->parse_opts = parse_opts;
    lybctx->val_opts = val_opts;
    lybctx->int_opts = int_opts;
    lybctx->free = lyb_parse_ctx_free;
    lybctx->ext = ext;

    /* find the operation node if it exists already */
    LY_CHECK_GOTO(rc = lyd_parser_find_operation(parent, int_opts, &lybctx->op_node), cleanup);

    /* read header */
    rc = lyb_parse_header(lybctx->parse_ctx);
    LY_CHECK_GOTO(rc, cleanup);

    /* read sibling(s) */
    rc = lyb_parse_siblings(lybctx, parent, 1, first_p, parsed);
    LY_CHECK_GOTO(rc, cleanup);

    if ((int_opts & (LYD_INTOPT_RPC | LYD_INTOPT_ACTION | LYD_INTOPT_NOTIF | LYD_INTOPT_REPLY)) && !lybctx->op_node) {
        LOGVAL(ctx, LYVE_DATA, "Missing the operation node.");
        rc = LY_EVALID;
        goto cleanup;
    }

cleanup:
    /* there should be no unres stored if validation should be skipped */
    assert(!(parse_opts & LYD_PARSE_ONLY) || (!lybctx->node_types.count && !lybctx->meta_types.count &&
            !lybctx->node_when.count));

    if (rc) {
        lyb_parse_ctx_free((struct lyd_ctx *)lybctx);
    } else {
        *lydctx_p = (struct lyd_ctx *)lybctx;
    }
    return rc;
}
