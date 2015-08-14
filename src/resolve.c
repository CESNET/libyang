/**
 * @file resolve.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang resolve functions
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "libyang.h"
#include "resolve.h"
#include "common.h"
#include "parse.h"
#include "dict.h"
#include "tree_internal.h"

/**
 * @brief Resolves length or range intervals. Does not log.
 * Syntax is assumed to be correct, *local_intv MUST be NULL.
 *
 * @param[in] str_restr The restriction as a string.
 * @param[in] type The type of the restriction.
 * @param[in] superior_restr Flag whether to check superior
 * types.
 * @param[out] local_intv The final interval structure.
 *
 * @return EXIT_SUCCESS on succes, EXIT_FAILURE otherwise.
 */
int
resolve_len_ran_interval(const char *str_restr, struct lys_type *type, int superior_restr, struct len_ran_intv** local_intv)
{
    /* 0 - unsigned, 1 - signed, 2 - floating point */
    int kind, ret = 0;
    int64_t local_smin, local_smax;
    uint64_t local_umin, local_umax;
    long double local_fmin, local_fmax;
    const char *seg_ptr, *ptr;
    struct len_ran_intv *tmp_local_intv = NULL, *tmp_intv, *intv = NULL;

    switch (type->base) {
    case LY_TYPE_BINARY:
        kind = 0;
        local_umin = 0;
        local_umax = 18446744073709551615UL;

        if (!str_restr && type->info.binary.length) {
            str_restr = type->info.binary.length->expr;
        }
        break;
    case LY_TYPE_DEC64:
        kind = 2;
        local_fmin = -9223372036854775808.0;
        local_fmin /= 1 << type->info.dec64.dig;
        local_fmax = 9223372036854775807.0;
        local_fmax /= 1 << type->info.dec64.dig;

        if (!str_restr && type->info.dec64.range) {
            str_restr = type->info.dec64.range->expr;
        }
        break;
    case LY_TYPE_INT8:
        kind = 1;
        local_smin = -128;
        local_smax = 127;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_INT16:
        kind = 1;
        local_smin = -32768;
        local_smax = 32767;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_INT32:
        kind = 1;
        local_smin = -2147483648;
        local_smax = 2147483647;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_INT64:
        kind = 1;
        local_smin = -9223372036854775807L - 1L;
        local_smax = 9223372036854775807L;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT8:
        kind = 0;
        local_umin = 0;
        local_umax = 255;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT16:
        kind = 0;
        local_umin = 0;
        local_umax = 65535;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT32:
        kind = 0;
        local_umin = 0;
        local_umax = 4294967295;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT64:
        kind = 0;
        local_umin = 0;
        local_umax = 18446744073709551615UL;

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_STRING:
        kind = 0;
        local_umin = 0;
        local_umax = 18446744073709551615UL;

        if (!str_restr && type->info.str.length) {
            str_restr = type->info.str.length->expr;
        }
        break;
    default:
        LOGINT;
        return EXIT_FAILURE;
    }

    /* process superior types */
    if (type->der && superior_restr) {
        if (resolve_len_ran_interval(NULL, &type->der->type, superior_restr, &intv)) {
            LOGINT;
            return EXIT_FAILURE;
        }
        assert(!intv || (intv->kind == kind));
    }

    if (!str_restr) {
        /* we are validating data and not have any restriction, but a superior type might have */
        if (type->der && !superior_restr && !intv) {
            if (resolve_len_ran_interval(NULL, &type->der->type, superior_restr, &intv)) {
                LOGINT;
                return EXIT_FAILURE;
            }
            assert(!intv || (intv->kind == kind));
        }
        *local_intv = intv;
        return EXIT_SUCCESS;
    }

    /* adjust local min and max */
    if (intv) {
        tmp_intv = intv;

        if (kind == 0) {
            local_umin = tmp_intv->value.uval.min;
        } else if (kind == 1) {
            local_smin = tmp_intv->value.sval.min;
        } else if (kind == 2) {
            local_fmin = tmp_intv->value.fval.min;
        }

        while (tmp_intv->next) {
            tmp_intv = tmp_intv->next;
        }

        if (kind == 0) {
            local_umax = tmp_intv->value.uval.max;
        } else if (kind == 1) {
            local_smax = tmp_intv->value.sval.max;
        } else if (kind == 2) {
            local_fmax = tmp_intv->value.fval.max;
        }
    }

    /* finally parse our restriction */
    seg_ptr = str_restr;
    while (1) {
        if (!*local_intv && !tmp_local_intv) {
            *local_intv = malloc(sizeof **local_intv);
            tmp_local_intv = *local_intv;
        } else {
            tmp_local_intv->next = malloc(sizeof **local_intv);
            tmp_local_intv = tmp_local_intv->next;
        }

        tmp_local_intv->kind = kind;
        tmp_local_intv->next = NULL;

        /* min */
        ptr = seg_ptr;
        while (isspace(ptr[0])) {
            ++ptr;
        }
        if (isdigit(ptr[0]) || (ptr[0] == '+') || (ptr[0] == '-')) {
            if (kind == 0) {
                tmp_local_intv->value.uval.min = atoll(ptr);
            } else if (kind == 1) {
                tmp_local_intv->value.sval.min = atoll(ptr);
            } else if (kind == 2) {
                tmp_local_intv->value.fval.min = atoll(ptr);
            }

            if ((ptr[0] == '+') || (ptr[0] == '-')) {
                ++ptr;
            }
            while (isdigit(ptr[0])) {
                ++ptr;
            }
        } else if (!strncmp(ptr, "min", 3)) {
            if (kind == 0) {
                tmp_local_intv->value.uval.min = local_umin;
            } else if (kind == 1) {
                tmp_local_intv->value.sval.min = local_smin;
            } else if (kind == 2) {
                tmp_local_intv->value.fval.min = local_fmin;
            }

            ptr += 3;
        } else if (!strncmp(ptr, "max", 3)) {
            if (kind == 0) {
                tmp_local_intv->value.uval.min = local_umax;
            } else if (kind == 1) {
                tmp_local_intv->value.sval.min = local_smax;
            } else if (kind == 2) {
                tmp_local_intv->value.fval.min = local_fmax;
            }

            ptr += 3;
        } else {
            LOGINT;
            ret = EXIT_FAILURE;
            goto cleanup;
        }

        while (isspace(ptr[0])) {
            ptr++;
        }

        /* no interval or interval */
        if ((ptr[0] == '|') || !ptr[0]) {
            if (kind == 0) {
                tmp_local_intv->value.uval.max = tmp_local_intv->value.uval.min;
            } else if (kind == 1) {
                tmp_local_intv->value.sval.max = tmp_local_intv->value.sval.min;
            } else if (kind == 2) {
                tmp_local_intv->value.fval.max = tmp_local_intv->value.fval.min;
            }
        } else if (!strncmp(ptr, "..", 2)) {
            /* skip ".." */
            ptr += 2;
            while (isspace(ptr[0])) {
                ++ptr;
            }

            /* max */
            if (isdigit(ptr[0]) || (ptr[0] == '+') || (ptr[0] == '-')) {
                if (kind == 0) {
                    tmp_local_intv->value.uval.max = atoll(ptr);
                } else if (kind == 1) {
                    tmp_local_intv->value.sval.max = atoll(ptr);
                } else if (kind == 2) {
                    tmp_local_intv->value.fval.max = atoll(ptr);
                }
            } else if (!strncmp(ptr, "max", 3)) {
                if (kind == 0) {
                    tmp_local_intv->value.uval.max = local_umax;
                } else if (kind == 1) {
                    tmp_local_intv->value.sval.max = local_smax;
                } else if (kind == 2) {
                    tmp_local_intv->value.fval.max = local_fmax;
                }
            } else {
                LOGINT;
                ret = EXIT_FAILURE;
                goto cleanup;
            }
        } else {
            LOGINT;
            ret = EXIT_FAILURE;
            goto cleanup;
        }

        /* next segment (next OR) */
        seg_ptr = strchr(seg_ptr, '|');
        if (!seg_ptr) {
            break;
        }
        seg_ptr++;
    }

    /* check local restrictions against superior ones */
    if (intv) {
        tmp_intv = intv;
        tmp_local_intv = *local_intv;

        while (tmp_local_intv && tmp_intv) {
            /* reuse local variables */
            if (kind == 0) {
                local_umin = tmp_local_intv->value.uval.min;
                local_umax = tmp_local_intv->value.uval.max;

                /* it must be in this interval */
                if ((local_umin >= tmp_intv->value.uval.min) && (local_umin <= tmp_intv->value.uval.max)) {
                    /* this interval is covered, next one */
                    if (local_umax <= tmp_intv->value.uval.max) {
                        tmp_local_intv = tmp_local_intv->next;
                        continue;
                    /* ascending order of restrictions -> fail */
                    } else {
                        ret = EXIT_FAILURE;
                        goto cleanup;
                    }
                }
            } else if (kind == 1) {
                local_smin = tmp_local_intv->value.sval.min;
                local_smax = tmp_local_intv->value.sval.max;

                if ((local_smin >= tmp_intv->value.sval.min) && (local_smin <= tmp_intv->value.sval.max)) {
                    if (local_smax <= tmp_intv->value.sval.max) {
                        tmp_local_intv = tmp_local_intv->next;
                        continue;
                    } else {
                        ret = EXIT_FAILURE;
                        goto cleanup;
                    }
                }
            } else if (kind == 2) {
                local_fmin = tmp_local_intv->value.fval.min;
                local_fmax = tmp_local_intv->value.fval.max;

                 if ((local_fmin >= tmp_intv->value.fval.min) && (local_fmin <= tmp_intv->value.fval.max)) {
                    if (local_fmax <= tmp_intv->value.fval.max) {
                        tmp_local_intv = tmp_local_intv->next;
                        continue;
                    } else {
                        ret = EXIT_FAILURE;
                        goto cleanup;
                    }
                }
            }

            tmp_intv = tmp_intv->next;
        }

        /* some interval left uncovered -> fail */
        if (tmp_local_intv) {
            ret = EXIT_FAILURE;
        }

    }

cleanup:
    while (intv) {
        tmp_intv = intv->next;
        free(intv);
        intv = tmp_intv;
    }

    /* fail */
    if (ret) {
        while (*local_intv) {
            tmp_local_intv = (*local_intv)->next;
            free(*local_intv);
            *local_intv = tmp_local_intv;
        }
    }

    return ret;
}

/**
 * @brief Resolve a typedef. Does not log.
 *
 * @param[in] name Typedef name.
 * @param[in] prefix Typedef name prefix.
 * @param[in] module The main module.
 * @param[in] parent The parent of the resolved type definition.
 *
 * @return Typedef pointer on succes, NULL otherwise.
 */
struct lys_tpdf *
resolve_superior_type(const char *name, const char *prefix, struct lys_module *module, struct lys_node *parent)
{
    int i, j, found = 0;
    struct lys_tpdf *tpdf;
    int tpdf_size;

    if (!prefix) {
        /* no prefix, try built-in types */
        for (i = 1; i < LY_DATA_TYPE_COUNT; i++) {
            if (!strcmp(ly_types[i].def->name, name)) {
                return ly_types[i].def;
            }
        }
    } else {
        if (!strcmp(prefix, module->prefix)) {
            /* prefix refers to the current module, ignore it */
            prefix = NULL;
        }
    }

    if (!prefix && parent) {
        /* search in local typedefs */
        while (parent) {
            switch (parent->nodetype) {
            case LYS_CONTAINER:
                tpdf_size = ((struct lys_node_container *)parent)->tpdf_size;
                tpdf = ((struct lys_node_container *)parent)->tpdf;
                break;

            case LYS_LIST:
                tpdf_size = ((struct lys_node_list *)parent)->tpdf_size;
                tpdf = ((struct lys_node_list *)parent)->tpdf;
                break;

            case LYS_GROUPING:
                tpdf_size = ((struct lys_node_grp *)parent)->tpdf_size;
                tpdf = ((struct lys_node_grp *)parent)->tpdf;
                break;

            case LYS_RPC:
                tpdf_size = ((struct lys_node_rpc *)parent)->tpdf_size;
                tpdf = ((struct lys_node_rpc *)parent)->tpdf;
                break;

            case LYS_NOTIF:
                tpdf_size = ((struct lys_node_notif *)parent)->tpdf_size;
                tpdf = ((struct lys_node_notif *)parent)->tpdf;
                break;

            case LYS_INPUT:
            case LYS_OUTPUT:
                tpdf_size = ((struct lys_node_rpc_inout *)parent)->tpdf_size;
                tpdf = ((struct lys_node_rpc_inout *)parent)->tpdf;
                break;

            default:
                parent = parent->parent;
                continue;
            }

            for (i = 0; i < tpdf_size; i++) {
                if (!strcmp(tpdf[i].name, name)) {
                    return &tpdf[i];
                }
            }

            parent = parent->parent;
        }
    } else if (prefix) {
        /* get module where to search */
        for (i = 0; i < module->imp_size; i++) {
            if (!strcmp(module->imp[i].prefix, prefix)) {
                module = module->imp[i].module;
                found = 1;
                break;
            }
        }
        if (!found) {
            return NULL;
        }
    }

    /* search in top level typedefs */
    for (i = 0; i < module->tpdf_size; i++) {
        if (!strcmp(module->tpdf[i].name, name)) {
            return &module->tpdf[i];
        }
    }

    /* search in submodules */
    for (i = 0; i < module->inc_size; i++) {
        for (j = 0; j < module->inc[i].submodule->tpdf_size; j++) {
            if (!strcmp(module->inc[i].submodule->tpdf[j].name, name)) {
                return &module->inc[i].submodule->tpdf[j];
            }
        }
    }

    return NULL;
}

static int
check_default(struct lys_type *type, const char *value)
{
    /* TODO - RFC 6020, sec. 7.3.4 */
    (void)type;
    (void)value;
    return EXIT_SUCCESS;
}

/**
 * @brief Check a key for mandatory attributes. Logs directly.
 *
 * @param[in] key The key to check.
 * @param[in] flags What flags to check.
 * @param[in] list The list of all the keys.
 * @param[in] index Index of the key in the key list.
 * @param[in] name The name of the keys.
 * @param[in] len The name length.
 * @param[in] line The line in the input file.
 *
 * @return EXIT_SUCCESS on succes, EXIT_FAILURE otherwise.
 */
static int
check_key(struct lys_node_leaf *key, uint8_t flags, struct lys_node_leaf **list, int index, const char *name, int len,
          uint32_t line)
{
    char *dup = NULL;
    int j;

    /* existence */
    if (!key) {
        if (name[len] != '\0') {
            dup = strdup(name);
            dup[len] = '\0';
            name = dup;
        }
        LOGVAL(LYE_KEY_MISS, line, name);
        free(dup);
        return EXIT_FAILURE;
    }

    /* uniqueness */
    for (j = index - 1; j >= 0; j--) {
        if (list[index] == list[j]) {
            LOGVAL(LYE_KEY_DUP, line, key->name);
            return EXIT_FAILURE;
        }
    }

    /* key is a leaf */
    if (key->nodetype != LYS_LEAF) {
        LOGVAL(LYE_KEY_NLEAF, line, key->name);
        return EXIT_FAILURE;
    }

    /* type of the leaf is not built-in empty */
    if (key->type.base == LY_TYPE_EMPTY) {
        LOGVAL(LYE_KEY_TYPE, line, key->name);
        return EXIT_FAILURE;
    }

    /* config attribute is the same as of the list */
    if ((flags & LYS_CONFIG_MASK) != (key->flags & LYS_CONFIG_MASK)) {
        LOGVAL(LYE_KEY_CONFIG, line, key->name);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve (find) a prefix in a module include import. Does not log.
 *
 * @param[in] mod The module with the import.
 * @param[in] prefix The prefix to find.
 * @param[in] pref_len The prefix length.
 *
 * @return The matching module on success, NULL on error.
 */
static struct lys_module *
resolve_import_in_includes_recursive(struct lys_module *mod, const char *prefix, uint32_t pref_len)
{
    int i, j;
    struct lys_submodule *sub_mod;
    struct lys_module *ret;

    for (i = 0; i < mod->inc_size; i++) {
        sub_mod = mod->inc[i].submodule;
        for (j = 0; j < sub_mod->imp_size; j++) {
            if ((pref_len == strlen(sub_mod->imp[j].prefix))
                    && !strncmp(sub_mod->imp[j].prefix, prefix, pref_len)) {
                return sub_mod->imp[j].module;
            }
        }
    }

    for (i = 0; i < mod->inc_size; i++) {
        ret = resolve_import_in_includes_recursive((struct lys_module *)mod->inc[i].submodule, prefix, pref_len);
        if (ret) {
            return ret;
        }
    }

    return NULL;
}

/**
 * @brief Resolve (find) a prefix in a module import. Does not log.
 *
 * @param[in] mod The module with the import.
 * @param[in] prefix The prefix to find.
 * @param[in] pref_len The prefix length.
 *
 * @return The matching module on success, NULL on error.
 */
static struct lys_module *
resolve_prefixed_module(struct lys_module *mod, const char *prefix, uint32_t pref_len)
{
    int i;

    /* module itself */
    if (!strncmp(mod->prefix, prefix, pref_len) && mod->prefix[pref_len] == '\0') {
        return mod;
    }

    /* imported modules */
    for (i = 0; i < mod->imp_size; i++) {
        if (!strncmp(mod->imp[i].prefix, prefix, pref_len) && mod->imp[i].prefix[pref_len] == '\0') {
            return mod->imp[i].module;
        }
    }

    /* imports in includes */
    return resolve_import_in_includes_recursive(mod, prefix, pref_len);
}

/**
 * @brief Resolve (fill) a unique. Logs directly.
 *
 * @param[in] parent The parent node of the unique structure.
 * @param[in] uniq_str The value of the unique node.
 * @param[in] uniq_s The unique structure in question.
 * @param[in] line The line in the input file.
 *
 * @return EXIT_SUCCESS on succes, EXIT_FAILURE otherwise.
 */
int
resolve_unique(struct lys_node *parent, const char *uniq_str, struct lys_unique *uniq_s, uint32_t line)
{
    char *uniq_val, *uniq_begin, *start;
    int i, j;

    /* count the number of unique values */
    uniq_val = uniq_begin = strdup(uniq_str);
    uniq_s->leafs_size = 0;
    while ((uniq_val = strpbrk(uniq_val, " \t\n"))) {
        uniq_s->leafs_size++;
        while (isspace(*uniq_val)) {
            uniq_val++;
        }
    }
    uniq_s->leafs_size++;
    uniq_s->leafs = calloc(uniq_s->leafs_size, sizeof *uniq_s->leafs);

    /* interconnect unique values with the leafs */
    uniq_val = uniq_begin;
    for (i = 0; uniq_val && i < uniq_s->leafs_size; i++) {
        start = uniq_val;
        if ((uniq_val = strpbrk(start, " \t\n"))) {
            *uniq_val = '\0'; /* add terminating NULL byte */
            uniq_val++;
            while (isspace(*uniq_val)) {
                uniq_val++;
            }
        } /* else only one nodeid present/left already NULL byte terminated */

        uniq_s->leafs[i] = (struct lys_node_leaf *)resolve_schema_nodeid(start, parent->child, parent->module, LYS_LEAF);
        if (!uniq_s->leafs[i] || (uniq_s->leafs[i]->nodetype != LYS_LEAF)) {
            LOGVAL(LYE_INARG, line, start, "unique");
            if (!uniq_s->leafs[i]) {
                LOGVAL(LYE_SPEC, 0, "Target leaf not found.");
            } else {
                LOGVAL(LYE_SPEC, 0, "Target is not a leaf.");
            }
            goto error;
        }

        for (j = 0; j < i; j++) {
            if (uniq_s->leafs[j] == uniq_s->leafs[i]) {
                LOGVAL(LYE_INARG, line, start, "unique");
                LOGVAL(LYE_SPEC, 0, "The identifier is not unique");
                goto error;
            }
        }
    }

    free(uniq_begin);
    return EXIT_SUCCESS;

error:

    free(uniq_s->leafs);
    free(uniq_begin);

    return EXIT_FAILURE;
}

/**
 * @brief Resolve (fill) a grouping in an uses. Logs directly.
 *
 * @param[in] uses The uses in question.
 * @param[in] line The line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise.
 */
static int
resolve_grouping(struct lys_node_uses *uses, uint32_t line)
{
    struct lys_module *module = uses->module;
    const char *prefix, *name;
    int i, pref_len, nam_len;
    struct lys_node *start;

    /* parse the identifier, it must be parsed on one call */
    if ((i = parse_node_identifier(uses->name, &prefix, &pref_len, &name, &nam_len)) < 1) {
        LOGVAL(LYE_INCHAR, line, uses->name[-i], &uses->name[-i]);
        return EXIT_FAILURE;
    } else if (uses->name[i]) {
        LOGVAL(LYE_INCHAR, line, uses->name[i], &uses->name[i]);
        return EXIT_FAILURE;
    }

    if (!prefix) {
        /* search in local tree hierarchy */
        if (!uses->parent) {
            start = (struct lys_node *)uses;
            while (start->prev->next) {
                start = start->prev;
            }
        } else {
            start = uses->parent->child;
        }
        while (start) {
            uses->grp = (struct lys_node_grp *)resolve_sibling(module, start, prefix, pref_len, name, nam_len, LYS_GROUPING);
            if (uses->grp) {
                return EXIT_SUCCESS;
            }
            start = start->parent;
        }
    }

    LOGVAL(LYE_INRESOLV, line, "grouping", uses->name);
    return EXIT_FAILURE;
}

/**
 * @brief Resolve (find) a feature definition. Logs directly.
 *
 * @param[in] name Feature name.
 * @param[in] module Module to search in.
 * @param[in] line The line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise.
 */
static struct lys_feature *
resolve_feature(const char *name, struct lys_module *module, uint32_t line)
{
    const char *prefix;
    uint32_t prefix_len = 0;
    int i, j;

    assert(name);
    assert(module);

    /* check prefix */
    prefix = name;
    name = strchr(prefix, ':');
    if (name) {
        /* there is prefix */
        prefix_len = name - prefix;
        name++;

        /* check whether the prefix points to the current module */
        if (!strncmp(prefix, module->prefix, prefix_len) && !module->prefix[prefix_len]) {
            /* then ignore prefix and works as there is no prefix */
            prefix_len = 0;
        }
    } else {
        /* no prefix, set pointers correctly */
        name = prefix;
    }

    if (prefix_len) {
        /* search in imported modules */
        module = resolve_prefixed_module(module, prefix, prefix_len);
        if (!module) {
            /* identity refers unknown data model */
            LOGVAL(LYE_INPREF, line, prefix);
            return NULL;
        }
    } else {
        /* search in submodules */
        for (i = 0; i < module->inc_size; i++) {
            for (j = 0; j < module->inc[i].submodule->features_size; j++) {
                if (!strcmp(name, module->inc[i].submodule->features[j].name)) {
                    return &(module->inc[i].submodule->features[j]);
                }
            }
        }
    }

    /* search in the identified module */
    for (j = 0; j < module->features_size; j++) {
        if (!strcmp(name, module->features[j].name)) {
            return &module->features[j];
        }
    }

    /* not found */
    LOGVAL(LYE_INRESOLV, line, "feature", prefix);
    return NULL;
}

/**
 * @brief Resolve (find) a valid sibling. Does not log.
 *
 * Valid child means a schema pointer to a node that is part of
 * the data meaning uses are skipped. Includes module comparison
 * (can handle augments). Includes are also searched if siblings
 * are top-level nodes.
 *
 * @param[in] mod Main module. Prefix is considered to be from this module.
 * @param[in] siblings Siblings to consider. They are first adjusted to
 *                     point to the first sibling.
 * @param[in] prefix Node prefix.
 * @param[in] pref_len Node prefix length.
 * @param[in] name Node name.
 * @param[in] nam_len Node name length.
 * @param[in] type ORed desired type of the node. 0 means any type.
 *
 * @return Node of the desired type, NULL if no matching node was found.
 */
struct lys_node *
resolve_sibling(struct lys_module *mod, struct lys_node *siblings, const char *prefix, int pref_len, const char *name,
                int nam_len, LYS_NODE type)
{
    struct lys_node *node, *result, *old_siblings = NULL;
    struct lys_module *prefix_mod, *cur_mod;
    int in_submod;

    assert(mod && siblings && name);
    assert(!(type & LYS_USES));

    /* find the beginning */
    while (siblings->prev->next) {
        siblings = siblings->prev;
    }

    /* fill the name length in case the caller is so indifferent */
    if (!nam_len) {
        nam_len = strlen(name);
    }

    /* we start with the module itself, submodules come later */
    in_submod = 0;

    /* set prefix_mod correctly */
    if (prefix) {
        prefix_mod = resolve_prefixed_module(mod, prefix, pref_len);
        if (!prefix_mod) {
            return NULL;
        }
        cur_mod = prefix_mod;
        /* it is our module */
        if (cur_mod != mod) {
            old_siblings = siblings;
            siblings = cur_mod->data;
        }
    } else {
        prefix_mod = mod;
        if (prefix_mod->type) {
            prefix_mod = ((struct lys_submodule *)prefix_mod)->belongsto;
        }
        cur_mod = prefix_mod;
    }

    while (1) {
        /* try to find the node */
        LY_TREE_FOR(siblings, node) {
            if (node->nodetype == LYS_USES) {
                /* an unresolved uses, we can still find it elsewhere */
                if (!node->child) {
                    continue;
                }

                /* search recursively */
                result = resolve_sibling(mod, node->child, prefix, pref_len, name, nam_len, type);
                if (result) {
                    return result;
                }
            }

            if (!type || (node->nodetype & type)) {
                /* module check */
                if (!node->module->type) {
                    if (cur_mod != node->module) {
                        continue;
                    }
                } else {
                    if (cur_mod != ((struct lys_submodule *)node->module)->belongsto) {
                        continue;
                    }
                }

                /* direct name check */
                if (node->name == name || (!strncmp(node->name, name, nam_len) && !node->name[nam_len])) {
                    return node;
                }
            }
        }

        /* The original siblings may be valid,
         * it's a special case when we're looking
         * for a node from augment.
         */
        if (old_siblings) {
            siblings = old_siblings;
            old_siblings = NULL;
            continue;
        }

        /* we're not top-level, search ended */
        if (siblings->parent) {
            break;
        }

        /* let's try the submodules */
        if (in_submod == prefix_mod->inc_size) {
            break;
        }
        cur_mod = (struct lys_module *)prefix_mod->inc[in_submod].submodule;
        siblings = cur_mod->data;
        ++in_submod;
    }

    return NULL;
}

/**
 * @brief Resolve (find) a schema node based on a schema-nodeid. Does not log.
 *
 * node_type - LYS_AUGMENT (searches also RPCs and notifications)
 *           - LYS_USES    (only descendant-schema-nodeid allowed, ".." not allowed, always return a grouping)
 *           - LYS_CHOICE  (search only start->child, only descendant-schema-nodeid allowed)
 *           - LYS_LEAF    (like LYS_USES, but always returns a data node)
 *
 * If id is absolute, start is ignored. If id is relative, start must be the first child to be searched
 * continuing with its siblings.
 *
 * @param[in] id Schema-nodeid string.
 * @param[in] start Start of the relative search.
 * @param[in] mod Module in question.
 * @param[in] node_type Decides how to modify the search.
 *
 * @return Matching node, NULL on fail.
 */
struct lys_node *
resolve_schema_nodeid(const char *id, struct lys_node *start, struct lys_module *mod, LYS_NODE node_type)
{
    const char *name, *prefix;
    struct lys_node *sibling;
    int ret, nam_len, pref_len, is_relative = -1;
    struct lys_module *prefix_mod, *start_mod;
    /* 0 - in module, 1 - in 1st submodule, 2 - in 2nd submodule, ... */
    uint8_t in_submod = 0;
    /* 0 - in data, 1 - in RPCs, 2 - in notifications (relevant only with LYS_AUGMENT) */
    uint8_t in_mod_part = 0;

    assert(mod);
    assert(id);

    if ((ret = parse_schema_nodeid(id, &prefix, &pref_len, &name, &nam_len, &is_relative)) < 1) {
        return NULL;
    }
    id += ret;

    if (!is_relative && (node_type & (LYS_USES | LYS_CHOICE | LYS_LEAF))) {
        return NULL;
    }

    /* absolute-schema-nodeid */
    if (!is_relative) {
        if (prefix) {
            start_mod = resolve_prefixed_module(mod, prefix, pref_len);
            if (!start_mod) {
                return NULL;
            }
            start = start_mod->data;
        } else {
            start = mod->data;
            start_mod = mod;
        }
    /* descendant-schema-nodeid */
    } else {
        if (start) {
            start_mod = start->module;
        } else {
            start_mod = mod;
        }
    }

    while (1) {
        sibling = NULL;
        LY_TREE_FOR(start, sibling) {
            /* name match */
            if (((sibling->nodetype != LYS_GROUPING) || (node_type == LYS_USES))
                    && ((sibling->name && !strncmp(name, sibling->name, nam_len) && !sibling->name[nam_len])
                    || (!strncmp(name, "input", 5) && (nam_len == 5) && (sibling->nodetype == LYS_INPUT))
                    || (!strncmp(name, "output", 6) && (nam_len == 6) && (sibling->nodetype == LYS_OUTPUT)))) {

                /* prefix match check */
                if (prefix) {
                    prefix_mod = resolve_prefixed_module(mod, prefix, pref_len);
                    if (!prefix_mod) {
                        return NULL;
                    }
                } else {
                    prefix_mod = mod;
                    if (prefix_mod->type) {
                        prefix_mod = ((struct lys_submodule *)prefix_mod)->belongsto;
                    }
                }

                /* modules need to always be checked, we want to skip augments */
                if (!sibling->module->type) {
                    if (prefix_mod != sibling->module) {
                        continue;
                    }
                } else {
                    if (prefix_mod != ((struct lys_submodule *)sibling->module)->belongsto) {
                        continue;
                    }
                }

                /* the result node? */
                if (!id[0]) {
                    /* we're looking only for groupings, this is a data node */
                    if ((node_type == LYS_USES) && (sibling->nodetype != LYS_GROUPING)) {
                        continue;
                    }
                    return sibling;
                }

                /* we're looking for a grouping (node_type == LYS_USES),
                 * but this isn't it, we cannot search inside
                 */
                if (sibling->nodetype == LYS_GROUPING) {
                    continue;
                }

                /* check for shorthand cases - then 'start' does not change */
                if (!sibling->parent || (sibling->parent->nodetype != LYS_CHOICE)
                        || (sibling->nodetype == LYS_CASE)) {
                    start = sibling->child;
                }
                break;
            }
        }

        /* we did not find the case in direct siblings */
        if (node_type == LYS_CHOICE) {
            return NULL;
        }

        /* no match */
        if (!sibling) {
            /* on augment search also RPCs and notifications, if we are in top-level */
            if ((node_type == LYS_AUGMENT) && (!start || !start->parent)) {
                /* we have searched all the data nodes */
                if (in_mod_part == 0) {
                    if (!in_submod) {
                        start = start_mod->rpc;
                    } else {
                        start = start_mod->inc[in_submod-1].submodule->rpc;
                    }
                    in_mod_part = 1;
                    continue;
                }
                /* we have searched all the RPCs */
                if (in_mod_part == 1) {
                    if (!in_submod) {
                        start = start_mod->notif;
                    } else {
                        start = start_mod->inc[in_submod-1].submodule->notif;
                    }
                    in_mod_part = 2;
                    continue;
                }
                /* we have searched all the notifications, nothing else to search in this module */
            }

            /* are we done with the included submodules as well? */
            if (in_submod == start_mod->inc_size) {
                return NULL;
            }

            /* we aren't, check the next one */
            ++in_submod;
            in_mod_part = 0;
            start = start_mod->inc[in_submod-1].submodule->data;
            continue;
        }

        /* we found our submodule */
        if (in_submod) {
            start_mod = (struct lys_module *)start_mod->inc[in_submod-1].submodule;
            in_submod = 0;
        }

        if ((ret = parse_schema_nodeid(id, &prefix, &pref_len, &name, &nam_len, &is_relative)) < 1) {
            return NULL;
        }
        id += ret;
    }

    /* cannot get here */
    return NULL;
}

/**
 * @brief Resolve (find) a data node. Does not log.
 *
 * @param[in] prefix Prefix of the data node.
 * @param[in] pref_len Length of the prefix.
 * @param[in] name Name of the data node.
 * @param[in] nam_len Length of the name.
 * @param[in] data_source Data node that defines the prefix and the name,
 *                        to find the correct module.
 * @param[in,out] parents Resolved nodes. If there are some parents,
 *                        they are replaced (!!) with the resolvents.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise.
 */
static int
resolve_data(struct lys_module *mod, const char *name, int nam_len, struct lyd_node *start, struct unres_data **parents)
{
    struct unres_data *item, *par_iter, *prev = NULL;
    struct lyd_node *node;
    int flag;

    if (!*parents) {
        *parents = malloc(sizeof **parents);
        (*parents)->dnode = NULL;
        (*parents)->next = NULL;
    }
    for (par_iter = *parents; par_iter; ) {
        if (par_iter->dnode && (par_iter->dnode->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            /* skip */
            continue;
        }
        flag = 0;
        LY_TREE_FOR(par_iter->dnode ? par_iter->dnode->child : start, node) {
            if (node->schema->module == mod && !strncmp(node->schema->name, name, nam_len)
                    && node->schema->name[nam_len] == '\0') {
                /* matching target */
                if (!flag) {
                    /* replace leafref instead of the current parent */
                    par_iter->dnode = node;
                    flag = 1;
                } else {
                    /* multiple matching, so create new leafref structure */
                    item = malloc(sizeof *item);
                    item->dnode = node;
                    item->next = par_iter->next;
                    par_iter->next = item;
                    par_iter = par_iter->next;
                }
            }
        }

        if (!flag) {
            /* remove item from the parents list */
            if (prev) {
                prev->next = par_iter->next;
                free(par_iter);
                par_iter = prev->next;
            } else {
                item = par_iter->next;
                free(par_iter);
                par_iter = *parents = item;
            }
        } else {
            prev = par_iter;
            par_iter = par_iter->next;
        }
    }

    return *parents ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* does not log */
static int
resolve_data_nodeid(const char *prefix, int pref_len, const char *name, int name_len, struct lyd_node *start,
                    struct unres_data **parents)
{
    struct lys_module *mod;

    if (prefix) {
        /* we have prefix, find appropriate module */
        mod = resolve_prefixed_module(start->schema->module, prefix, pref_len);
        if (!mod) {
            /* invalid prefix */
            return EXIT_FAILURE;
        }
    } else {
        /* no prefix, module is the same as of current node */
        mod = start->schema->module;
    }

    return resolve_data(mod, name, name_len, start, parents);
}

/**
 * @brief Resolve a path predicate (leafref) in data context. Logs directly.
 *
 * @param[in] pred Predicate in question.
 * @param[in,out] node_match Nodes satisfying the restriction
 *                           without the predicate. Nodes not
 *                           satisfying the predicate are removed.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise.
 */
static int
resolve_path_predicate_data(const char *pred, struct unres_data **node_match, uint32_t line)
{
    /* ... /node[source = destination] ... */
    struct unres_data *source_match, *dest_match, *node, *node_prev = NULL;
    const char *path_key_expr, *source, *sour_pref, *dest, *dest_pref;
    int pke_len, sour_len, sour_pref_len, dest_len, dest_pref_len, parsed = 0, pke_parsed = 0;
    int has_predicate, dest_parent_times, i;

    do {
        if ((i = parse_path_predicate(pred, &sour_pref, &sour_pref_len, &source, &sour_len, &path_key_expr,
                                      &pke_len, &has_predicate)) < 1) {
            LOGVAL(LYE_INCHAR, line, pred[-i], pred-i);
            return -parsed+i;
        }
        parsed += i;
        pred += i;

        for (node = *node_match; node;) {
            /* source */
            source_match = NULL;
            /* must be leaf (key of a list) */
            if (resolve_data_nodeid(sour_pref, sour_pref_len, source, sour_len, node->dnode, &source_match)
                    || !source_match || source_match->next
                    || (source_match->dnode->schema->nodetype != LYS_LEAF)) {
                LOGVAL(LYE_LINE, line);
                /* general error, the one written later will suffice */
                return -parsed;
            }

            /* destination */
            dest_match = calloc(1, sizeof *dest_match);
            dest_match->dnode = node->dnode;
            if ((i = parse_path_key_expr(path_key_expr, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                            &dest_parent_times)) < 1) {
                LOGVAL(LYE_INCHAR, line, path_key_expr[-i], path_key_expr-i);
                return -parsed+i;
            }
            pke_parsed += i;
            for (i = 0; i < dest_parent_times; ++i) {
                dest_match->dnode = dest_match->dnode->parent;
                if (!dest_match->dnode) {
                    free(dest_match);
                    LOGVAL(LYE_LINE, line);
                    /* general error, the one written later will suffice */
                    return -parsed;
                }
            }
            while (1) {
                if (resolve_data_nodeid(dest_pref, dest_pref_len, dest, dest_len, dest_match->dnode, &dest_match)
                        || !dest_match->dnode || dest_match->next) {
                    free(dest_match);
                    LOGVAL(LYE_LINE, line);
                    /* general error, the one written later will suffice */
                    return -parsed;
                }

                if (pke_len == pke_parsed) {
                    break;
                }
                if ((i = parse_path_key_expr(path_key_expr+pke_parsed, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                             &dest_parent_times)) < 1) {
                    LOGVAL(LYE_INCHAR, line, path_key_expr[-i], path_key_expr-i);
                    return -parsed+i;
                }
                pke_parsed += i;
            }

            /* check match between source and destination nodes */
            if (((struct lys_node_leaf *)source_match->dnode->schema)->type.base
                    != ((struct lys_node_leaf *)dest_match->dnode->schema)->type.base) {
                goto remove_leafref;
            }

            if (((struct lyd_node_leaf *)source_match->dnode)->value_str
                    != ((struct lyd_node_leaf *)dest_match->dnode)->value_str) {
                goto remove_leafref;
            }

            /* leafref is ok, continue check with next leafref */
            node_prev = node;
            node = node->next;
            continue;

remove_leafref:
            /* does not fulfill conditions, remove leafref record */
            if (node_prev) {
                node_prev->next = node->next;
                free(node);
                node = node_prev->next;
            } else {
                node = (*node_match)->next;
                free(*node_match);
                *node_match = node;
            }
        }
    } while (has_predicate);

    return parsed;
}

/**
 * @brief Resolve a path (leafref) in data context. Logs directly.
 *
 * @param[in] unres Nodes matching the schema path.
 * @param[in] path Path in question.
 * @param[in,out] ret Matching nodes.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise.
 */
int
resolve_path_arg_data(struct unres_data *unres, const char *path, struct unres_data **ret)
{
    struct lyd_node *data = NULL;
    struct unres_data *riter = NULL, *raux;
    const char *prefix, *name;
    int pref_len, nam_len, has_predicate, parent_times, i;

    *ret = NULL;
    parent_times = 0;

    /* searching for nodeset */
    do {
        if ((i = parse_path_arg(path, &prefix, &pref_len, &name, &nam_len, &parent_times, &has_predicate)) < 1) {
            LOGVAL(LYE_INCHAR, LOGLINE(unres), path[-i], path-i);
            goto error;
        }
        path += i;

        if (!*ret) {
            *ret = calloc(1, sizeof **ret);
            for (i = 0; i < parent_times; ++i) {
                /* relative path */
                if (!*ret) {
                    /* error, too many .. */
                    LOGVAL(LYE_INVAL, LOGLINE(unres), path, unres->dnode->schema->name);
                    goto error;
                } else if (!(*ret)->dnode) {
                    /* first .. */
                    data = (*ret)->dnode = unres->dnode->parent;
                } else if (!(*ret)->dnode->parent) {
                    /* we are in root */
                    free(*ret);
                    *ret = NULL;
                } else {
                    /* multiple .. */
                    data = (*ret)->dnode = (*ret)->dnode->parent;
                }
            }

            /* absolute path */
            if (parent_times == -1) {
                for (data = unres->dnode; data->parent; data = data->parent);
                for (; data->prev->next; data = data->prev);
            }
        }

        /* node identifier */
        if (resolve_data_nodeid(prefix, pref_len, name, nam_len, data, ret)) {
            LOGVAL(LYE_INELEM_LEN, LOGLINE(unres), nam_len, name);
            goto error;
        }

        if (has_predicate) {
            /* we have predicate, so the current results must be lists */
            for (raux = NULL, riter = *ret; riter; ) {
                if (riter->dnode->schema->nodetype == LYS_LIST &&
                        ((struct lys_node_list *)riter->dnode->schema)->keys) {
                    /* leafref is ok, continue check with next leafref */
                    raux = riter;
                    riter = riter->next;
                    continue;
                }

                /* does not fulfill conditions, remove leafref record */
                if (raux) {
                    raux->next = riter->next;
                    free(riter);
                    riter = raux->next;
                } else {
                    *ret = riter->next;
                    free(riter);
                    riter = *ret;
                }
            }
            if ((i = resolve_path_predicate_data(path, ret, LOGLINE(unres))) < 1) {
                goto error;
            }
            path += i;

            if (!*ret) {
                LOGVAL(LYE_LINE, LOGLINE(unres));
                /* general error, the one written later will suffice */
                goto error;
            }
        }
    } while (path[0] != '\0');

    return EXIT_SUCCESS;

error:

    while (*ret) {
        raux = (*ret)->next;
        free(*ret);
        *ret = raux;
    }

    return EXIT_FAILURE;
}

/**
 * @brief Resolve a path (leafref) predicate in schema context. Logs directly.
 *
 * @param[in] path Path in question.
 * @param[in] mod Schema module.
 * @param[in] source_node Left operand node.
 * @param[in] dest_node Right ooperand node.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise.
 */
static int
resolve_path_predicate_schema(const char *path, struct lys_module *mod, struct lys_node *source_node,
                              struct lys_node *dest_node, uint32_t line)
{
    struct lys_node *src_node, *dst_node;
    const char *path_key_expr, *source, *sour_pref, *dest, *dest_pref;
    int pke_len, sour_len, sour_pref_len, dest_len, dest_pref_len, parsed = 0, pke_parsed = 0;
    int has_predicate, dest_parent_times = 0, i;

    do {
        if ((i = parse_path_predicate(path, &sour_pref, &sour_pref_len, &source, &sour_len, &path_key_expr,
                                      &pke_len, &has_predicate)) < 1) {
            LOGVAL(LYE_INCHAR, line, path[-i], path-i);
            return -parsed+i;
        }
        parsed += i;
        path += i;

        /* source (must be leaf) */
        src_node = resolve_sibling(mod, source_node->child, sour_pref, sour_pref_len, source, sour_len, LYS_LEAF);
        if (!src_node) {
            LOGVAL(LYE_LINE, line);
            /* general error, the one written later will suffice */
            return -parsed;
        }

        /* destination */
        if ((i = parse_path_key_expr(path_key_expr, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                     &dest_parent_times)) < 1) {
            LOGVAL(LYE_INCHAR, line, path_key_expr[-i], path_key_expr-i);
            return -parsed;
        }
        pke_parsed += i;

        /* dest_node is actually the parent of this leaf, so skip the first ".." */
        dst_node = dest_node;
        for (i = 1; i < dest_parent_times; ++i) {
            dst_node = dst_node->parent;
            if (!dst_node) {
                LOGVAL(LYE_LINE, line);
                /* general error, the one written later will suffice */
                return -parsed;
            }
        }
        while (1) {
            dst_node = resolve_sibling(mod, dst_node->child, dest_pref, dest_pref_len, dest, dest_len,
                                       LYS_CONTAINER | LYS_LIST | LYS_LEAF);
            if (!dst_node) {
                LOGVAL(LYE_LINE, line);
                /* general error, the one written later will suffice */
                return -parsed;
            }

            if (pke_len == pke_parsed) {
                break;
            }

            if ((i = parse_path_key_expr(path_key_expr+pke_parsed, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                         &dest_parent_times)) < 1) {
                LOGVAL(LYE_INCHAR, line, (path_key_expr+pke_parsed)[-i], (path_key_expr+pke_parsed)-i);
                return -parsed;
            }
            pke_parsed += i;
        }

        /* check source - dest match */
        if ((dst_node->nodetype != LYS_LEAF) || ((struct lys_node_leaf *)dst_node)->type.base
                != ((struct lys_node_leaf *)src_node)->type.base) {
            LOGVAL(LYE_LINE, line);
            /* general error, the one written later will suffice */
            return -parsed;
        }
    } while (has_predicate);

    return parsed;
}

/**
 * @brief Resolve a path (leafref) in schema context. Logs indirectly.
 *
 * @param[in] mod Module in question.
 * @param[in] path Path in question.
 * @param[in] parent_node Parent of the leafref.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise.
 */
static struct lys_node*
resolve_path_arg_schema(struct lys_module *mod, const char *path, struct lys_node *parent_node, uint32_t line)
{
    struct lys_node *node;
    const char *id, *prefix, *name;
    int pref_len, nam_len, parent_times, has_predicate;
    int i, first;

    first = 1;
    parent_times = 0;
    id = path;

    do {
        if ((i = parse_path_arg(id, &prefix, &pref_len, &name, &nam_len, &parent_times, &has_predicate)) < 1) {
            LOGVAL(LYE_INCHAR, line, id[-i], &id[-i]);
            return NULL;
        }
        id += i;

        if (first) {
            if (parent_times == -1) {
                node = mod->data;
                if (!node) {
                    LOGVAL(LYE_LINE, line);
                    /* general error, the one written later will suffice */
                    return NULL;
                }
            } else if (parent_times > 0) {
                /* node is the parent already, skip one ".." */
                node = parent_node;
                i = 0;
                while (1) {
                    if (!node) {
                        LOGVAL(LYE_LINE, line);
                        /* general error, the one written later will suffice */
                        return NULL;
                    }

                    /* this node is a wrong node, we actually need the augment target */
                    if (node->nodetype == LYS_AUGMENT) {
                        node = ((struct lys_node_augment *)node)->target;
                        if (!node) {
                            continue;
                        }
                    }

                    ++i;
                    if (i == parent_times) {
                        break;
                    }
                    node = node->parent;
                }
                node = node->child;
            } else {
                LOGINT;
                return NULL;
            }
            first = 0;
        } else {
            node = node->child;
        }

        node = resolve_sibling(mod, node, prefix, pref_len, name, nam_len, LYS_ANY & ~(LYS_GROUPING | LYS_USES));
        if (!node) {
            LOGVAL(LYE_LINE, line);
            /* general error, the one written later will suffice */
            return NULL;
        }

        if (has_predicate) {
            /* we have predicate, so the current result must be list */
            if (node->nodetype != LYS_LIST) {
                LOGVAL(LYE_LINE, line);
                /* general error, the one written later will suffice */
                return NULL;
            }

            if ((i = resolve_path_predicate_schema(id, mod, node, parent_node, line)) < 1) {
                return NULL;
            }
            id += i;
        }
    } while (id[0]);

    /* the target must be leaf or leaf-list */
    if (!(node->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        LOGVAL(LYE_LINE, line);
        /* general error, the one written later will suffice */
        return NULL;
    }

    return node;
}

/**
 * @brief Resolve instance-identifier predicate. Does not log.
 *
 * @param[in] pred Predicate in question.
 * @param[in,out] node_match Nodes matching the restriction without
 *                           the predicate. Nodes not satisfying
 *                           the predicate are removed.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise.
 */
static int
resolve_predicate(const char *pred, struct unres_data **node_match)
{
    /* ... /node[target = value] ... */
    struct unres_data *target_match, *node, *node_prev = NULL, *tmp;
    const char *prefix, *name, *value;
    int pref_len, nam_len, val_len, i, has_predicate, cur_idx, idx, parsed;

    idx = -1;
    parsed = 0;

    do {
        if ((i = parse_predicate(pred, &prefix, &pref_len, &name, &nam_len, &value, &val_len, &has_predicate)) < 1) {
            return -parsed+i;
        }
        parsed += i;
        pred += i;

        if (isdigit(name[0])) {
            idx = atoi(name);
        }

        for (cur_idx = 0, node = *node_match; node; ++cur_idx) {
            /* target */
            target_match = NULL;
            if ((name[0] == '.') || !value) {
                target_match = calloc(1, sizeof *target_match);
                target_match->dnode = node->dnode;
            } else if (resolve_data_nodeid(prefix, pref_len, name, nam_len, node->dnode, &target_match)) {
                goto remove_instid;
            }

            /* check that we have the correct type */
            if (name[0] == '.') {
                if (node->dnode->schema->nodetype != LYS_LEAFLIST) {
                    goto remove_instid;
                }
            } else if (value) {
                if (node->dnode->schema->nodetype != LYS_LIST) {
                    goto remove_instid;
                }
            }

            if ((value && (strncmp(((struct lyd_node_leaf *)target_match->dnode)->value_str, value, val_len)
                    || ((struct lyd_node_leaf *)target_match->dnode)->value_str[val_len]))
                    || (!value && (idx != cur_idx))) {
                goto remove_instid;
            }

            while (target_match) {
                tmp = target_match->next;
                free(target_match);
                target_match = tmp;
            }

            /* leafref is ok, continue check with next leafref */
            node_prev = node;
            node = node->next;
            continue;

remove_instid:
            while (target_match) {
                tmp = target_match->next;
                free(target_match);
                target_match = tmp;
            }

            /* does not fulfill conditions, remove leafref record */
            if (node_prev) {
                node_prev->next = node->next;
                free(node);
                node = node_prev->next;
            } else {
                node = (*node_match)->next;
                free(*node_match);
                *node_match = node;
            }
        }
    } while (has_predicate);

    return parsed;
}

/**
 * @brief Resolve instance-identifier. Logs directly.
 *
 * @param[in] data Any node in the data tree, used to get a data tree root and context
 * @param[in] path Instance-identifier node value.
 * @param[in] line Source line for error messages.
 *
 * @return Matching node or NULL if no such a node exists. If error occurs, NULL is returned and ly_errno is set.
 */
struct lyd_node *
resolve_instid(struct lyd_node *data, const char *path, int line)

{
    int i = 0, j;
    struct lyd_node *result = NULL;
    struct lys_module *mod = NULL;
    struct ly_ctx *ctx = data->schema->module->ctx;
    const char *prefix, *name;
    char *str;
    int pref_len, name_len, has_predicate;
    struct unres_data *workingnodes = NULL;
    struct unres_data *riter = NULL, *raux;

    /* we need root to resolve absolute path */
    for (; data->parent; data = data->parent);
    for (; data->prev->next; data = data->prev);

    /* search for the instance node */
    while (path[i]) {
        j = parse_instance_identifier(&path[i], &prefix, &pref_len, &name, &name_len, &has_predicate);
        if (j <= 0) {
            LOGVAL(LYE_INCHAR, line, path[i-j], &path[i-j]);
            goto error;
        }
        i += j;

        if (prefix) {
            str = strndup(prefix, pref_len);
            mod = ly_ctx_get_module(ctx, str, NULL);
            free(str);
        }

        if (!mod) {
            /* no instance exists */
            return NULL;
        }

        if (resolve_data(mod, name, name_len, data, &workingnodes)) {
            /* no instance exists */
            return NULL;
        }

        if (has_predicate) {
            /* we have predicate, so the current results must be list or leaf-list */
            for (raux = NULL, riter = workingnodes; riter; ) {
                if ((riter->dnode->schema->nodetype == LYS_LIST &&
                        ((struct lys_node_list *)riter->dnode->schema)->keys)
                        || (riter->dnode->schema->nodetype == LYS_LEAFLIST)) {
                    /* instid is ok, continue check with next instid */
                    raux = riter;
                    riter = riter->next;
                    continue;
                }

                /* does not fulfill conditions, remove inst record */
                if (raux) {
                    raux->next = riter->next;
                    free(riter);
                    riter = raux->next;
                } else {
                    workingnodes = riter->next;
                    free(riter);
                    riter = workingnodes;
                }
            }

            j = resolve_predicate(&path[i], &workingnodes);
            if (j < 1) {
                LOGVAL(LYE_INPRED, line, &path[i-j]);
                goto error;
            }
            i += j;;

            if (!workingnodes) {
                /* no instance exists */
                return NULL;
            }
        }
    }

    if (!workingnodes) {
        /* no instance exists */
        return NULL;
    } else if (workingnodes->next) {
        /* instance identifier must resolve to a single node */
        LOGVAL(LYE_TOOMANY, line, path, "data tree");

        /* cleanup */
        while (workingnodes) {
            raux = workingnodes->next;
            free(workingnodes);
            workingnodes = raux;
        }

        return NULL;
    } else {
        /* we have required result, remember it and cleanup */
        result = workingnodes->dnode;
        free(workingnodes);

        return result;
    }

error:

    /* cleanup */
    while (workingnodes) {
        raux = workingnodes->next;
        free(workingnodes);
        workingnodes = raux;
    }

    return NULL;
}

/**
 * @brief Passes config flag down to children. Does not log.
 *
 * @param[in] node Parent node.
 */
static void
inherit_config_flag(struct lys_node *node)
{
    LY_TREE_FOR(node, node) {
        node->flags |= node->parent->flags & LYS_CONFIG_MASK;
        inherit_config_flag(node->child);
    }
}

/**
 * @brief Resolve augment target, Does not log.
 *
 * @param[in] aug Augment in question.
 * @param[in] siblings Nodes where to start the search in.
 * @param[in] module Main module.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise.
 */
int
resolve_augment(struct lys_node_augment *aug, struct lys_node *siblings, struct lys_module *module)
{
    struct lys_node *sub, *aux;

    assert(module);

    /* resolve target node */
    aug->target = resolve_schema_nodeid(aug->target_name, siblings, module, LYS_AUGMENT);
    if (!aug->target) {
        return EXIT_FAILURE;
    }

    if (!aug->child) {
        /* nothing to do */
        return EXIT_SUCCESS;
    }

    /* inherit config information from parent, augment does not have
     * config property, but we need to keep the information for subelements
     */
    aug->flags |= aug->target->flags & LYS_CONFIG_MASK;
    LY_TREE_FOR(aug->child, sub) {
        inherit_config_flag(sub);
    }

    /* TODO check identifier uniquness as in lys_node_addchild() */
    /* reconnect augmenting data into the target - add them to the target child list */
    if (aug->target->child) {
        aux = aug->target->child->prev; /* remember current target's last node */
        aux->next = aug->child;         /* connect augmenting data after target's last node */
        aug->target->child->prev = aug->child->prev; /* new target's last node is last augmenting node */
        aug->child->prev = aux;         /* finish connecting of both child lists */
    } else {
        aug->target->child = aug->child;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve uses, apply augments, refines. Logs directly.
 *
 * @param[in] uses Uses in question.
 * @param[in,out] unres List of unresolved items.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURER otherwise.
 */
int
resolve_uses(struct lys_node_uses *uses, struct unres_schema *unres, uint32_t line)
{
    struct ly_ctx *ctx;
    struct lys_node *node = NULL, *node_aux;
    struct lys_refine *rfn;
    struct lys_restr *newmust;
    int i, j;
    uint8_t size;

    assert(uses->grp);

    /* copy the data nodes from grouping into the uses context */
    LY_TREE_FOR(uses->grp->child, node) {
        node_aux = lys_node_dup(uses->module, node, uses->flags, uses->nacm, 1, unres);
        if (!node_aux) {
            LOGVAL(LYE_SPEC, line, "Copying data from grouping failed.");
            return EXIT_FAILURE;
        }
        if (lys_node_addchild((struct lys_node *)uses, NULL, node_aux)) {
            /* error logged */
            lys_node_free(node_aux);
            return EXIT_FAILURE;
        }
    }
    ctx = uses->module->ctx;

    /* apply refines */
    for (i = 0; i < uses->refine_size; i++) {
        rfn = &uses->refine[i];
        node = resolve_schema_nodeid(rfn->target_name, uses->child, uses->module, LYS_LEAF);
        if (!node) {
            LOGVAL(LYE_INARG, line, rfn->target_name, "refine");
            return EXIT_FAILURE;
        }

        if (rfn->target_type && !(node->nodetype & rfn->target_type)) {
            LOGVAL(LYE_SPEC, line, "Refine substatements not applicable to the target-node.");
            return EXIT_FAILURE;
        }

        /* description on any nodetype */
        if (rfn->dsc) {
            lydict_remove(ctx, node->dsc);
            node->dsc = lydict_insert(ctx, rfn->dsc, 0);
        }

        /* reference on any nodetype */
        if (rfn->ref) {
            lydict_remove(ctx, node->ref);
            node->ref = lydict_insert(ctx, rfn->ref, 0);
        }

        /* config on any nodetype */
        if (rfn->flags & LYS_CONFIG_MASK) {
            node->flags &= ~LYS_CONFIG_MASK;
            node->flags |= (rfn->flags & LYS_CONFIG_MASK);
        }

        /* default value ... */
        if (rfn->mod.dflt) {
            if (node->nodetype == LYS_LEAF) {
                /* leaf */
                lydict_remove(ctx, ((struct lys_node_leaf *)node)->dflt);
                ((struct lys_node_leaf *)node)->dflt = lydict_insert(ctx, rfn->mod.dflt, 0);
            } else if (node->nodetype == LYS_CHOICE) {
                /* choice */
                ((struct lys_node_choice *)node)->dflt = resolve_schema_nodeid(rfn->mod.dflt, node->child, node->module, LYS_CHOICE);
                if (!((struct lys_node_choice *)node)->dflt) {
                    LOGVAL(LYE_INARG, line, rfn->mod.dflt, "default");
                    return EXIT_FAILURE;
                }
            }
        }

        /* mandatory on leaf, anyxml or choice */
        if (rfn->flags & LYS_MAND_MASK) {
            if (node->nodetype & (LYS_LEAF | LYS_ANYXML | LYS_CHOICE)) {
                /* remove current value */
                node->flags &= ~LYS_MAND_MASK;

                /* set new value */
                node->flags |= (rfn->flags & LYS_MAND_MASK);
            }
        }

        /* presence on container */
        if ((node->nodetype & LYS_CONTAINER) && rfn->mod.presence) {
            lydict_remove(ctx, ((struct lys_node_container *)node)->presence);
            ((struct lys_node_container *)node)->presence = lydict_insert(ctx, rfn->mod.presence, 0);
        }

        /* min/max-elements on list or leaf-list */
        /* magic - bit 3 in flags means min set, bit 4 says max set */
        if (node->nodetype == LYS_LIST) {
            if (rfn->flags & 0x04) {
                ((struct lys_node_list *)node)->min = rfn->mod.list.min;
            }
            if (rfn->flags & 0x08) {
                ((struct lys_node_list *)node)->max = rfn->mod.list.max;
            }
        } else if (node->nodetype == LYS_LEAFLIST) {
            if (rfn->flags & 0x04) {
                ((struct lys_node_leaflist *)node)->min = rfn->mod.list.min;
            }
            if (rfn->flags & 0x08) {
                ((struct lys_node_leaflist *)node)->max = rfn->mod.list.max;
            }
        }

        /* must in leaf, leaf-list, list, container or anyxml */
        if (rfn->must_size) {
            size = ((struct lys_node_leaf *)node)->must_size + rfn->must_size;
            newmust = realloc(((struct lys_node_leaf *)node)->must, size * sizeof *rfn->must);
            if (!newmust) {
                LOGMEM;
                return EXIT_FAILURE;
            }
            for (i = 0, j = ((struct lys_node_leaf *)node)->must_size; i < rfn->must_size; i++, j++) {
                newmust[j].expr = lydict_insert(ctx, rfn->must[i].expr, 0);
                newmust[j].dsc = lydict_insert(ctx, rfn->must[i].dsc, 0);
                newmust[j].ref = lydict_insert(ctx, rfn->must[i].ref, 0);
                newmust[j].eapptag = lydict_insert(ctx, rfn->must[i].eapptag, 0);
                newmust[j].emsg = lydict_insert(ctx, rfn->must[i].emsg, 0);
            }

            ((struct lys_node_leaf *)node)->must = newmust;
            ((struct lys_node_leaf *)node)->must_size = size;
        }
    }

    /* apply augments */
    for (i = 0; i < uses->augment_size; i++) {
        if (resolve_augment(&uses->augment[i], uses->child, uses->module)) {
            LOGVAL(LYE_INRESOLV, line, "augment", uses->augment[i].target_name);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve base identity recursively. Does not log.
 *
 * @param[in] module Main module.
 * @param[in] ident Identity in question.
 * @param[in] basename Base name of the identity.
 *
 * @return Pointer to the identity, NULL on error.
 */
static struct lys_ident *
resolve_base_ident_sub(struct lys_module *module, struct lys_ident *ident, const char *basename)
{
    uint32_t i, j;
    struct lys_ident *base_iter = NULL;
    struct lys_ident_der *der;

    /* search module */
    for (i = 0; i < module->ident_size; i++) {
        if (!strcmp(basename, module->ident[i].name)) {

            if (!ident) {
                /* just search for type, so do not modify anything, just return
                 * the base identity pointer
                 */
                return &module->ident[i];
            }

            /* we are resolving identity definition, so now update structures */
            ident->base = base_iter = &module->ident[i];

            break;
        }
    }

    /* search submodules */
    if (!base_iter) {
        for (j = 0; j < module->inc_size; j++) {
            for (i = 0; i < module->inc[j].submodule->ident_size; i++) {
                if (!strcmp(basename, module->inc[j].submodule->ident[i].name)) {

                    if (!ident) {
                        return &module->inc[j].submodule->ident[i];
                    }

                    ident->base = base_iter = &module->inc[j].submodule->ident[i];
                    break;
                }
            }
        }
    }

    /* we found it somewhere */
    if (base_iter) {
        while (base_iter) {
            for (der = base_iter->der; der && der->next; der = der->next);
            if (der) {
                der->next = malloc(sizeof *der);
                der = der->next;
            } else {
                ident->base->der = der = malloc(sizeof *der);
            }
            der->next = NULL;
            der->ident = ident;

            base_iter = base_iter->base;
        }
        return ident->base;
    }

    return NULL;
}

/**
 * @brief Resolve base identity. Logs directly.
 *
 * @param[in] module Main module.
 * @param[in] ident Identity in question.
 * @param[in] basename Base name of the identity.
 * @param[in] parent Either "type" or "ident".
 * @param[in] line Line in the input file.
 *
 * @return Pointer to the base identity, NULL on error.
 */
static struct lys_ident *
resolve_base_ident(struct lys_module *module, struct lys_ident *ident, const char *basename, const char* parent,
                   uint32_t line)
{
    const char *name;
    int i, prefix_len = 0;
    struct lys_ident *result;

    /* search for the base identity */
    name = strchr(basename, ':');
    if (name) {
        /* set name to correct position after colon */
        prefix_len = name - basename;
        name++;

        if (!strncmp(basename, module->prefix, prefix_len) && !module->prefix[prefix_len]) {
            /* prefix refers to the current module, ignore it */
            prefix_len = 0;
        }
    } else {
        name = basename;
    }

    if (prefix_len) {
        /* get module where to search */
        module = resolve_prefixed_module(module, basename, prefix_len);
        if (!module) {
            /* identity refers unknown data model */
            LOGVAL(LYE_INPREF, line, basename);
            return NULL;
        }
    } else {
        /* search in submodules */
        for (i = 0; i < module->inc_size; i++) {
            result = resolve_base_ident_sub((struct lys_module *)module->inc[i].submodule, ident, name);
            if (result) {
                return result;
            }
        }
    }

    /* search in the identified module */
    result = resolve_base_ident_sub(module, ident, name);
    if (!result) {
        LOGVAL(LYE_INARG, line, basename, parent);
    }

    return result;
}

/**
 * @brief Resolve identityref. Does not log.
 *
 * @param[in] base Base identity.
 * @param[in] name Identityref name.
 * @param[in] ns Namespace of the identityref.
 *
 * @return Pointer to the identity resolvent, NULL on error.
 */
struct lys_ident *
resolve_identityref(struct lys_ident *base, const char *name, const char *ns)
{
    struct lys_ident_der *der;

    if (!base || !name || !ns) {
        return NULL;
    }

    for(der = base->der; der; der = der->next) {
        if (!strcmp(der->ident->name, name) && ns == der->ident->module->ns) {
            /* we have match */
            return der->ident;
        }
    }

    /* not found */
    return NULL;
}

/**
 * @brief Resolve unres identity. Logs directly.
 *
 * @param[in] mod Main module.
 * @param[in] ident Identity in question.
 * @param[in] base_name Base name of the identity.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
static int
resolve_unres_ident(struct lys_module *mod, struct lys_ident *ident, const char *base_name, uint32_t line)
{
    if (resolve_base_ident(mod, ident, base_name, "ident", line)) {
        return EXIT_SUCCESS;
    }

    LOGVAL(LYE_INRESOLV, (line == UINT_MAX ? line : 0), "identity", base_name);
    return EXIT_FAILURE;
}

/**
 * @brief Resolve unres identityref. Logs directly.
 *
 * @param[in] mod Main module.
 * @param[in] type Type in question.
 * @param[in] base_name Base name of the identity.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
static int
resolve_unres_type_identref(struct lys_module *mod, struct lys_type *type, const char *base_name, uint32_t line)
{
    type->info.ident.ref = resolve_base_ident(mod, NULL, base_name, "type", line);
    if (type->info.ident.ref) {
        return EXIT_SUCCESS;
    }

    LOGVAL(LYE_INRESOLV, (line == UINT_MAX ? line : 0), "identityref", base_name);
    return EXIT_FAILURE;
}

/**
 * @brief Resolve unres leafref. Logs directly.
 *
 * @param[in] mod Main module.
 * @param[in] type Type in question.
 * @param[in] node Leafref schema node.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
static int
resolve_unres_type_leafref(struct lys_module *mod, struct lys_type *type, struct lys_node *node, uint32_t line)
{
    type->info.lref.target = (struct lys_node_leaf *)resolve_path_arg_schema(mod, type->info.lref.path, node, line);
    if (type->info.lref.target) {
        return EXIT_SUCCESS;
    }

    LOGVAL(LYE_INRESOLV, (line == UINT_MAX ? line : 0), "leafref", type->info.lref.path);
    return EXIT_FAILURE;
}

/**
 * @brief Resolve unres derived type. Logs directly.
 *
 * @param[in] mod Main module.
 * @param[in] type Type in question.
 * @param[in] type_name Derived type name,
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
static int
resolve_unres_type_der(struct lys_module *mod, struct lys_type *type, const char *type_name, uint32_t line)
{
    type->der = resolve_superior_type(type_name, type->prefix, mod, (struct lys_node *)type->der);
    if (type->der) {
        type->base = type->der->type.base;
        return EXIT_SUCCESS;
    }

    LOGVAL(LYE_INRESOLV, line, "type", type_name);
    return EXIT_FAILURE;
}

/**
 * @brief Resolve unres if-feature. Logs directly.
 *
 * @param[in] mod Main module.
 * @param[in,out] feat_ptr Pointer to the resolved feature.
 * @param[in] feat_name Name of the feature.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
static int
resolve_unres_iffeature(struct lys_module *mod, struct lys_feature **feat_ptr, const char *feat_name, uint32_t line)
{
    *feat_ptr = resolve_feature(feat_name, mod, line);
    if (*feat_ptr) {
        return EXIT_SUCCESS;
    }

    LOGVAL(LYE_INRESOLV, (line == UINT_MAX ? line : 0), "if-feature", feat_name);
    return EXIT_FAILURE;
}

/**
 * @brief Resolve unres uses. Logs directly.
 *
 * @param[in] uses Uses in question.
 * @param[in] unres Specific unres item.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
static int
resolve_unres_uses(struct lys_node_uses *uses, struct unres_schema *unres, uint32_t line)
{
    struct lys_node *parent;

    /* HACK change unres uses count if it's in a grouping (nacm field used for it) */
    for (parent = uses->parent; parent && (parent->nodetype != LYS_GROUPING); parent = parent->parent);

    if (uses->grp || !resolve_grouping(uses, line)) {
        if (uses->grp->nacm) {
            LOGVRB("Cannot copy the grouping, it is not fully resolved yet.");
            return EXIT_FAILURE;
        } else if (!resolve_uses(uses, unres, line)) {
            /* decrease unres count only if not first try */
            if ((line < UINT_MAX) && parent) {
                if (!parent->nacm) {
                    LOGINT;
                    return EXIT_FAILURE;
                }
                --parent->nacm;
            }
            return EXIT_SUCCESS;
        }
    }

    if (parent) {
        ++parent->nacm;
    }
    LOGVAL(LYE_INRESOLV, (line == UINT_MAX ? line : 0), "uses", uses->name);
    return EXIT_FAILURE;
}

/**
 * @brief Resolve unres identity. Logs directly.
 *
 * @param[in] type Type in question.
 * @param[in] dflt Default value.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
static int
resolve_unres_type_dflt(struct lys_type *type, const char *dflt, uint32_t line)
{
    if (!check_default(type, dflt)) {
        return EXIT_SUCCESS;
    }

    LOGVAL(LYE_INRESOLV, line, "type default", dflt);
    return EXIT_FAILURE;
}

/**
 * @brief Resolve choice default. Logs directly.
 *
 * @param[in] choice Main module.
 * @param[in] dflt Default case name.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
static int
resolve_unres_choice_dflt(struct lys_module *mod, struct lys_node_choice *choice, const char *dflt, uint32_t line)
{
    choice->dflt = resolve_sibling(mod, choice->child, NULL, 0, dflt, 0, LYS_ANYXML | LYS_CASE
                                   | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST);
    if (choice->dflt) {
        return EXIT_SUCCESS;
    }

    LOGVAL(LYE_INRESOLV, line, "choice default", dflt);
    return EXIT_FAILURE;
}

/**
 * @brief Resolve unres identity. Logs directly.
 *
 * @param[in] list List in question.
 * @param[in] keys_str Keys node value.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
static int
resolve_unres_list_keys(struct lys_module *mod, struct lys_node_list *list, const char *keys_str, uint32_t line)
{
    int i, len;
    const char *value;

    for (i = 0; i < list->keys_size; ++i) {
        /* get the key name */
        if ((value = strpbrk(keys_str, " \t\n"))) {
            len = value - keys_str;
            while (isspace(value[0])) {
                value++;
            }
        } else {
            len = strlen(keys_str);
        }

        list->keys[i] = (struct lys_node_leaf *)resolve_sibling(mod, list->child, NULL, 0, keys_str, len, LYS_LEAF);

        if (check_key(list->keys[i], list->flags, list->keys, i, keys_str, len, line)) {
            LOGVAL(LYE_INRESOLV, (line == UINT_MAX ? line : 0), "list keys", keys_str);
            return EXIT_FAILURE;
        }

        /* prepare for next iteration */
        while (value && isspace(value[0])) {
            value++;
        }
        keys_str = value;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve unres unique. Logs directly.
 *
 * @param[in] uniq Unique in question.
 * @param[in] uniq_str Unique node value.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
static int
resolve_unres_list_uniq(struct lys_unique *uniq, const char *uniq_str, uint32_t line)
{
    if (!resolve_unique((struct lys_node *)uniq->leafs, uniq_str, uniq, line)) {
        return EXIT_SUCCESS;
    }

    LOGVAL(LYE_INRESOLV, (line == UINT_MAX ? line : 0), "list unique", uniq_str);
    return EXIT_FAILURE;
}

static int
resolve_unres_when(struct lys_when *UNUSED(when), struct lys_node *UNUSED(start), uint32_t UNUSED(line))
{
    /* TODO */
    return EXIT_SUCCESS;
}

static int
resolve_unres_must(struct lys_restr *UNUSED(must), struct lys_node *UNUSED(start), uint32_t UNUSED(line))
{
    /* TODO */
    return EXIT_SUCCESS;
}

/* logs indirectly
 * line == -1 means do not log, 0 means unknown */
static int
resolve_unres_item(struct lys_module *mod, void *item, enum UNRES_ITEM type, void *str_node, struct unres_schema *unres,
                   uint32_t line)
{
    int ret = EXIT_FAILURE, has_str = 0;

    switch (type) {
    case UNRES_RESOLVED:
        LOGINT;
        break;
    case UNRES_IDENT:
        ret = resolve_unres_ident(mod, item, str_node, line);
        has_str = 1;
        break;
    case UNRES_TYPE_IDENTREF:
        ret = resolve_unres_type_identref(mod, item, str_node, line);
        has_str = 1;
        break;
    case UNRES_TYPE_LEAFREF:
        ret = resolve_unres_type_leafref(mod, item, str_node, line);
        has_str = 0;
        break;
    case UNRES_TYPE_DER:
        ret = resolve_unres_type_der(mod, item, str_node, line);
        has_str = 1;
        break;
    case UNRES_IFFEAT:
        ret = resolve_unres_iffeature(mod, item, str_node, line);
        has_str = 1;
        break;
    case UNRES_USES:
        ret = resolve_unres_uses(item, unres, line);
        has_str = 0;
        break;
    case UNRES_TYPE_DFLT:
        ret = resolve_unres_type_dflt(item, str_node, line);
        /* do not remove str_node (dflt), it's in a typedef */
        has_str = 0;
        break;
    case UNRES_CHOICE_DFLT:
        ret = resolve_unres_choice_dflt(mod, item, str_node, line);
        has_str = 1;
        break;
    case UNRES_LIST_KEYS:
        ret = resolve_unres_list_keys(mod, item, str_node, line);
        has_str = 1;
        break;
    case UNRES_LIST_UNIQ:
        ret = resolve_unres_list_uniq(item, str_node, line);
        has_str = 1;
        break;
    case UNRES_WHEN:
        ret = resolve_unres_when(item, str_node, line);
        has_str = 0;
        break;
    case UNRES_MUST:
        ret = resolve_unres_must(item, str_node, line);
        has_str = 0;
        break;
    }

    if (has_str && !ret) {
        lydict_remove(mod->ctx, str_node);
    }

    return ret;
}

/* logs directly */
static void
print_unres_item_fail(void *item, enum UNRES_ITEM type, void *str_node, uint32_t line)
{
    char line_str[18];

    if (line) {
        sprintf(line_str, " (line %u)", line);
    } else {
        line_str[0] = '\0';
    }

    switch (type) {
    case UNRES_RESOLVED:
        LOGINT;
        break;
    case UNRES_IDENT:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later%s.", "identity", (char *)str_node, line_str);
        break;
    case UNRES_TYPE_IDENTREF:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later%s.", "identityref", (char *)str_node, line_str);
        break;
    case UNRES_TYPE_LEAFREF:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later%s.", "leafref", ((struct lys_type *)item)->info.lref.path, line_str);
        break;
    case UNRES_TYPE_DER:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later%s.", "derived type", (char *)str_node, line_str);
        break;
    case UNRES_IFFEAT:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later%s.", "if-feature", (char *)str_node, line_str);
        break;
    case UNRES_USES:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later%s.", "uses", ((struct lys_node_uses *)item)->name, line_str);
        break;
    case UNRES_TYPE_DFLT:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later%s.", "type default", (char *)str_node, line_str);
        break;
    case UNRES_CHOICE_DFLT:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later%s.", "choice default", (char *)str_node, line_str);
        break;
    case UNRES_LIST_KEYS:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later%s.", "list keys", (char *)str_node, line_str);
        break;
    case UNRES_LIST_UNIQ:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later%s.", "list unique", (char *)str_node, line_str);
        break;
    case UNRES_WHEN:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later%s.", "when", ((struct lys_when *)item)->cond, line_str);
        break;
    case UNRES_MUST:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later%s.", "must", ((struct lys_restr *)item)->expr, line_str);
        break;
    }
}

/* logs directly */
int
resolve_unres(struct lys_module *mod, struct unres_schema *unres)
{
    uint32_t i, resolved, unres_uses, res_uses, line;

    assert(unres);

    line = 0;
    resolved = 0;

    /* uses */
    do {
        unres_uses = 0;
        res_uses = 0;

        for (i = 0; i < unres->count; ++i) {
            if (unres->type[i] != UNRES_USES) {
                continue;
            }

#ifndef NDEBUG
            line = unres->line[i];
#endif

            ++unres_uses;
            if (!resolve_unres_item(mod, unres->item[i], unres->type[i], unres->str_snode[i], unres, line)) {
                unres->type[i] = UNRES_RESOLVED;
                ++resolved;
                ++res_uses;
            } else {
                print_unres_item_fail(unres->item[i], unres->type[i], unres->str_snode[i], line);
            }
        }
    } while (res_uses && (res_uses < unres_uses));

    if (res_uses < unres_uses) {
        LOGVAL(LYE_SPEC, 0, "There are unresolved uses left.");
        return EXIT_FAILURE;
    }

    /* the rest */
    for (i = 0; i < unres->count; ++i) {
        if (unres->type[i] == UNRES_RESOLVED) {
            continue;
        }

#ifndef NDEBUG
            line = unres->line[i];
#endif

        if (!resolve_unres_item(mod, unres->item[i], unres->type[i], unres->str_snode[i], unres, line)) {
            unres->type[i] = UNRES_RESOLVED;
            ++resolved;
        }
    }

    if (resolved < unres->count) {
        LOGVAL(LYE_SPEC, 0, "There are unresolved items left.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* logs indirectly */
void
unres_add_str(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type, const char *str,
              uint32_t line)
{
    str = lydict_insert(mod->ctx, str, 0);
    unres_add_node(mod, unres, item, type, (struct lys_node *)str, line);
}

/* logs indirectly */
void
unres_add_node(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type,
                struct lys_node *node, uint32_t line)
{
    assert(unres && item);

    if (!resolve_unres_item(mod, item, type, node, unres, UINT_MAX)) {
        return;
    }

    print_unres_item_fail(item, type, node, line);

    unres->count++;
    unres->item = realloc(unres->item, unres->count*sizeof *unres->item);
    unres->item[unres->count-1] = item;
    unres->type = realloc(unres->type, unres->count*sizeof *unres->type);
    unres->type[unres->count-1] = type;
    unres->str_snode = realloc(unres->str_snode, unres->count*sizeof *unres->str_snode);
    unres->str_snode[unres->count-1] = node;
#ifndef NDEBUG
    unres->line = realloc(unres->line, unres->count*sizeof *unres->line);
    unres->line[unres->count-1] = line;
#endif
}

/* logs indirectly */
int
unres_dup(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type, void *new_item)
{
    int i;

    if (!item || !new_item) {
        LOGINT;
        return EXIT_FAILURE;
    }

    i = unres_find(unres, item, type);

    if (i == -1) {
        return EXIT_FAILURE;
    }

    if ((type == UNRES_TYPE_LEAFREF) || (type == UNRES_USES) || (type == UNRES_TYPE_DFLT)
            || (type == UNRES_WHEN) || (type == UNRES_MUST)) {
        unres_add_node(mod, unres, new_item, type, unres->str_snode[i], 0);
    } else {
        unres_add_str(mod, unres, new_item, type, unres->str_snode[i], 0);
    }

    return EXIT_SUCCESS;
}

/* does not log */
int
unres_find(struct unres_schema *unres, void *item, enum UNRES_ITEM type)
{
    uint32_t ret = -1, i;

    for (i = 0; i < unres->count; ++i) {
        if ((unres->item[i] == item) && (unres->type[i] == type)) {
            ret = i;
            break;
        }
    }

    return ret;
}
