/**
 * @file printer_context.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Compiled context printer
 *
 * Copyright (c) 2024 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "hash_table_internal.h"
#include "log.h"
#include "ly_common.h"
#include "plugins_exts.h"
#include "tree_schema_internal.h"
#include "xpath.h"

/**
 * @brief Get the size of a sized array.
 *
 * @param[in] ARRAY Sized array.
 * @return Size of @p ARRAY.
 */
#define CTXS_SIZED_ARRAY(ARRAY) \
        (ARRAY ? sizeof(LY_ARRAY_COUNT_TYPE) : 0) + LY_ARRAY_COUNT(ARRAY) * sizeof *ARRAY

/**
 * @brief Print (serialize) a sized array.
 *
 * @param[in] ORIG_ARRAY Sized array to print.
 * @param[in,out] ARRAY Printed sized array.
 * @param[in,out] MEM Memory to use.
 */
#define CTXP_SIZED_ARRAY(ORIG_ARRAY, ARRAY, MEM) \
        if (ORIG_ARRAY) { \
            LY_ARRAY_COUNT_TYPE count = LY_ARRAY_COUNT(ORIG_ARRAY); \
            memcpy(*MEM, &count, sizeof count); \
            *MEM = (char *)*MEM + sizeof count; \
            \
            ARRAY = *MEM; \
            *MEM = (char *)*MEM + count * sizeof *ORIG_ARRAY; \
        } else { \
            ARRAY = NULL; \
        }

/**
 * @brief Print a structure that is optional (may be NULL).
 *
 * @param[in] FUNC Function to call to fill the structure.
 * @param[in] ORIG_PTR Pointer to the structure to print.
 * @param[in] PTR Pointer to the printed structure.
 * @param[in] ADDR_HT addr_ht.
 * @param[in] PTR_SET ptr_set.
 * @param[in,out] MEM mem.
 */
#define CTXP_OPTIONAL_STRUCT(FUNC, ORIG_PTR, PTR, ADDR_HT, PTR_SET, MEM) \
        if (ORIG_PTR) { \
            PTR = *MEM; \
            *MEM = (char *)*MEM + sizeof *PTR; \
            FUNC(ORIG_PTR, PTR, ADDR_HT, PTR_SET, MEM); \
        } else { \
            PTR = NULL; \
        }

/**
 * @brief Alignment of all the printed data, ensures all memory access is aligned to this number (B)
 */
#define CTXP_MEM_ALIGN 8

/**
 * @brief Adjust data size to an aligned size to make sure the following data is aligned.
 *
 * @param[in,out] SIZE Data size that is adjusted.
 */
#define CTXP_MEM_SIZE(SIZE) ((SIZE) + ((~(SIZE) + 1) & (CTXP_MEM_ALIGN - 1)))

static void
ctxs_dict_ht(const struct ly_ht *ht, int *size)
{
    uint32_t i, j;
    struct ly_ht_rec *rec;
    struct ly_dict_rec *dict_rec;

    /* hash table */
    *size += sizeof *ht;

    /* hlists */
    *size += ht->size * sizeof *ht->hlists;

    /* records (with string pointers) */
    *size += ht->size * ht->rec_size;

    LYHT_ITER_ALL_RECS(ht, i, j, rec) {
        dict_rec = (struct ly_dict_rec *)&rec->val;

        /* strings */
        *size += CTXP_MEM_SIZE(strlen(dict_rec->value) + 1);
    }
}

static void
ctxs_exts(const struct lysc_ext_instance *exts, struct ly_ht *ht, int *size)
{
    LY_ARRAY_COUNT_TYPE u;

    /* sized array */
    *size += CTXS_SIZED_ARRAY(exts);

    LY_ARRAY_FOR(exts, u) {
        ctxs_exts(exts[u].exts, ht, size);

        /* substmts */
        *size += sizeof(LY_ARRAY_COUNT_TYPE) + LY_ARRAY_COUNT(exts[u].substmts) * sizeof *exts[u].substmts;

        /* compiled, substmts storage */
        if (exts[u].def->plugin && exts[u].def->plugin->compiled_size) {
            *size += exts[u].def->plugin->compiled_size(&exts[u], ht);
        }
    }
}

static void
ctxs_prefixes(const struct lysc_prefix *prefixes, int *size)
{
    LY_ARRAY_COUNT_TYPE u;

    *size += CTXS_SIZED_ARRAY(prefixes);
    LY_ARRAY_FOR(prefixes, u) {
        /* string not in the dictionary */
        if (prefixes[u].prefix) {
            *size += CTXP_MEM_SIZE(strlen(prefixes[u].prefix) + 1);
        }
    }
}

static void
ctxs_expr(const struct lyxp_expr *exp, int *size)
{
    uint32_t i, j;

    *size += sizeof *exp;

    *size += exp->used * sizeof *exp->tokens;
    *size += exp->used * sizeof *exp->tok_pos;
    *size += exp->used * sizeof *exp->tok_len;
    *size += exp->used * sizeof *exp->repeat;
    for (i = 0; i < exp->used; ++i) {
        if (!exp->repeat[i]) {
            continue;
        }

        for (j = 0; exp->repeat[j]; ++j) {}
        *size += (j + 1) * sizeof **exp->repeat;
    }
}

static void
ctxs_musts(const struct lysc_must *musts, struct ly_ht *ht, int *size)
{
    LY_ARRAY_COUNT_TYPE u;

    *size += CTXS_SIZED_ARRAY(musts);
    LY_ARRAY_FOR(musts, u) {
        ctxs_expr(musts[u].cond, size);
        ctxs_prefixes(musts[u].prefixes, size);
        ctxs_exts(musts[u].exts, ht, size);
    }
}

static void
ctxs_when(const struct lysc_when *when, struct ly_ht *ht, int *size)
{
    uint32_t hash;

    /* ht check, make sure the structure is stored only once */
    hash = lyht_hash((const char *)&when, sizeof when);
    if (lyht_insert(ht, &when, hash, NULL) == LY_EEXIST) {
        return;
    }

    *size += sizeof *when;

    ctxs_expr(when->cond, size);
    ctxs_prefixes(when->prefixes, size);
    ctxs_exts(when->exts, ht, size);
}

static void
ctxs_whens(const struct lysc_when **whens, struct ly_ht *ht, int *size)
{
    LY_ARRAY_COUNT_TYPE u;

    *size += CTXS_SIZED_ARRAY(whens);
    LY_ARRAY_FOR(whens, u) {
        ctxs_when(whens[u], ht, size);
    }
}

static void
ctxs_range(const struct lysc_range *range, struct ly_ht *ht, int *size)
{
    if (!range) {
        return;
    }

    *size += sizeof *range;

    *size += CTXS_SIZED_ARRAY(range->parts);
    ctxs_exts(range->exts, ht, size);
}

static void
ctxs_patterns(const struct lysc_pattern **patterns, struct ly_ht *ht, int *size)
{
    uint32_t hash;
    size_t code_size;
    LY_ARRAY_COUNT_TYPE u;

    *size += CTXS_SIZED_ARRAY(patterns);
    LY_ARRAY_FOR(patterns, u) {
        /* ht check, make sure the structure is stored only once */
        hash = lyht_hash((const char *)patterns[u], sizeof *patterns);
        if (lyht_insert(ht, (void *)patterns[u], hash, NULL) == LY_EEXIST) {
            continue;
        }

        *size += sizeof *patterns[u];

        pcre2_pattern_info(patterns[u]->code, PCRE2_INFO_SIZE, &code_size);
        *size += CTXP_MEM_SIZE(code_size);
        ctxs_exts(patterns[u]->exts, ht, size);
    }
}

static void
ctxs_enums(const struct lysc_type_bitenum_item *enums, struct ly_ht *ht, int *size)
{
    LY_ARRAY_COUNT_TYPE u;

    *size += CTXS_SIZED_ARRAY(enums);
    LY_ARRAY_FOR(enums, u) {
        ctxs_exts(enums[u].exts, ht, size);
    }
}

static void
ctxs_type(const struct lysc_type *type, struct ly_ht *ht, int *size)
{
    uint32_t hash;
    const struct lysc_type_num *type_num;
    const struct lysc_type_dec *type_dec;
    const struct lysc_type_str *type_str;
    const struct lysc_type_enum *type_enum_bits;
    const struct lysc_type_leafref *type_lref;
    const struct lysc_type_identityref *type_identref;
    const struct lysc_type_instanceid *type_instid;
    const struct lysc_type_union *type_union;
    const struct lysc_type_bin *type_bin;
    LY_ARRAY_COUNT_TYPE u;

    /* ht check, make sure the structure is stored only once */
    hash = lyht_hash((const char *)&type, sizeof type);
    if (lyht_insert(ht, &type, hash, NULL) == LY_EEXIST) {
        return;
    }

    /* common members */
    ctxs_exts(type->exts, ht, size);

    switch (type->basetype) {
    case LY_TYPE_BINARY:
        type_bin = (const struct lysc_type_bin *)type;
        *size += sizeof *type_bin;

        ctxs_range(type_bin->length, ht, size);
        break;
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
        type_num = (const struct lysc_type_num *)type;
        *size += sizeof *type_num;

        ctxs_range(type_num->range, ht, size);
        break;
    case LY_TYPE_STRING:
        type_str = (const struct lysc_type_str *)type;
        *size += sizeof *type_str;

        ctxs_range(type_str->length, ht, size);
        ctxs_patterns((const struct lysc_pattern **)type_str->patterns, ht, size);
        break;
    case LY_TYPE_BITS:
    case LY_TYPE_ENUM:
        type_enum_bits = (const struct lysc_type_enum *)type;
        *size += sizeof *type_enum_bits;

        ctxs_enums(type_enum_bits->enums, ht, size);
        break;
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
        *size += sizeof *type;
        break;
    case LY_TYPE_DEC64:
        type_dec = (const struct lysc_type_dec *)type;
        *size += sizeof *type_dec;

        ctxs_range(type_dec->range, ht, size);
        break;
    case LY_TYPE_IDENT:
        type_identref = (const struct lysc_type_identityref *)type;
        *size += sizeof *type_identref;

        *size += CTXS_SIZED_ARRAY(type_identref->bases);
        break;
    case LY_TYPE_INST:
        type_instid = (const struct lysc_type_instanceid *)type;
        *size += sizeof *type_instid;
        break;
    case LY_TYPE_LEAFREF:
        type_lref = (const struct lysc_type_leafref *)type;
        *size += sizeof *type_lref;

        ctxs_expr(type_lref->path, size);
        ctxs_prefixes(type_lref->prefixes, size);
        ctxs_type(type_lref->realtype, ht, size);
        break;
    case LY_TYPE_UNION:
        type_union = (const struct lysc_type_union *)type;
        *size += sizeof *type_union;

        *size += CTXS_SIZED_ARRAY(type_union->types);
        LY_ARRAY_FOR(type_union->types, u) {
            ctxs_type(type_union->types[u], ht, size);
        }
        break;
    case LY_TYPE_UNKNOWN:
        LOGINT(NULL);
        break;
    }
}

static void
ctxs_node(const struct lysc_node *node, struct ly_ht *ht, int *size)
{
    const struct lysc_node_container *cont;
    const struct lysc_node_choice *choic;
    const struct lysc_node_leaf *leaf;
    const struct lysc_node_leaflist *llist;
    const struct lysc_node_list *list;
    const struct lysc_node_anydata *any;
    const struct lysc_node_case *cas;
    const struct lysc_node_action *act;
    const struct lysc_node_notif *notif;
    const struct lysc_node *child;
    LY_ARRAY_COUNT_TYPE u;

    /* common members */
    ctxs_exts(node->exts, ht, size);

    switch (node->nodetype) {
    case LYS_CONTAINER:
        cont = (const struct lysc_node_container *)node;
        *size += sizeof *cont;

        LY_LIST_FOR(cont->child, child) {
            ctxs_node(child, ht, size);
        }
        ctxs_musts(cont->musts, ht, size);
        ctxs_whens((const struct lysc_when **)cont->when, ht, size);
        LY_LIST_FOR((const struct lysc_node *)cont->actions, child) {
            ctxs_node(child, ht, size);
        }
        LY_LIST_FOR((const struct lysc_node *)cont->notifs, child) {
            ctxs_node(child, ht, size);
        }
        break;
    case LYS_CHOICE:
        choic = (const struct lysc_node_choice *)node;
        *size += sizeof *choic;

        LY_LIST_FOR((const struct lysc_node *)choic->cases, child) {
            ctxs_node(child, ht, size);
        }
        ctxs_whens((const struct lysc_when **)choic->when, ht, size);
        break;
    case LYS_LEAF:
        leaf = (const struct lysc_node_leaf *)node;
        *size += sizeof *leaf;

        ctxs_musts(leaf->musts, ht, size);
        ctxs_whens((const struct lysc_when **)leaf->when, ht, size);
        ctxs_type(leaf->type, ht, size);
        ctxs_prefixes(leaf->dflt.prefixes, size);
        break;
    case LYS_LEAFLIST:
        llist = (const struct lysc_node_leaflist *)node;
        *size += sizeof *llist;

        ctxs_musts(llist->musts, ht, size);
        ctxs_whens((const struct lysc_when **)llist->when, ht, size);
        ctxs_type(llist->type, ht, size);
        *size += CTXS_SIZED_ARRAY(llist->dflts);
        LY_ARRAY_FOR(llist->dflts, u) {
            ctxs_prefixes(llist->dflts[u].prefixes, size);
        }
        break;
    case LYS_LIST:
        list = (const struct lysc_node_list *)node;
        *size += sizeof *list;

        LY_LIST_FOR(list->child, child) {
            ctxs_node(child, ht, size);
        }
        ctxs_musts(list->musts, ht, size);
        ctxs_whens((const struct lysc_when **)list->when, ht, size);
        LY_LIST_FOR((const struct lysc_node *)list->actions, child) {
            ctxs_node(child, ht, size);
        }
        LY_LIST_FOR((const struct lysc_node *)list->notifs, child) {
            ctxs_node(child, ht, size);
        }
        *size += CTXS_SIZED_ARRAY(list->uniques);
        LY_ARRAY_FOR(list->uniques, u) {
            *size += sizeof(LY_ARRAY_COUNT_TYPE) + LY_ARRAY_COUNT(list->uniques[u]) * sizeof **list->uniques;
        }
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        any = (const struct lysc_node_anydata *)node;
        *size += sizeof *any;

        ctxs_musts(any->musts, ht, size);
        ctxs_whens((const struct lysc_when **)any->when, ht, size);
        break;
    case LYS_CASE:
        cas = (const struct lysc_node_case *)node;
        *size += sizeof *cas;

        LY_LIST_FOR(cas->child, child) {
            ctxs_node(child, ht, size);
        }
        ctxs_whens((const struct lysc_when **)cas->when, ht, size);
        break;
    case LYS_RPC:
    case LYS_ACTION:
        act = (const struct lysc_node_action *)node;
        *size += sizeof *act;

        ctxs_whens((const struct lysc_when **)act->when, ht, size);
        LY_LIST_FOR(act->input.child, child) {
            ctxs_node(child, ht, size);
        }
        ctxs_musts(act->input.musts, ht, size);
        LY_LIST_FOR(act->output.child, child) {
            ctxs_node(child, ht, size);
        }
        ctxs_musts(act->output.musts, ht, size);
        break;
    case LYS_NOTIF:
        notif = (const struct lysc_node_notif *)node;
        *size += sizeof *notif;

        LY_LIST_FOR(notif->child, child) {
            ctxs_node(child, ht, size);
        }
        ctxs_musts(notif->musts, ht, size);
        ctxs_whens((const struct lysc_when **)notif->when, ht, size);
        break;
    default:
        LOGINT(NULL);
        break;
    }
}

static void
ctxs_compiled(const struct lysc_module *compiled, struct ly_ht *ht, int *size)
{
    const struct lysc_node *node;

    if (!compiled) {
        return;
    }

    /* compiled module */
    *size += sizeof *compiled;

    *size += CTXS_SIZED_ARRAY(compiled->features);
    LY_LIST_FOR(compiled->data, node) {
        ctxs_node(node, ht, size);
    }
    LY_LIST_FOR((const struct lysc_node *)compiled->rpcs, node) {
        ctxs_node(node, ht, size);
    }
    LY_LIST_FOR((const struct lysc_node *)compiled->notifs, node) {
        ctxs_node(node, ht, size);
    }
    ctxs_exts(compiled->exts, ht, size);
}

static void
ctxs_extensions(const struct lysc_ext *extensions, struct ly_ht *ht, int *size)
{
    LY_ARRAY_COUNT_TYPE u;

    /* sized array */
    *size += CTXS_SIZED_ARRAY(extensions);

    LY_ARRAY_FOR(extensions, u) {
        ctxs_exts(extensions[u].exts, ht, size);
    }
}

static void
ctxs_identities(const struct lysc_ident *identities, struct ly_ht *ht, int *size)
{
    LY_ARRAY_COUNT_TYPE u;

    /* sized array */
    *size += CTXS_SIZED_ARRAY(identities);

    LY_ARRAY_FOR(identities, u) {
        *size += CTXS_SIZED_ARRAY(identities[u].derived);
        ctxs_exts(identities[u].exts, ht, size);
    }
}

static void
ctxs_module(const struct lys_module *mod, struct ly_ht *ht, int *size)
{
    /* module */
    *size += sizeof *mod;

    /* compiled module */
    ctxs_compiled(mod->compiled, ht, size);

    /* extensions, identities, submodules */
    ctxs_extensions(mod->extensions, ht, size);
    ctxs_identities(mod->identities, ht, size);
    *size += CTXS_SIZED_ARRAY(mod->submodules);

    /* augmented_by, deviated_by */
    *size += CTXS_SIZED_ARRAY(mod->augmented_by);
    *size += CTXS_SIZED_ARRAY(mod->deviated_by);
}

void
ly_ctx_compiled_size_context(const struct ly_ctx *ctx, struct ly_ht *addr_ht, int *size)
{
    uint32_t i;
    const struct lys_module *mod;

    /* context */
    *size += sizeof *ctx;

    /* dictionary ht (with all the strings) */
    ctxs_dict_ht(ctx->dict.hash_tab, size);

    /* module set */
    *size += ctx->modules.count * sizeof ctx->modules.objs;
    for (i = 0; i < ctx->modules.count; ++i) {
        mod = ctx->modules.objs[i];

        /* modules */
        ctxs_module(mod, addr_ht, size);
    }

    /* plugins sets */
    *size += ctx->plugins_types.count * sizeof ctx->plugins_types.objs;
    *size += ctx->plugins_extensions.count * sizeof ctx->plugins_extensions.objs;
}

int
ly_ctx_compiled_ext_stmts_storage_size(const struct lysc_ext_substmt *substmts, struct ly_ht *addr_ht)
{
    LY_ARRAY_COUNT_TYPE u;
    int size = 0;
    uint32_t hash;
    const struct lysc_node *node;

    LY_ARRAY_FOR(substmts, u) {
        switch (substmts[u].stmt) {
        case LY_STMT_NOTIFICATION:
        case LY_STMT_INPUT:
        case LY_STMT_OUTPUT:
        case LY_STMT_ACTION:
        case LY_STMT_RPC:
        case LY_STMT_ANYDATA:
        case LY_STMT_ANYXML:
        case LY_STMT_CASE:
        case LY_STMT_CHOICE:
        case LY_STMT_CONTAINER:
        case LY_STMT_LEAF:
        case LY_STMT_LEAF_LIST:
        case LY_STMT_LIST:
        case LY_STMT_USES:
            node = *(const struct lysc_node **)substmts[u].storage_p;

            /* ht check, make sure the node list is stored only once */
            hash = lyht_hash((const char *)&node, sizeof node);
            if (lyht_insert(addr_ht, &node, hash, NULL) == LY_EEXIST) {
                break;
            }

            LY_LIST_FOR(node, node) {
                ctxs_node(node, addr_ht, &size);
            }
            break;
        case LY_STMT_ARGUMENT:
        case LY_STMT_CONTACT:
        case LY_STMT_DESCRIPTION:
        case LY_STMT_ERROR_APP_TAG:
        case LY_STMT_ERROR_MESSAGE:
        case LY_STMT_KEY:
        case LY_STMT_MODIFIER:
        case LY_STMT_NAMESPACE:
        case LY_STMT_ORGANIZATION:
        case LY_STMT_PRESENCE:
        case LY_STMT_REFERENCE:
        case LY_STMT_UNITS:
            /* string, in the dictionary */
            break;
        case LY_STMT_BIT:
        case LY_STMT_ENUM:
            ctxs_enums(*substmts[u].storage_p, addr_ht, &size);
            break;
        case LY_STMT_CONFIG:
        case LY_STMT_MANDATORY:
        case LY_STMT_ORDERED_BY:
        case LY_STMT_STATUS:
        case LY_STMT_FRACTION_DIGITS:
        case LY_STMT_REQUIRE_INSTANCE:
        case LY_STMT_MAX_ELEMENTS:
        case LY_STMT_MIN_ELEMENTS:
        case LY_STMT_POSITION:
        case LY_STMT_VALUE:
            /* number stored directly */
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            ctxs_exts(*substmts[u].storage_p, addr_ht, &size);
            break;
        case LY_STMT_IDENTITY:
            ctxs_identities(*substmts[u].storage_p, addr_ht, &size);
            break;
        case LY_STMT_LENGTH:
        case LY_STMT_RANGE:
            ctxs_range(*substmts[u].storage_p, addr_ht, &size);
            break;
        case LY_STMT_MUST:
            ctxs_musts(*substmts[u].storage_p, addr_ht, &size);
            break;
        case LY_STMT_PATTERN:
            ctxs_patterns(*substmts[u].storage_p, addr_ht, &size);
            break;
        case LY_STMT_TYPE:
            ctxs_type(*substmts[u].storage_p, addr_ht, &size);
            break;
        case LY_STMT_WHEN:
            ctxs_when(*substmts[u].storage_p, addr_ht, &size);
            break;
        case LY_STMT_IF_FEATURE:
            /* not compiled, but can be a substatement */
            break;
        case LY_STMT_NONE:
        case LY_STMT_AUGMENT:
        case LY_STMT_GROUPING:
        case LY_STMT_BASE:
        case LY_STMT_BELONGS_TO:
        case LY_STMT_DEFAULT:
        case LY_STMT_DEVIATE:
        case LY_STMT_DEVIATION:
        case LY_STMT_EXTENSION:
        case LY_STMT_FEATURE:
        case LY_STMT_IMPORT:
        case LY_STMT_INCLUDE:
        case LY_STMT_MODULE:
        case LY_STMT_PATH:
        case LY_STMT_PREFIX:
        case LY_STMT_REFINE:
        case LY_STMT_REVISION:
        case LY_STMT_REVISION_DATE:
        case LY_STMT_SUBMODULE:
        case LY_STMT_TYPEDEF:
        case LY_STMT_UNIQUE:
        case LY_STMT_YANG_VERSION:
        case LY_STMT_YIN_ELEMENT:
        case LY_STMT_SYNTAX_SEMICOLON:
        case LY_STMT_SYNTAX_LEFT_BRACE:
        case LY_STMT_SYNTAX_RIGHT_BRACE:
        case LY_STMT_ARG_TEXT:
        case LY_STMT_ARG_VALUE:
            /* not compiled, unreachable */
            LOGINT(NULL);
            size = -1;
            goto cleanup;
        }
    }

cleanup:
    return size;
}

static void ctxp_node(const struct lysc_node *orig_node, struct lysc_node *node, struct ly_ht *addr_ht,
        struct ly_set *ptr_set, void **mem);

static void
ctxp_mutex(pthread_mutex_t *mutex)
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

static void
ctxp_set(const struct ly_set *orig_set, struct ly_set *set, void **mem)
{
    set->size = orig_set->count;
    set->count = orig_set->count;
    set->objs = *mem;
    *mem = (char *)*mem + (set->count * sizeof set->objs);
}

static void
ctxp_dict_ht(const struct ly_ht *orig_ht, struct ly_ht *ht, struct ly_ht *addr_ht, void **mem)
{
    uint32_t i, j;
    struct ly_ht_rec *rec;
    struct ly_dict_rec *dict_rec;
    int len;
    void *orig_addr;

    /* hash table, must not be modified in the context */
    *ht = *orig_ht;
    ht->val_equal = NULL;
    ht->cb_data = NULL;

    /* hlists */
    ht->hlists = *mem;
    *mem = (char *)*mem + ht->size * sizeof *ht->hlists;

    memcpy(ht->hlists, orig_ht->hlists, ht->size * sizeof *ht->hlists);

    /* records */
    ht->recs = *mem;
    *mem = (char *)*mem + ht->size * ht->rec_size;

    memcpy(ht->recs, orig_ht->recs, ht->size * ht->rec_size);

    LYHT_ITER_ALL_RECS(ht, i, j, rec) {
        dict_rec = (struct ly_dict_rec *)&rec->val;

        /* strings */
        len = strlen(dict_rec->value) + 1;
        orig_addr = dict_rec->value;

        dict_rec->value = *mem;
        *mem = (char *)*mem + CTXP_MEM_SIZE(len);

        memcpy(dict_rec->value, orig_addr, len);

        /* shared */
        ly_ctx_compiled_addr_ht_add(addr_ht, orig_addr, dict_rec->value);
    }
}

static void
ctxp_ext(const struct lysc_ext_instance *orig_ext, struct lysc_ext_instance *ext, struct ly_ht *addr_ht,
        struct ly_set *ptr_set, void **mem)
{
    LY_ARRAY_COUNT_TYPE u;

    if (orig_ext->exts) {
        /* may be referenced in the parent */
        ly_ctx_compiled_addr_ht_add(addr_ht, orig_ext, ext);
    }

    /* def, argument, module, only a reference */
    ext->def = orig_ext->def;
    ly_set_add(ptr_set, &ext->def, 1, NULL);
    ext->argument = ly_ctx_compiled_addr_ht_get(addr_ht, orig_ext->argument, 0);
    ext->module = orig_ext->module;
    ly_set_add(ptr_set, &ext->module, 1, NULL);

    /* exts */
    CTXP_SIZED_ARRAY(orig_ext->exts, ext->exts, mem);
    LY_ARRAY_FOR(orig_ext->exts, u) {
        ctxp_ext(&orig_ext->exts[u], &ext->exts[u], addr_ht, ptr_set, mem);
    }

    /* parent, only a reference */
    ext->parent = orig_ext->parent;
    ly_set_add(ptr_set, &ext->parent, 1, NULL);
    ext->parent_stmt = orig_ext->parent_stmt;
    ext->parent_stmt_index = orig_ext->parent_stmt_index;

    /* substmts */
    CTXP_SIZED_ARRAY(orig_ext->substmts, ext->substmts, mem);
    LY_ARRAY_FOR(orig_ext->substmts, u) {
        ext->substmts[u].stmt = orig_ext->substmts[u].stmt;
        ext->substmts[u].storage_p = NULL;
    }

    /* compiled, substmts storage, use the plugin */
    if (ext->def->plugin && ext->def->plugin->compiled_print) {
        ext->def->plugin->compiled_print(orig_ext, ext, addr_ht, ptr_set, mem);
    } else {
        ext->compiled = NULL;
    }
}

static void
ctxp_expr(const struct lyxp_expr *orig_exp, struct lyxp_expr *exp, struct ly_ht *addr_ht, void **mem)
{
    uint32_t i, len;

    exp->tokens = *mem;
    *mem = (char *)*mem + orig_exp->used * sizeof *exp->tokens;
    memcpy(exp->tokens, orig_exp->tokens, orig_exp->used * sizeof *exp->tokens);

    exp->tok_pos = *mem;
    *mem = (char *)*mem + orig_exp->used * sizeof *exp->tok_pos;
    memcpy(exp->tok_pos, orig_exp->tok_pos, orig_exp->used * sizeof *exp->tok_pos);

    exp->tok_len = *mem;
    *mem = (char *)*mem + orig_exp->used * sizeof *exp->tok_len;
    memcpy(exp->tok_len, orig_exp->tok_len, orig_exp->used * sizeof *exp->tok_len);

    exp->repeat = *mem;
    *mem = (char *)*mem + orig_exp->used * sizeof *exp->repeat;
    for (i = 0; i < orig_exp->used; ++i) {
        if (orig_exp->repeat[i]) {
            for (len = 0; orig_exp->repeat[i][len]; ++len) {}
            ++len;

            exp->repeat[i] = *mem;
            *mem = (char *)*mem + len * sizeof **exp->repeat;
            memcpy(exp->repeat[i], orig_exp->repeat[i], len * sizeof **exp->repeat);
        } else {
            exp->repeat[i] = NULL;
        }
    }

    exp->used = orig_exp->used;
    exp->size = orig_exp->used;
    exp->expr = ly_ctx_compiled_addr_ht_get(addr_ht, orig_exp->expr, 0);
}

static void
ctxp_prefix(const struct lysc_prefix *orig_prefix, struct lysc_prefix *prefix, struct ly_ht *UNUSED(addr_ht),
        struct ly_set *ptr_set, void **mem)
{
    int size;

    if (orig_prefix->prefix) {
        size = strlen(orig_prefix->prefix) + 1;

        prefix->prefix = *mem;
        *mem = (char *)*mem + CTXP_MEM_SIZE(size);
        memcpy(prefix->prefix, orig_prefix->prefix, size);
    } else {
        prefix->prefix = NULL;
    }

    prefix->mod = orig_prefix->mod;
    ly_set_add(ptr_set, &prefix->mod, 1, NULL);
}

static void
ctxp_must(const struct lysc_must *orig_must, struct lysc_must *must, struct ly_ht *addr_ht, struct ly_set *ptr_set,
        void **mem)
{
    LY_ARRAY_COUNT_TYPE u;

    if (orig_must->exts) {
        /* may be referenced in the parent */
        ly_ctx_compiled_addr_ht_add(addr_ht, orig_must, must);
    }

    must->cond = *mem;
    *mem = (char *)*mem + sizeof *must->cond;
    ctxp_expr(orig_must->cond, must->cond, addr_ht, mem);

    CTXP_SIZED_ARRAY(orig_must->prefixes, must->prefixes, mem);
    LY_ARRAY_FOR(orig_must->prefixes, u) {
        ctxp_prefix(&orig_must->prefixes[u], &must->prefixes[u], addr_ht, ptr_set, mem);
    }

    must->dsc = ly_ctx_compiled_addr_ht_get(addr_ht, orig_must->dsc, 0);
    must->ref = ly_ctx_compiled_addr_ht_get(addr_ht, orig_must->ref, 0);
    must->emsg = ly_ctx_compiled_addr_ht_get(addr_ht, orig_must->emsg, 0);
    must->eapptag = ly_ctx_compiled_addr_ht_get(addr_ht, orig_must->eapptag, 0);

    CTXP_SIZED_ARRAY(orig_must->exts, must->exts, mem);
    LY_ARRAY_FOR(orig_must->exts, u) {
        ctxp_ext(&orig_must->exts[u], &must->exts[u], addr_ht, ptr_set, mem);
    }
}

static void
ctxp_when(const struct lysc_when *orig_when, struct lysc_when **when, struct ly_ht *addr_ht, struct ly_set *ptr_set,
        void **mem)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_when *w;

    /* may have already been printed */
    w = ly_ctx_compiled_addr_ht_get(addr_ht, orig_when, 1);
    if (w) {
        *when = w;
        return;
    }

    w = *mem;
    *mem = (char *)*mem + sizeof *w;

    if (orig_when->exts) {
        /* may be referenced in the parent */
        ly_ctx_compiled_addr_ht_add(addr_ht, orig_when, w);
    }

    w->cond = *mem;
    *mem = (char *)*mem + sizeof *w->cond;
    ctxp_expr(orig_when->cond, w->cond, addr_ht, mem);

    w->context = ly_ctx_compiled_addr_ht_get(addr_ht, orig_when->context, 0);
    CTXP_SIZED_ARRAY(orig_when->prefixes, w->prefixes, mem);
    LY_ARRAY_FOR(orig_when->prefixes, u) {
        ctxp_prefix(&orig_when->prefixes[u], &w->prefixes[u], addr_ht, ptr_set, mem);
    }

    w->dsc = ly_ctx_compiled_addr_ht_get(addr_ht, orig_when->dsc, 0);
    w->ref = ly_ctx_compiled_addr_ht_get(addr_ht, orig_when->ref, 0);

    CTXP_SIZED_ARRAY(orig_when->exts, w->exts, mem);
    LY_ARRAY_FOR(orig_when->exts, u) {
        ctxp_ext(&orig_when->exts[u], &w->exts[u], addr_ht, ptr_set, mem);
    }

    w->refcount = orig_when->refcount;
    w->flags = orig_when->flags;

    /* shared */
    ly_ctx_compiled_addr_ht_add(addr_ht, orig_when, w);
    *when = w;
}

static void
ctxp_children(const struct lysc_node *orig_child, struct lysc_node **child, struct ly_ht *addr_ht,
        struct ly_set *ptr_set, void **mem)
{
    struct lysc_node *ch;
    int node_size;

    *child = NULL;

    LY_LIST_FOR(orig_child, orig_child) {
        switch (orig_child->nodetype) {
        case LYS_CONTAINER:
            node_size = sizeof(struct lysc_node_container);
            break;
        case LYS_CHOICE:
            node_size = sizeof(struct lysc_node_choice);
            break;
        case LYS_LEAF:
            node_size = sizeof(struct lysc_node_leaf);
            break;
        case LYS_LEAFLIST:
            node_size = sizeof(struct lysc_node_leaflist);
            break;
        case LYS_LIST:
            node_size = sizeof(struct lysc_node_list);
            break;
        case LYS_ANYXML:
        case LYS_ANYDATA:
            node_size = sizeof(struct lysc_node_anydata);
            break;
        case LYS_CASE:
            node_size = sizeof(struct lysc_node_case);
            break;
        case LYS_RPC:
        case LYS_ACTION:
            node_size = sizeof(struct lysc_node_action);
            break;
        case LYS_NOTIF:
            node_size = sizeof(struct lysc_node_notif);
            break;
        default:
            LOGINT(NULL);
            return;
        }

        ch = *mem;
        *mem = (char *)*mem + node_size;

        ly_ctx_compiled_addr_ht_add(addr_ht, orig_child, ch);
        ctxp_node(orig_child, ch, addr_ht, ptr_set, mem);

        if (!*child) {
            *child = ch;
        }
    }
}

static void
ctxp_range(const struct lysc_range *orig_range, struct lysc_range *range, struct ly_ht *addr_ht, struct ly_set *ptr_set,
        void **mem)
{
    LY_ARRAY_COUNT_TYPE u;

    if (orig_range->exts) {
        /* may be referenced in the parent */
        ly_ctx_compiled_addr_ht_add(addr_ht, orig_range, range);
    }

    CTXP_SIZED_ARRAY(orig_range->parts, range->parts, mem);
    LY_ARRAY_FOR(orig_range->parts, u) {
        range->parts[u].min_64 = orig_range->parts[u].min_64;
        range->parts[u].max_64 = orig_range->parts[u].max_64;
    }

    range->dsc = ly_ctx_compiled_addr_ht_get(addr_ht, orig_range->dsc, 0);
    range->ref = ly_ctx_compiled_addr_ht_get(addr_ht, orig_range->ref, 0);
    range->emsg = ly_ctx_compiled_addr_ht_get(addr_ht, orig_range->emsg, 0);
    range->eapptag = ly_ctx_compiled_addr_ht_get(addr_ht, orig_range->eapptag, 0);

    CTXP_SIZED_ARRAY(orig_range->exts, range->exts, mem);
    LY_ARRAY_FOR(orig_range->exts, u) {
        ctxp_ext(&orig_range->exts[u], &range->exts[u], addr_ht, ptr_set, mem);
    }
}

static void
ctxp_pattern(const struct lysc_pattern *orig_pattern, struct lysc_pattern **pattern, struct ly_ht *addr_ht,
        struct ly_set *ptr_set, void **mem)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_pattern *p;
    size_t code_size;

    /* may have already been printed */
    p = ly_ctx_compiled_addr_ht_get(addr_ht, orig_pattern, 1);
    if (p) {
        *pattern = p;
        return;
    }

    p = *mem;
    *mem = (char *)*mem + sizeof *p;

    if (orig_pattern->exts) {
        /* may be referenced in the parent */
        ly_ctx_compiled_addr_ht_add(addr_ht, orig_pattern, p);
    }

    p->expr = ly_ctx_compiled_addr_ht_get(addr_ht, orig_pattern->expr, 0);

    /* TODO code->tables - seems to be static, probably in the PCRE2 lib */
    pcre2_pattern_info(orig_pattern->code, PCRE2_INFO_SIZE, &code_size);
    p->code = *mem;
    *mem = (char *)*mem + CTXP_MEM_SIZE(code_size);
    memcpy(p->code, orig_pattern->code, code_size);

    p->dsc = ly_ctx_compiled_addr_ht_get(addr_ht, orig_pattern->dsc, 0);
    p->ref = ly_ctx_compiled_addr_ht_get(addr_ht, orig_pattern->ref, 0);
    p->emsg = ly_ctx_compiled_addr_ht_get(addr_ht, orig_pattern->emsg, 0);
    p->eapptag = ly_ctx_compiled_addr_ht_get(addr_ht, orig_pattern->eapptag, 0);

    CTXP_SIZED_ARRAY(orig_pattern->exts, p->exts, mem);
    LY_ARRAY_FOR(orig_pattern->exts, u) {
        ctxp_ext(&orig_pattern->exts[u], &p->exts[u], addr_ht, ptr_set, mem);
    }

    p->inverted = orig_pattern->inverted;
    p->refcount = orig_pattern->refcount;

    /* shared */
    ly_ctx_compiled_addr_ht_add(addr_ht, orig_pattern, p);
    *pattern = p;
}

static void
ctxp_enum(const struct lysc_type_bitenum_item *orig_enum, struct lysc_type_bitenum_item *en, struct ly_ht *addr_ht,
        struct ly_set *ptr_set, void **mem)
{
    LY_ARRAY_COUNT_TYPE u;

    if (orig_enum->exts) {
        /* may be referenced in the parent */
        ly_ctx_compiled_addr_ht_add(addr_ht, orig_enum, en);
    }

    en->name = ly_ctx_compiled_addr_ht_get(addr_ht, orig_enum->name, 0);
    en->dsc = ly_ctx_compiled_addr_ht_get(addr_ht, orig_enum->dsc, 0);
    en->ref = ly_ctx_compiled_addr_ht_get(addr_ht, orig_enum->ref, 0);

    CTXP_SIZED_ARRAY(orig_enum->exts, en->exts, mem);
    LY_ARRAY_FOR(orig_enum->exts, u) {
        ctxp_ext(&orig_enum->exts[u], &en->exts[u], addr_ht, ptr_set, mem);
    }

    en->value = orig_enum->value;
    en->flags = orig_enum->flags;
}

static void
ctxp_type(const struct lysc_type *orig_type, struct lysc_type **type, struct ly_ht *addr_ht, struct ly_set *ptr_set,
        void **mem)
{
    struct lysc_type *t;
    const struct lysc_type_num *orig_type_num;
    struct lysc_type_num *t_num;
    const struct lysc_type_dec *orig_type_dec;
    struct lysc_type_dec *t_dec;
    const struct lysc_type_str *orig_type_str;
    struct lysc_type_str *t_str;
    const struct lysc_type_enum *orig_type_enum_bits;
    struct lysc_type_enum *t_enum_bits;
    const struct lysc_type_leafref *orig_type_lref;
    struct lysc_type_leafref *t_lref;
    const struct lysc_type_identityref *orig_type_identref;
    struct lysc_type_identityref *t_identref;
    const struct lysc_type_instanceid *orig_type_instid;
    struct lysc_type_instanceid *t_instid;
    const struct lysc_type_union *orig_type_union;
    struct lysc_type_union *t_union;
    const struct lysc_type_bin *orig_type_bin;
    struct lysc_type_bin *t_bin;
    LY_ARRAY_COUNT_TYPE u;

    /* may have already been printed */
    t = ly_ctx_compiled_addr_ht_get(addr_ht, orig_type, 1);
    if (t) {
        *type = t;
        return;
    }

    switch (orig_type->basetype) {
    case LY_TYPE_BINARY:
        orig_type_bin = (const struct lysc_type_bin *)orig_type;
        t = *mem;
        *mem = (char *)*mem + sizeof *orig_type_bin;
        t_bin = (struct lysc_type_bin *)t;

        CTXP_OPTIONAL_STRUCT(ctxp_range, orig_type_bin->length, t_bin->length, addr_ht, ptr_set, mem);
        break;
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
        orig_type_num = (const struct lysc_type_num *)orig_type;
        t = *mem;
        *mem = (char *)*mem + sizeof *orig_type_num;
        t_num = (struct lysc_type_num *)t;

        CTXP_OPTIONAL_STRUCT(ctxp_range, orig_type_num->range, t_num->range, addr_ht, ptr_set, mem);
        break;
    case LY_TYPE_STRING:
        orig_type_str = (const struct lysc_type_str *)orig_type;
        t = *mem;
        *mem = (char *)*mem + sizeof *orig_type_str;
        t_str = (struct lysc_type_str *)t;

        CTXP_OPTIONAL_STRUCT(ctxp_range, orig_type_str->length, t_str->length, addr_ht, ptr_set, mem);
        CTXP_SIZED_ARRAY(orig_type_str->patterns, t_str->patterns, mem);
        LY_ARRAY_FOR(orig_type_str->patterns, u) {
            ctxp_pattern(orig_type_str->patterns[u], &t_str->patterns[u], addr_ht, ptr_set, mem);
        }
        break;
    case LY_TYPE_BITS:
    case LY_TYPE_ENUM:
        orig_type_enum_bits = (const struct lysc_type_enum *)orig_type;
        t = *mem;
        *mem = (char *)*mem + sizeof *orig_type_enum_bits;
        t_enum_bits = (struct lysc_type_enum *)t;

        CTXP_SIZED_ARRAY(orig_type_enum_bits->enums, t_enum_bits->enums, mem);
        LY_ARRAY_FOR(orig_type_enum_bits->enums, u) {
            ctxp_enum(&orig_type_enum_bits->enums[u], &t_enum_bits->enums[u], addr_ht, ptr_set, mem);
        }
        break;
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
        /* no additional members */
        t = *mem;
        *mem = (char *)*mem + sizeof *orig_type;
        break;
    case LY_TYPE_DEC64:
        orig_type_dec = (const struct lysc_type_dec *)orig_type;
        t = *mem;
        *mem = (char *)*mem + sizeof *orig_type_dec;
        t_dec = (struct lysc_type_dec *)t;

        t_dec->fraction_digits = orig_type_dec->fraction_digits;
        CTXP_OPTIONAL_STRUCT(ctxp_range, orig_type_dec->range, t_dec->range, addr_ht, ptr_set, mem);
        break;
    case LY_TYPE_IDENT:
        orig_type_identref = (const struct lysc_type_identityref *)orig_type;
        t = *mem;
        *mem = (char *)*mem + sizeof *orig_type_identref;
        t_identref = (struct lysc_type_identityref *)t;

        CTXP_SIZED_ARRAY(orig_type_identref->bases, t_identref->bases, mem);
        LY_ARRAY_FOR(orig_type_identref->bases, u) {
            /* reference */
            t_identref->bases[u] = orig_type_identref->bases[u];
            ly_set_add(ptr_set, &t_identref->bases[u], 1, NULL);
        }
        break;
    case LY_TYPE_INST:
        orig_type_instid = (const struct lysc_type_instanceid *)orig_type;
        t = *mem;
        *mem = (char *)*mem + sizeof *orig_type_instid;
        t_instid = (struct lysc_type_instanceid *)t;

        t_instid->require_instance = orig_type_instid->require_instance;
        break;
    case LY_TYPE_LEAFREF:
        orig_type_lref = (const struct lysc_type_leafref *)orig_type;
        t = *mem;
        *mem = (char *)*mem + sizeof *orig_type_lref;
        t_lref = (struct lysc_type_leafref *)t;

        t_lref->path = *mem;
        *mem = (char *)*mem + sizeof *t_lref->path;
        ctxp_expr(orig_type_lref->path, t_lref->path, addr_ht, mem);

        CTXP_SIZED_ARRAY(orig_type_lref->prefixes, t_lref->prefixes, mem);
        LY_ARRAY_FOR(orig_type_lref->prefixes, u) {
            ctxp_prefix(&orig_type_lref->prefixes[u], &t_lref->prefixes[u], addr_ht, ptr_set, mem);
        }

        ctxp_type(orig_type_lref->realtype, &t_lref->realtype, addr_ht, ptr_set, mem);
        t_lref->require_instance = orig_type_lref->require_instance;
        break;
    case LY_TYPE_UNION:
        orig_type_union = (const struct lysc_type_union *)orig_type;
        t = *mem;
        *mem = (char *)*mem + sizeof *orig_type_union;
        t_union = (struct lysc_type_union *)t;

        CTXP_SIZED_ARRAY(orig_type_union->types, t_union->types, mem);
        LY_ARRAY_FOR(orig_type_union->types, u) {
            ctxp_type(orig_type_union->types[u], &t_union->types[u], addr_ht, ptr_set, mem);
        }
        break;
    case LY_TYPE_UNKNOWN:
        LOGINT(NULL);
        break;
    }

    /* common members */
    t->name = ly_ctx_compiled_addr_ht_get(addr_ht, orig_type->name, 0);

    CTXP_SIZED_ARRAY(orig_type->exts, t->exts, mem);
    LY_ARRAY_FOR(orig_type->exts, u) {
        ctxp_ext(&orig_type->exts[u], &t->exts[u], addr_ht, ptr_set, mem);
    }

    /* static structures in the shared library */
    t->plugin = orig_type->plugin;

    t->basetype = orig_type->basetype;
    t->refcount = orig_type->refcount;

    /* shared */
    ly_ctx_compiled_addr_ht_add(addr_ht, orig_type, t);
    *type = t;
}

static void
ctxp_dflt(const struct lysc_value *orig_dflt, struct lysc_value *dflt, struct ly_ht *addr_ht, struct ly_set *ptr_set,
        void **mem)
{
    LY_ARRAY_COUNT_TYPE u;

    dflt->str = ly_ctx_compiled_addr_ht_get(addr_ht, orig_dflt->str, 0);

    CTXP_SIZED_ARRAY(orig_dflt->prefixes, dflt->prefixes, mem);
    LY_ARRAY_FOR(orig_dflt->prefixes, u) {
        ctxp_prefix(&orig_dflt->prefixes[u], &dflt->prefixes[u], addr_ht, ptr_set, mem);
    }
}

static void
ctxp_node(const struct lysc_node *orig_node, struct lysc_node *node, struct ly_ht *addr_ht, struct ly_set *ptr_set,
        void **mem)
{
    const struct lysc_node_container *orig_cont;
    struct lysc_node_container *cont;
    const struct lysc_node_choice *orig_choic;
    struct lysc_node_choice *choic;
    const struct lysc_node_leaf *orig_leaf;
    struct lysc_node_leaf *leaf;
    const struct lysc_node_leaflist *orig_llist;
    struct lysc_node_leaflist *llist;
    const struct lysc_node_list *orig_list;
    struct lysc_node_list *list;
    const struct lysc_node_anydata *orig_any;
    struct lysc_node_anydata *any;
    const struct lysc_node_case *orig_cas;
    struct lysc_node_case *cas;
    const struct lysc_node_action *orig_act;
    struct lysc_node_action *act;
    const struct lysc_node_notif *orig_notif;
    struct lysc_node_notif *notif;
    LY_ARRAY_COUNT_TYPE u, v;

    /*
     * common members
     */
    node->nodetype = orig_node->nodetype;
    node->flags = orig_node->flags;
    memcpy(node->hash, orig_node->hash, sizeof node->hash);

    /* pointers, references */
    node->module = ly_ctx_compiled_addr_ht_get(addr_ht, orig_node->module, 0);
    node->parent = ly_ctx_compiled_addr_ht_get(addr_ht, orig_node->parent, 0);
    node->next = orig_node->next;
    ly_set_add(ptr_set, &node->next, 1, NULL);
    node->prev = orig_node->prev;
    ly_set_add(ptr_set, &node->prev, 1, NULL);

    /* strings */
    node->name = ly_ctx_compiled_addr_ht_get(addr_ht, orig_node->name, 0);
    node->dsc = ly_ctx_compiled_addr_ht_get(addr_ht, orig_node->dsc, 0);
    node->ref = ly_ctx_compiled_addr_ht_get(addr_ht, orig_node->ref, 0);

    /* exts */
    CTXP_SIZED_ARRAY(orig_node->exts, node->exts, mem);
    LY_ARRAY_FOR(orig_node->exts, u) {
        ctxp_ext(&orig_node->exts[u], &node->exts[u], addr_ht, ptr_set, mem);
    }

    /* priv */
    node->priv = NULL;

    switch (orig_node->nodetype) {
    case LYS_CONTAINER:
        orig_cont = (const struct lysc_node_container *)orig_node;
        cont = (struct lysc_node_container *)node;

        /* children */
        ctxp_children(orig_cont->child, &cont->child, addr_ht, ptr_set, mem);

        /* musts */
        CTXP_SIZED_ARRAY(orig_cont->musts, cont->musts, mem);
        LY_ARRAY_FOR(orig_cont->musts, u) {
            ctxp_must(&orig_cont->musts[u], &cont->musts[u], addr_ht, ptr_set, mem);
        }

        /* when */
        CTXP_SIZED_ARRAY(orig_cont->when, cont->when, mem);
        LY_ARRAY_FOR(orig_cont->when, u) {
            ctxp_when(orig_cont->when[u], &cont->when[u], addr_ht, ptr_set, mem);
        }

        /* actions, notifs */
        ctxp_children((const struct lysc_node *)orig_cont->actions, (struct lysc_node **)&cont->actions, addr_ht,
                ptr_set, mem);
        ctxp_children((const struct lysc_node *)orig_cont->notifs, (struct lysc_node **)&cont->notifs, addr_ht, ptr_set,
                mem);
        break;
    case LYS_CHOICE:
        orig_choic = (const struct lysc_node_choice *)orig_node;
        choic = (struct lysc_node_choice *)node;

        /* cases */
        ctxp_children((const struct lysc_node *)orig_choic->cases, (struct lysc_node **)&choic->cases, addr_ht, ptr_set,
                mem);

        /* when */
        CTXP_SIZED_ARRAY(orig_choic->when, choic->when, mem);
        LY_ARRAY_FOR(orig_choic->when, u) {
            ctxp_when(orig_choic->when[u], &choic->when[u], addr_ht, ptr_set, mem);
        }
        break;
    case LYS_LEAF:
        orig_leaf = (const struct lysc_node_leaf *)orig_node;
        leaf = (struct lysc_node_leaf *)node;

        /* musts */
        CTXP_SIZED_ARRAY(orig_leaf->musts, leaf->musts, mem);
        LY_ARRAY_FOR(orig_leaf->musts, u) {
            ctxp_must(&orig_leaf->musts[u], &leaf->musts[u], addr_ht, ptr_set, mem);
        }

        /* when */
        CTXP_SIZED_ARRAY(orig_leaf->when, leaf->when, mem);
        LY_ARRAY_FOR(orig_leaf->when, u) {
            ctxp_when(orig_leaf->when[u], &leaf->when[u], addr_ht, ptr_set, mem);
        }

        /* type */
        ctxp_type(orig_leaf->type, &leaf->type, addr_ht, ptr_set, mem);

        /* dflt */
        ctxp_dflt(&orig_leaf->dflt, &leaf->dflt, addr_ht, ptr_set, mem);
        break;
    case LYS_LEAFLIST:
        orig_llist = (const struct lysc_node_leaflist *)orig_node;
        llist = (struct lysc_node_leaflist *)node;

        /* musts */
        CTXP_SIZED_ARRAY(orig_llist->musts, llist->musts, mem);
        LY_ARRAY_FOR(orig_llist->musts, u) {
            ctxp_must(&orig_llist->musts[u], &llist->musts[u], addr_ht, ptr_set, mem);
        }

        /* when */
        CTXP_SIZED_ARRAY(orig_llist->when, llist->when, mem);
        LY_ARRAY_FOR(orig_llist->when, u) {
            ctxp_when(orig_llist->when[u], &llist->when[u], addr_ht, ptr_set, mem);
        }

        /* type */
        ctxp_type(orig_llist->type, &llist->type, addr_ht, ptr_set, mem);

        /* dflts */
        CTXP_SIZED_ARRAY(orig_llist->dflts, llist->dflts, mem);
        LY_ARRAY_FOR(orig_llist->dflts, u) {
            ctxp_dflt(&orig_llist->dflts[u], &llist->dflts[u], addr_ht, ptr_set, mem);
        }

        /* min, max */
        llist->min = orig_llist->min;
        llist->max = orig_llist->max;
        break;
    case LYS_LIST:
        orig_list = (const struct lysc_node_list *)orig_node;
        list = (struct lysc_node_list *)node;

        /* children */
        ctxp_children(orig_list->child, &list->child, addr_ht, ptr_set, mem);

        /* musts */
        CTXP_SIZED_ARRAY(orig_list->musts, list->musts, mem);
        LY_ARRAY_FOR(orig_list->musts, u) {
            ctxp_must(&orig_list->musts[u], &list->musts[u], addr_ht, ptr_set, mem);
        }

        /* when */
        CTXP_SIZED_ARRAY(orig_list->when, list->when, mem);
        LY_ARRAY_FOR(orig_list->when, u) {
            ctxp_when(orig_list->when[u], &list->when[u], addr_ht, ptr_set, mem);
        }

        /* actions, notifs */
        ctxp_children((const struct lysc_node *)orig_list->actions, (struct lysc_node **)&list->actions, addr_ht,
                ptr_set, mem);
        ctxp_children((const struct lysc_node *)orig_list->notifs, (struct lysc_node **)&list->notifs, addr_ht, ptr_set,
                mem);

        /* uniques */
        CTXP_SIZED_ARRAY(orig_list->uniques, list->uniques, mem);
        LY_ARRAY_FOR(orig_list->uniques, u) {
            CTXP_SIZED_ARRAY(orig_list->uniques[u], list->uniques[u], mem);
            LY_ARRAY_FOR(orig_list->uniques[u], v) {
                list->uniques[u][v] = ly_ctx_compiled_addr_ht_get(addr_ht, orig_list->uniques[u][v], 0);
            }
        }

        /* min, max */
        list->min = orig_list->min;
        list->max = orig_list->max;
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        orig_any = (const struct lysc_node_anydata *)orig_node;
        any = (struct lysc_node_anydata *)node;

        /* musts */
        CTXP_SIZED_ARRAY(orig_any->musts, any->musts, mem);
        LY_ARRAY_FOR(orig_any->musts, u) {
            ctxp_must(&orig_any->musts[u], &any->musts[u], addr_ht, ptr_set, mem);
        }

        /* when */
        CTXP_SIZED_ARRAY(orig_any->when, any->when, mem);
        LY_ARRAY_FOR(orig_any->when, u) {
            ctxp_when(orig_any->when[u], &any->when[u], addr_ht, ptr_set, mem);
        }
        break;
    case LYS_CASE:
        orig_cas = (const struct lysc_node_case *)orig_node;
        cas = (struct lysc_node_case *)node;

        /* children */
        ctxp_children(orig_cas->child, &cas->child, addr_ht, ptr_set, mem);

        /* when */
        CTXP_SIZED_ARRAY(orig_cas->when, cas->when, mem);
        LY_ARRAY_FOR(orig_cas->when, u) {
            ctxp_when(orig_cas->when[u], &cas->when[u], addr_ht, ptr_set, mem);
        }
        break;
    case LYS_RPC:
    case LYS_ACTION:
        orig_act = (const struct lysc_node_action *)orig_node;
        act = (struct lysc_node_action *)node;

        /* when */
        CTXP_SIZED_ARRAY(orig_act->when, act->when, mem);
        LY_ARRAY_FOR(orig_act->when, u) {
            ctxp_when(orig_act->when[u], &act->when[u], addr_ht, ptr_set, mem);
        }

        /* input children, input is the parent */
        ly_ctx_compiled_addr_ht_add(addr_ht, &orig_act->input, &act->input);
        ctxp_children(orig_act->input.child, &act->input.child, addr_ht, ptr_set, mem);

        /* input musts */
        CTXP_SIZED_ARRAY(orig_act->input.musts, act->input.musts, mem);
        LY_ARRAY_FOR(orig_act->input.musts, u) {
            ctxp_must(&orig_act->input.musts[u], &act->input.musts[u], addr_ht, ptr_set, mem);
        }

        /* output children, output is the parent */
        ly_ctx_compiled_addr_ht_add(addr_ht, &orig_act->output, &act->output);
        ctxp_children(orig_act->output.child, &act->output.child, addr_ht, ptr_set, mem);

        /* output musts */
        CTXP_SIZED_ARRAY(orig_act->output.musts, act->output.musts, mem);
        LY_ARRAY_FOR(orig_act->output.musts, u) {
            ctxp_must(&orig_act->output.musts[u], &act->output.musts[u], addr_ht, ptr_set, mem);
        }
        break;
    case LYS_NOTIF:
        orig_notif = (const struct lysc_node_notif *)orig_node;
        notif = (struct lysc_node_notif *)node;

        /* children */
        ctxp_children(orig_notif->child, &notif->child, addr_ht, ptr_set, mem);

        /* musts */
        CTXP_SIZED_ARRAY(orig_notif->musts, notif->musts, mem);
        LY_ARRAY_FOR(orig_notif->musts, u) {
            ctxp_must(&orig_notif->musts[u], &notif->musts[u], addr_ht, ptr_set, mem);
        }

        /* when */
        CTXP_SIZED_ARRAY(orig_notif->when, notif->when, mem);
        LY_ARRAY_FOR(orig_notif->when, u) {
            ctxp_when(orig_notif->when[u], &notif->when[u], addr_ht, ptr_set, mem);
        }
        break;
    default:
        LOGINT(NULL);
        break;
    }
}

static void
ctxp_compiled(const struct lysc_module *orig_compiled, struct lysc_module *compiled, struct ly_ht *addr_ht,
        struct ly_set *ptr_set, void **mem)
{
    LY_ARRAY_COUNT_TYPE u;

    if (orig_compiled->exts) {
        /* may be referenced in the parent */
        ly_ctx_compiled_addr_ht_add(addr_ht, orig_compiled, compiled);
    }

    /* mod */
    compiled->mod = ly_ctx_compiled_addr_ht_get(addr_ht, orig_compiled->mod, 0);

    /* features */
    CTXP_SIZED_ARRAY(orig_compiled->features, compiled->features, mem);
    LY_ARRAY_FOR(orig_compiled->features, u) {
        compiled->features[u] = ly_ctx_compiled_addr_ht_get(addr_ht, orig_compiled->features[u], 0);
    }

    /* data, referenced */
    ctxp_children(orig_compiled->data, &compiled->data, addr_ht, ptr_set, mem);

    /* rpcs, referenced */
    ctxp_children((const struct lysc_node *)orig_compiled->rpcs, (struct lysc_node **)&compiled->rpcs, addr_ht, ptr_set, mem);

    /* notifs, referenced */
    ctxp_children((const struct lysc_node *)orig_compiled->notifs, (struct lysc_node **)&compiled->notifs, addr_ht, ptr_set, mem);

    /* exts */
    CTXP_SIZED_ARRAY(orig_compiled->exts, compiled->exts, mem);
    LY_ARRAY_FOR(orig_compiled->exts, u) {
        ctxp_ext(&orig_compiled->exts[u], &compiled->exts[u], addr_ht, ptr_set, mem);
    }
}

static void
ctxp_extension(const struct lysc_ext *orig_extension, struct lysc_ext *extension, struct ly_ht *addr_ht,
        struct ly_set *ptr_set, void **mem)
{
    LY_ARRAY_COUNT_TYPE u;

    extension->name = ly_ctx_compiled_addr_ht_get(addr_ht, orig_extension->name, 0);
    extension->argname = ly_ctx_compiled_addr_ht_get(addr_ht, orig_extension->argname, 0);

    CTXP_SIZED_ARRAY(orig_extension->exts, extension->exts, mem);
    LY_ARRAY_FOR(orig_extension->exts, u) {
        ctxp_ext(&orig_extension->exts[u], &extension->exts[u], addr_ht, ptr_set, mem);
    }

    /* static structures in the shared library */
    extension->plugin = orig_extension->plugin;

    extension->module = ly_ctx_compiled_addr_ht_get(addr_ht, orig_extension->module, 0);
    extension->flags = orig_extension->flags;
}

static void
ctxp_identity(const struct lysc_ident *orig_ident, struct lysc_ident *ident, struct ly_ht *addr_ht,
        struct ly_set *ptr_set, void **mem)
{
    LY_ARRAY_COUNT_TYPE u;

    ident->name = ly_ctx_compiled_addr_ht_get(addr_ht, orig_ident->name, 0);
    ident->dsc = ly_ctx_compiled_addr_ht_get(addr_ht, orig_ident->dsc, 0);
    ident->ref = ly_ctx_compiled_addr_ht_get(addr_ht, orig_ident->ref, 0);
    ident->module = ly_ctx_compiled_addr_ht_get(addr_ht, orig_ident->module, 0);

    /* reference */
    CTXP_SIZED_ARRAY(orig_ident->derived, ident->derived, mem);
    LY_ARRAY_FOR(orig_ident->derived, u) {
        ident->derived[u] = orig_ident->derived[u];
        ly_set_add(ptr_set, &ident->derived[u], 1, NULL);
    }

    CTXP_SIZED_ARRAY(orig_ident->exts, ident->exts, mem);
    LY_ARRAY_FOR(orig_ident->exts, u) {
        ctxp_ext(&orig_ident->exts[u], &ident->exts[u], addr_ht, ptr_set, mem);
    }

    ident->flags = orig_ident->flags;
}

static void
ctxp_submodule(const struct lysc_submodule *orig_submod, struct lysc_submodule *submod, struct ly_ht *addr_ht)
{
    submod->name = ly_ctx_compiled_addr_ht_get(addr_ht, orig_submod->name, 0);
    submod->revision = ly_ctx_compiled_addr_ht_get(addr_ht, orig_submod->revision, 0);
    submod->filepath = ly_ctx_compiled_addr_ht_get(addr_ht, orig_submod->filepath, 0);
}

static void
ctxp_module(const struct lys_module *orig_mod, struct lys_module *mod, struct ly_ht *addr_ht, struct ly_set *ptr_set,
        void **mem)
{
    LY_ARRAY_COUNT_TYPE u;

    /* ctx */
    mod->ctx = ly_ctx_compiled_addr_ht_get(addr_ht, orig_mod->ctx, 0);

    /* strings */
    mod->name = ly_ctx_compiled_addr_ht_get(addr_ht, orig_mod->name, 0);
    mod->revision = ly_ctx_compiled_addr_ht_get(addr_ht, orig_mod->revision, 0);
    mod->ns = ly_ctx_compiled_addr_ht_get(addr_ht, orig_mod->ns, 0);
    mod->prefix = ly_ctx_compiled_addr_ht_get(addr_ht, orig_mod->prefix, 0);
    mod->filepath = ly_ctx_compiled_addr_ht_get(addr_ht, orig_mod->filepath, 0);
    mod->org = ly_ctx_compiled_addr_ht_get(addr_ht, orig_mod->org, 0);
    mod->contact = ly_ctx_compiled_addr_ht_get(addr_ht, orig_mod->contact, 0);
    mod->dsc = ly_ctx_compiled_addr_ht_get(addr_ht, orig_mod->dsc, 0);
    mod->ref = ly_ctx_compiled_addr_ht_get(addr_ht, orig_mod->ref, 0);

    /* no parsed modules */
    mod->parsed = NULL;

    /* compiled */
    CTXP_OPTIONAL_STRUCT(ctxp_compiled, orig_mod->compiled, mod->compiled, addr_ht, ptr_set, mem);

    /* extensions, referenced */
    CTXP_SIZED_ARRAY(orig_mod->extensions, mod->extensions, mem);
    LY_ARRAY_FOR(orig_mod->extensions, u) {
        ly_ctx_compiled_addr_ht_add(addr_ht, &orig_mod->extensions[u], &mod->extensions[u]);
        ctxp_extension(&orig_mod->extensions[u], &mod->extensions[u], addr_ht, ptr_set, mem);
    }

    /* identities, referenced */
    CTXP_SIZED_ARRAY(orig_mod->identities, mod->identities, mem);
    LY_ARRAY_FOR(orig_mod->identities, u) {
        ly_ctx_compiled_addr_ht_add(addr_ht, &orig_mod->identities[u], &mod->identities[u]);
        ctxp_identity(&orig_mod->identities[u], &mod->identities[u], addr_ht, ptr_set, mem);
    }

    /* submodules */
    CTXP_SIZED_ARRAY(orig_mod->submodules, mod->submodules, mem);
    LY_ARRAY_FOR(orig_mod->submodules, u) {
        ctxp_submodule(&orig_mod->submodules[u], &mod->submodules[u], addr_ht);
    }

    /* augmented_by, reference */
    CTXP_SIZED_ARRAY(orig_mod->augmented_by, mod->augmented_by, mem);
    LY_ARRAY_FOR(orig_mod->augmented_by, u) {
        mod->augmented_by[u] = orig_mod->augmented_by[u];
        ly_set_add(ptr_set, &mod->augmented_by[u], 1, NULL);
    }

    /* deviated_by, reference */
    CTXP_SIZED_ARRAY(orig_mod->deviated_by, mod->deviated_by, mem);
    LY_ARRAY_FOR(orig_mod->deviated_by, u) {
        mod->deviated_by[u] = orig_mod->deviated_by[u];
        ly_set_add(ptr_set, &mod->deviated_by[u], 1, NULL);
    }

    /* flags */
    mod->implemented = orig_mod->implemented;
    mod->to_compile = orig_mod->to_compile;
    mod->latest_revision = orig_mod->latest_revision;
}

void
ly_ctx_compiled_print_context(const struct ly_ctx *orig_ctx, struct ly_ctx *ctx, struct ly_ht *addr_ht,
        struct ly_set *ptr_set, void **mem)
{
    uint32_t i;

    /* dictionary */
    ctx->dict.hash_tab = *mem;
    *mem = (char *)*mem + sizeof *ctx->dict.hash_tab;

    ctxp_dict_ht(orig_ctx->dict.hash_tab, ctx->dict.hash_tab, addr_ht, mem);
    ctxp_mutex(&ctx->dict.lock);

    /* no search paths */
    memset(&ctx->search_paths, 0, sizeof ctx->search_paths);

    /* modules, referenced */
    ctxp_set(&orig_ctx->modules, &ctx->modules, mem);
    for (i = 0; i < ctx->modules.count; ++i) {
        ctx->modules.objs[i] = *mem;
        *mem = (char *)*mem + sizeof(struct lys_module);

        ly_ctx_compiled_addr_ht_add(addr_ht, orig_ctx->modules.objs[i], ctx->modules.objs[i]);
        ctxp_module(orig_ctx->modules.objs[i], ctx->modules.objs[i], addr_ht, ptr_set, mem);
    }

    /* no imp cb */
    ctx->imp_clb = NULL;
    ctx->imp_clb_data = NULL;

    /* no unres */
    memset(&ctx->unres, 0, sizeof ctx->unres);

    /* change_count and options */
    ctx->change_count = orig_ctx->change_count;
    ctx->opts = orig_ctx->opts;

    /* plugins */
    ctxp_set(&orig_ctx->plugins_types, &ctx->plugins_types, mem);
    memcpy(ctx->plugins_types.objs, orig_ctx->plugins_types.objs,
            orig_ctx->plugins_types.count * sizeof orig_ctx->plugins_types.objs);
    ctxp_set(&orig_ctx->plugins_extensions, &ctx->plugins_extensions, mem);
    memcpy(ctx->plugins_extensions.objs, orig_ctx->plugins_extensions.objs,
            orig_ctx->plugins_extensions.count * sizeof orig_ctx->plugins_extensions.objs);
}

LY_ERR
ly_ctx_compiled_ext_stmts_storage_print(const struct lysc_ext_substmt *orig_substmts, struct lysc_ext_substmt *substmts,
        struct ly_ht *addr_ht, struct ly_set *ptr_set, void **mem)
{
    LY_ERR rc = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;

    LY_ARRAY_FOR(orig_substmts, u) {
        assert(orig_substmts[u].stmt == substmts[u].stmt);

        switch (orig_substmts[u].stmt) {
        case LY_STMT_NOTIFICATION:
        case LY_STMT_INPUT:
        case LY_STMT_OUTPUT:
        case LY_STMT_ACTION:
        case LY_STMT_RPC:
        case LY_STMT_ANYDATA:
        case LY_STMT_ANYXML:
        case LY_STMT_CASE:
        case LY_STMT_CHOICE:
        case LY_STMT_CONTAINER:
        case LY_STMT_LEAF:
        case LY_STMT_LEAF_LIST:
        case LY_STMT_LIST:
        case LY_STMT_USES:
            ctxp_children(*orig_substmts[u].storage_p, (struct lysc_node **)substmts[u].storage_p, addr_ht, ptr_set, mem);
            break;
        case LY_STMT_ARGUMENT:
        case LY_STMT_CONTACT:
        case LY_STMT_DESCRIPTION:
        case LY_STMT_ERROR_APP_TAG:
        case LY_STMT_ERROR_MESSAGE:
        case LY_STMT_KEY:
        case LY_STMT_MODIFIER:
        case LY_STMT_NAMESPACE:
        case LY_STMT_ORGANIZATION:
        case LY_STMT_PRESENCE:
        case LY_STMT_REFERENCE:
        case LY_STMT_UNITS:
            /* string, in the dictionary */
            *substmts[u].storage_p = ly_ctx_compiled_addr_ht_get(addr_ht, *orig_substmts[u].storage_p, 0);
            break;
        case LY_STMT_BIT:
        case LY_STMT_ENUM: {
            const struct lysc_type_bitenum_item *orig_enums = *orig_substmts[u].storage_p;
            struct lysc_type_bitenum_item *enums = *substmts[u].storage_p;

            CTXP_SIZED_ARRAY(orig_enums, enums, mem);
            LY_ARRAY_FOR(orig_enums, v) {
                ctxp_enum(&orig_enums[v], &enums[v], addr_ht, ptr_set, mem);
            }
            break;
        }
        case LY_STMT_CONFIG:
        case LY_STMT_MANDATORY:
        case LY_STMT_ORDERED_BY:
        case LY_STMT_STATUS:
            memcpy(orig_substmts[u].storage_p, substmts[u].storage_p, sizeof(uint16_t));
            break;
        case LY_STMT_EXTENSION_INSTANCE: {
            const struct lysc_ext_instance *orig_exts = *orig_substmts[u].storage_p;
            struct lysc_ext_instance *exts = *substmts[u].storage_p;

            CTXP_SIZED_ARRAY(orig_exts, exts, mem);
            LY_ARRAY_FOR(orig_exts, v) {
                ctxp_ext(&orig_exts[v], &exts[v], addr_ht, ptr_set, mem);
            }
            break;
        }
        case LY_STMT_FRACTION_DIGITS:
        case LY_STMT_REQUIRE_INSTANCE:
            memcpy(orig_substmts[u].storage_p, substmts[u].storage_p, sizeof(uint8_t));
            break;
        case LY_STMT_IDENTITY: {
            const struct lysc_ident *orig_identities = *orig_substmts[u].storage_p;
            struct lysc_ident *identities = *substmts[u].storage_p;

            CTXP_SIZED_ARRAY(orig_identities, identities, mem);
            LY_ARRAY_FOR(orig_identities, v) {
                ctxp_identity(&orig_identities[v], &identities[v], addr_ht, ptr_set, mem);
            }
            break;
        }
        case LY_STMT_LENGTH:
        case LY_STMT_RANGE:
            *substmts[u].storage_p = *mem;
            *mem = (char *)*mem + sizeof(struct lysc_range);
            ctxp_range(*orig_substmts[u].storage_p, *substmts[u].storage_p, addr_ht, ptr_set, mem);
            break;
        case LY_STMT_MAX_ELEMENTS:
        case LY_STMT_MIN_ELEMENTS:
            memcpy(orig_substmts[u].storage_p, substmts[u].storage_p, sizeof(uint32_t));
            break;
        case LY_STMT_MUST: {
            const struct lysc_must *orig_musts = *orig_substmts[u].storage_p;
            struct lysc_must *musts = *substmts[u].storage_p;

            CTXP_SIZED_ARRAY(orig_musts, musts, mem);
            LY_ARRAY_FOR(orig_musts, v) {
                ctxp_must(&orig_musts[v], &musts[v], addr_ht, ptr_set, mem);
            }
            break;
        }
        case LY_STMT_PATTERN: {
            const struct lysc_pattern **orig_patterns = *orig_substmts[u].storage_p;
            struct lysc_pattern **patterns = *substmts[u].storage_p;

            CTXP_SIZED_ARRAY(orig_patterns, patterns, mem);
            LY_ARRAY_FOR(orig_patterns, v) {
                ctxp_pattern(orig_patterns[v], &patterns[v], addr_ht, ptr_set, mem);
            }
            break;
        }
        case LY_STMT_POSITION:
        case LY_STMT_VALUE:
            memcpy(orig_substmts[u].storage_p, substmts[u].storage_p, sizeof(uint64_t));
            break;
        case LY_STMT_TYPE:
            ctxp_type(*orig_substmts[u].storage_p, (struct lysc_type **)substmts[u].storage_p, addr_ht, ptr_set, mem);
            break;
        case LY_STMT_WHEN:
            ctxp_when(*orig_substmts[u].storage_p, (struct lysc_when **)substmts[u].storage_p, addr_ht, ptr_set, mem);
            break;
        case LY_STMT_IF_FEATURE:
            /* not compiled, but can be a substatement */
            break;
        case LY_STMT_NONE:
        case LY_STMT_AUGMENT:
        case LY_STMT_GROUPING:
        case LY_STMT_BASE:
        case LY_STMT_BELONGS_TO:
        case LY_STMT_DEFAULT:
        case LY_STMT_DEVIATE:
        case LY_STMT_DEVIATION:
        case LY_STMT_EXTENSION:
        case LY_STMT_FEATURE:
        case LY_STMT_IMPORT:
        case LY_STMT_INCLUDE:
        case LY_STMT_MODULE:
        case LY_STMT_PATH:
        case LY_STMT_PREFIX:
        case LY_STMT_REFINE:
        case LY_STMT_REVISION:
        case LY_STMT_REVISION_DATE:
        case LY_STMT_SUBMODULE:
        case LY_STMT_TYPEDEF:
        case LY_STMT_UNIQUE:
        case LY_STMT_YANG_VERSION:
        case LY_STMT_YIN_ELEMENT:
        case LY_STMT_SYNTAX_SEMICOLON:
        case LY_STMT_SYNTAX_LEFT_BRACE:
        case LY_STMT_SYNTAX_RIGHT_BRACE:
        case LY_STMT_ARG_TEXT:
        case LY_STMT_ARG_VALUE:
            /* not compiled, unreachable */
            LOGINT(NULL);
            rc = LY_EINT;
            goto cleanup;
        }
    }

cleanup:
    return rc;
}

void *
ly_ctx_compiled_addr_ht_get(const struct ly_ht *addr_ht, const void *addr, ly_bool ignore_not_found)
{
    struct ctxp_ht_addr val = {0}, *val_p;
    uint32_t hash;

    if (!addr) {
        return NULL;
    }

    val.orig_addr = addr;
    hash = lyht_hash((const char *)&addr, sizeof addr);

    /* try to find the orig_addr */
    if (lyht_find(addr_ht, &val, hash, (void **)&val_p) == LY_ENOTFOUND) {
        if (!ignore_not_found) {
            assert(0);
            LOGINT(NULL);
        }
        return NULL;
    }

    return (void *)val_p->addr;
}

LY_ERR
ly_ctx_compiled_addr_ht_add(struct ly_ht *addr_ht, const void *orig_addr, const void *addr)
{
    struct ctxp_ht_addr val;
    uint32_t hash;

    val.orig_addr = orig_addr;
    val.addr = addr;
    hash = lyht_hash((const char *)&orig_addr, sizeof orig_addr);

    /* insert the value */
    return lyht_insert(addr_ht, &val, hash, NULL);
}
