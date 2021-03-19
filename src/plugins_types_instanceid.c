/**
 * @file plugins_types_instanceid.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in instance-identifier type plugin.
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "plugins_types.h"

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"

#include "path.h"

/**
 * @brief Printer callback printing the instance-identifier value.
 *
 * Implementation of the ly_type_print_clb.
 */
const char *
ly_type_print_instanceid(const struct lyd_value *value, LY_PREFIX_FORMAT format, void *prefix_data, ly_bool *dynamic)
{
    LY_ARRAY_COUNT_TYPE u, v;
    char *result = NULL;

    if (!value->target) {
        /* value was not fully processed */
        return NULL;
    }

    if ((format == LY_PREF_XML) || (format == LY_PREF_SCHEMA)) {
        /* everything is prefixed */
        LY_ARRAY_FOR(value->target, u) {
            ly_strcat(&result, "/%s:%s", ly_type_get_prefix(value->target[u].node->module, format, prefix_data),
                    value->target[u].node->name);
            LY_ARRAY_FOR(value->target[u].predicates, v) {
                struct ly_path_predicate *pred = &value->target[u].predicates[v];

                switch (value->target[u].pred_type) {
                case LY_PATH_PREDTYPE_NONE:
                    break;
                case LY_PATH_PREDTYPE_POSITION:
                    /* position predicate */
                    ly_strcat(&result, "[%" PRIu64 "]", pred->position);
                    break;
                case LY_PATH_PREDTYPE_LIST: {
                    /* key-predicate */
                    ly_bool d = 0;
                    const char *str = pred->value.realtype->plugin->print(&pred->value, format, prefix_data, &d);
                    char quot = '\'';
                    if (strchr(str, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[%s:%s=%c%s%c]", ly_type_get_prefix(pred->key->module, format, prefix_data),
                            pred->key->name, quot, str, quot);
                    if (d) {
                        free((char *)str);
                    }
                    break;
                }
                case LY_PATH_PREDTYPE_LEAFLIST: {
                    /* leaf-list-predicate */
                    ly_bool d = 0;
                    const char *str = pred->value.realtype->plugin->print(&pred->value, format, prefix_data, &d);
                    char quot = '\'';
                    if (strchr(str, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[.=%c%s%c]", quot, str, quot);
                    if (d) {
                        free((char *)str);
                    }
                    break;
                }
                }
            }
        }
    } else if (format == LY_PREF_JSON) {
        /* only the first node or the node changing module is prefixed */
        struct lys_module *mod = NULL;
        LY_ARRAY_FOR(value->target, u) {
            if (mod != value->target[u].node->module) {
                mod = value->target[u].node->module;
                ly_strcat(&result, "/%s:%s", ly_type_get_prefix(mod, format, prefix_data), value->target[u].node->name);
            } else {
                ly_strcat(&result, "/%s", value->target[u].node->name);
            }
            LY_ARRAY_FOR(value->target[u].predicates, v) {
                struct ly_path_predicate *pred = &value->target[u].predicates[v];

                switch (value->target[u].pred_type) {
                case LY_PATH_PREDTYPE_NONE:
                    break;
                case LY_PATH_PREDTYPE_POSITION:
                    /* position predicate */
                    ly_strcat(&result, "[%" PRIu64 "]", pred->position);
                    break;
                case LY_PATH_PREDTYPE_LIST: {
                    /* key-predicate */
                    ly_bool d = 0;
                    const char *str = pred->value.realtype->plugin->print(&pred->value, format, prefix_data, &d);
                    char quot = '\'';
                    if (strchr(str, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[%s=%c%s%c]", pred->key->name, quot, str, quot);
                    if (d) {
                        free((char *)str);
                    }
                    break;
                }
                case LY_PATH_PREDTYPE_LEAFLIST: {
                    /* leaf-list-predicate */
                    ly_bool d = 0;
                    const char *str = pred->value.realtype->plugin->print(&pred->value, format, prefix_data, &d);
                    char quot = '\'';
                    if (strchr(str, quot)) {
                        quot = '"';
                    }
                    ly_strcat(&result, "[.=%c%s%c]", quot, str, quot);
                    if (d) {
                        free((char *)str);
                    }
                    break;
                }
                }
            }
        }
    } else {
        /* not supported format */
        free(result);
        return NULL;
    }

    *dynamic = 1;
    return result;
}

/**
 * @brief Validate and store value of the YANG built-in instance-identifier type.
 *
 * Implementation of the ly_type_store_clb.
 */
LY_ERR
ly_type_store_instanceid(const struct ly_ctx *ctx, const struct lysc_type *type, const char *value, size_t value_len,
        uint32_t options, LY_PREFIX_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_instanceid *type_inst = (struct lysc_type_instanceid *)type;
    char *str;
    struct ly_path *path = NULL;
    ly_bool dyn;

    /* init */
    *err = NULL;

    /* check hints */
    ret = ly_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret != LY_SUCCESS,  cleanup_value);

    LY_CHECK_GOTO(ret = ly_type_lypath_new(ctx, value, value_len, options, format, prefix_data, ctx_node,
            unres, &path, err), cleanup);

    /* store resolved schema path */
    storage->target = path;
    path = NULL;

    /* store JSON string value */
    str = (char *)ly_type_print_instanceid(storage, LY_PREF_JSON, NULL, &dyn);
    assert(str && dyn);
    LY_CHECK_GOTO(ret = lydict_insert_zc(ctx, str, &storage->canonical), cleanup);
    storage->realtype = type;

    /* cleanup */
cleanup:
    ly_type_lypath_free(ctx, path);

cleanup_value:
    if (options & LY_TYPE_STORE_DYNAMIC) {
        free((char *)value);
    }

    if ((ret == LY_SUCCESS) && type_inst->require_instance) {
        /* needs to be resolved */
        return LY_EINCOMPLETE;
    } else {
        return ret;
    }
}

/**
 * @brief Validate value of the YANG built-in instance-identifier type.
 *
 * Implementation of the ly_type_validate_clb.
 */
LY_ERR
ly_type_validate_instanceid(const struct ly_ctx *UNUSED(ctx), const struct lysc_type *UNUSED(type),
        const struct lyd_node *UNUSED(ctx_node), const struct lyd_node *tree, struct lyd_value *storage,
        struct ly_err_item **err)
{
    struct lysc_type_instanceid *type_inst = (struct lysc_type_instanceid *)storage->realtype;
    LY_ERR ret = LY_SUCCESS;

    *err = NULL;

    if (!type_inst->require_instance) {
        /* redundant to resolve */
        return LY_SUCCESS;
    }

    /* find the target in data */
    ret = ly_path_eval(storage->target, tree, NULL);
    return ly_err_new(err, ret, LYVE_DATA, NULL, NULL, LY_ERRMSG_NOINST, storage->canonical);
}

/**
 * @brief Comparison callback checking the instance-identifier value.
 *
 * Implementation of the ly_type_compare_clb.
 */
LY_ERR
ly_type_compare_instanceid(const struct lyd_value *val1, const struct lyd_value *val2)
{
    LY_ARRAY_COUNT_TYPE u, v;

    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    if (val1 == val2) {
        return LY_SUCCESS;
    } else if (!val1->target || !val2->target || (LY_ARRAY_COUNT(val1->target) != LY_ARRAY_COUNT(val2->target))) {
        return LY_ENOT;
    }

    LY_ARRAY_FOR(val1->target, u) {
        struct ly_path *s1 = &val1->target[u];
        struct ly_path *s2 = &val2->target[u];

        if ((s1->node != s2->node) || (s1->pred_type != s2->pred_type) ||
                (s1->predicates && (LY_ARRAY_COUNT(s1->predicates) != LY_ARRAY_COUNT(s2->predicates)))) {
            return LY_ENOT;
        }
        if (s1->predicates) {
            LY_ARRAY_FOR(s1->predicates, v) {
                struct ly_path_predicate *pred1 = &s1->predicates[v];
                struct ly_path_predicate *pred2 = &s2->predicates[v];

                switch (s1->pred_type) {
                case LY_PATH_PREDTYPE_NONE:
                    break;
                case LY_PATH_PREDTYPE_POSITION:
                    /* position predicate */
                    if (pred1->position != pred2->position) {
                        return LY_ENOT;
                    }
                    break;
                case LY_PATH_PREDTYPE_LIST:
                    /* key-predicate */
                    if ((pred1->key != pred2->key) || ((struct lysc_node_leaf *)pred1->key)->type->plugin->compare(&pred1->value, &pred2->value)) {
                        return LY_ENOT;
                    }
                    break;
                case LY_PATH_PREDTYPE_LEAFLIST:
                    /* leaf-list predicate */
                    if (((struct lysc_node_leaflist *)s1->node)->type->plugin->compare(&pred1->value, &pred2->value)) {
                        return LY_ENOT;
                    }
                }
            }
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Duplication callback of the instance-identifier values.
 *
 * Implementation of the ly_type_dup_clb.
 */
LY_ERR
ly_type_dup_instanceid(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_CHECK_RET(lydict_insert(ctx, original->canonical, strlen(original->canonical), &dup->canonical));
    dup->realtype = original->realtype;
    return ly_path_dup(ctx, original->target, &dup->target);
}

/**
 * @brief Free value of the YANG built-in instance-identifier types.
 *
 * Implementation of the ly_type_free_clb.
 */
void
ly_type_free_instanceid(const struct ly_ctx *ctx, struct lyd_value *value)
{
    ly_path_free(ctx, value->target);
    value->target = NULL;
    ly_type_free_simple(ctx, value);
}
