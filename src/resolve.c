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
#include "xpath.h"
#include "parser.h"
#include "dict.h"
#include "tree_internal.h"

/**
 * @brief Parse an identifier.
 *
 * ;; An identifier MUST NOT start with (('X'|'x') ('M'|'m') ('L'|'l'))
 * identifier          = (ALPHA / "_")
 *                       *(ALPHA / DIGIT / "_" / "-" / ".")
 *
 * @param[in] id Identifier to use.
 *
 * @return Number of characters successfully parsed.
 */
int
parse_identifier(const char *id)
{
    int parsed = 0;

    if (((id[0] == 'x') || (id[0] == 'X'))
            && ((id[1] == 'm') || (id[0] == 'M'))
            && ((id[2] == 'l') || (id[2] == 'L'))) {
        return -parsed;
    }

    if (!isalpha(id[0]) && (id[0] != '_')) {
        return -parsed;
    }

    ++parsed;
    ++id;

    while (isalnum(id[0]) || (id[0] == '_') || (id[0] == '-') || (id[0] == '.')) {
        ++parsed;
        ++id;
    }

    return parsed;
}

/**
 * @brief Parse a node-identifier.
 *
 * node-identifier     = [prefix ":"] identifier
 *
 * @param[in] id Identifier to use.
 * @param[out] prefix Points to the prefix, NULL if there is not any.
 * @param[out] pref_len Length of the prefix, 0 if there is not any.
 * @param[out] name Points to the node name.
 * @param[out] nam_len Length of the node name.
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
parse_node_identifier(const char *id, const char **prefix, int *pref_len, const char **name, int *nam_len)
{
    int parsed = 0, ret;

    assert(id);
    if (prefix) {
        *prefix = NULL;
    }
    if (pref_len) {
        *pref_len = 0;
    }
    if (name) {
        *name = NULL;
    }
    if (nam_len) {
        *nam_len = 0;
    }

    if ((ret = parse_identifier(id)) < 1) {
        return ret;
    }

    if (prefix) {
        *prefix = id;
    }
    if (pref_len) {
        *pref_len = ret;
    }

    parsed += ret;
    id += ret;

    /* there is prefix */
    if (id[0] == ':') {
        ++parsed;
        ++id;

    /* there isn't */
    } else {
        if (name && prefix) {
            *name = *prefix;
        }
        if (prefix) {
            *prefix = NULL;
        }

        if (nam_len && pref_len) {
            *nam_len = *pref_len;
        }
        if (pref_len) {
            *pref_len = 0;
        }

        return parsed;
    }

    /* identifier (node name) */
    if ((ret = parse_identifier(id)) < 1) {
        return -parsed+ret;
    }

    if (name) {
        *name = id;
    }
    if (nam_len) {
        *nam_len = ret;
    }

    return parsed+ret;
}

/**
 * @brief Parse a path-predicate (leafref).
 *
 * path-predicate      = "[" *WSP path-equality-expr *WSP "]"
 * path-equality-expr  = node-identifier *WSP "=" *WSP path-key-expr
 *
 * @param[in] id Identifier to use.
 * @param[out] prefix Points to the prefix, NULL if there is not any.
 * @param[out] pref_len Length of the prefix, 0 if there is not any.
 * @param[out] name Points to the node name.
 * @param[out] nam_len Length of the node name.
 * @param[out] path_key_expr Points to the path-key-expr.
 * @param[out] pke_len Length of the path-key-expr.
 * @param[out] has_predicate Flag to mark whether there is another predicate following.
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
parse_path_predicate(const char *id, const char **prefix, int *pref_len, const char **name, int *nam_len,
                     const char **path_key_expr, int *pke_len, int *has_predicate)
{
    const char *ptr;
    int parsed = 0, ret;

    assert(id);
    if (prefix) {
        *prefix = NULL;
    }
    if (pref_len) {
        *pref_len = 0;
    }
    if (name) {
        *name = NULL;
    }
    if (nam_len) {
        *nam_len = 0;
    }
    if (path_key_expr) {
        *path_key_expr = NULL;
    }
    if (pke_len) {
        *pke_len = 0;
    }
    if (has_predicate) {
        *has_predicate = 0;
    }

    if (id[0] != '[') {
        return -parsed;
    }

    ++parsed;
    ++id;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    if ((ret = parse_node_identifier(id, prefix, pref_len, name, nam_len)) < 1) {
        return -parsed+ret;
    }

    parsed += ret;
    id += ret;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    if (id[0] != '=') {
        return -parsed;
    }

    ++parsed;
    ++id;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    if ((ptr = strchr(id, ']')) == NULL) {
        return -parsed;
    }

    --ptr;
    while (isspace(ptr[0])) {
        --ptr;
    }
    ++ptr;

    ret = ptr-id;
    if (path_key_expr) {
        *path_key_expr = id;
    }
    if (pke_len) {
        *pke_len = ret;
    }

    parsed += ret;
    id += ret;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    assert(id[0] == ']');

    if (id[1] == '[') {
        *has_predicate = 1;
    }

    return parsed+1;
}

/**
 * @brief Parse a path-key-expr (leafref). First call parses "current()", all
 *        the ".." and the first node-identifier, other calls parse a single
 *        node-identifier each.
 *
 * path-key-expr       = current-function-invocation *WSP "/" *WSP
 *                       rel-path-keyexpr
 * rel-path-keyexpr    = 1*(".." *WSP "/" *WSP)
 *                       *(node-identifier *WSP "/" *WSP)
 *                       node-identifier
 *
 * @param[in] id Identifier to use.
 * @param[out] prefix Points to the prefix, NULL if there is not any.
 * @param[out] pref_len Length of the prefix, 0 if there is not any.
 * @param[out] name Points to the node name.
 * @param[out] nam_len Length of the node name.
 * @param[out] parent_times Number of ".." in the path. Must be 0 on the first call,
 *                          must not be changed between consecutive calls.
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
parse_path_key_expr(const char *id, const char **prefix, int *pref_len, const char **name, int *nam_len,
                    int *parent_times)
{
    int parsed = 0, ret, par_times = 0;

    assert(id);
    assert(parent_times);
    if (prefix) {
        *prefix = NULL;
    }
    if (pref_len) {
        *pref_len = 0;
    }
    if (name) {
        *name = NULL;
    }
    if (nam_len) {
        *nam_len = 0;
    }

    if (!*parent_times) {
        /* current-function-invocation *WSP "/" *WSP rel-path-keyexpr */
        if (strncmp(id, "current()", 9)) {
            return -parsed;
        }

        parsed += 9;
        id += 9;

        while (isspace(id[0])) {
            ++parsed;
            ++id;
        }

        if (id[0] != '/') {
            return -parsed;
        }

        ++parsed;
        ++id;

        while (isspace(id[0])) {
            ++parsed;
            ++id;
        }

        /* rel-path-keyexpr */
        if (strncmp(id, "..", 2)) {
            return -parsed;
        }
        ++par_times;

        parsed += 2;
        id += 2;

        while (isspace(id[0])) {
            ++parsed;
            ++id;
        }
    }

    /* 1*(".." *WSP "/" *WSP) *(node-identifier *WSP "/" *WSP) node-identifier
     *
     * first parent reference with whitespaces already parsed
     */
    if (id[0] != '/') {
        return -parsed;
    }

    ++parsed;
    ++id;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    while (!strncmp(id, "..", 2) && !*parent_times) {
        ++par_times;

        parsed += 2;
        id += 2;

        while (isspace(id[0])) {
            ++parsed;
            ++id;
        }

        if (id[0] != '/') {
            return -parsed;
        }

        ++parsed;
        ++id;

        while (isspace(id[0])) {
            ++parsed;
            ++id;
        }
    }

    if (!*parent_times) {
        *parent_times = par_times;
    }

    /* all parent references must be parsed at this point */
    if ((ret = parse_node_identifier(id, prefix, pref_len, name, nam_len)) < 1) {
        return -parsed+ret;
    }

    parsed += ret;
    id += ret;

    return parsed;
}

/**
 * @brief Parse path-arg (leafref).
 *
 * path-arg            = absolute-path / relative-path
 * absolute-path       = 1*("/" (node-identifier *path-predicate))
 * relative-path       = 1*(".." "/") descendant-path
 *
 * @param[in] id Identifier to use.
 * @param[out] prefix Points to the prefix, NULL if there is not any.
 * @param[out] pref_len Length of the prefix, 0 if there is not any.
 * @param[out] name Points to the node name.
 * @param[out] nam_len Length of the node name.
 * @param[out] parent_times Number of ".." in the path. Must be 0 on the first call,
 *                          must not be changed between consecutive calls. -1 if the
 *                          path is relative.
 * @param[out] has_predicate Flag to mark whether there is a predicate specified.
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
parse_path_arg(const char *id, const char **prefix, int *pref_len, const char **name, int *nam_len, int *parent_times,
               int *has_predicate)
{
    int parsed = 0, ret, par_times = 0;

    assert(id);
    assert(parent_times);
    if (prefix) {
        *prefix = NULL;
    }
    if (pref_len) {
        *pref_len = 0;
    }
    if (name) {
        *name = NULL;
    }
    if (nam_len) {
        *nam_len = 0;
    }
    if (has_predicate) {
        *has_predicate = 0;
    }

    if (!*parent_times && !strncmp(id, "..", 2)) {
        ++par_times;

        parsed += 2;
        id += 2;

        while (!strncmp(id, "/..", 3)) {
            ++par_times;

            parsed += 3;
            id += 3;
        }
    }

    if (!*parent_times) {
        if (par_times) {
            *parent_times = par_times;
        } else {
            *parent_times = -1;
        }
    }

    if (id[0] != '/') {
        return -parsed;
    }

    /* skip '/' */
    ++parsed;
    ++id;

    /* node-identifier ([prefix:]identifier) */
    if ((ret = parse_node_identifier(id, prefix, pref_len, name, nam_len)) < 1) {
        return -parsed-ret;
    }

    parsed += ret;
    id += ret;

    /* there is no predicate */
    if ((id[0] == '/') || !id[0]) {
        return parsed;
    } else if (id[0] != '[') {
        return -parsed;
    }

    if (has_predicate) {
        *has_predicate = 1;
    }

    return parsed;
}

/**
 * @brief Parse instance-identifier in JSON format. That means that prefixes
 *        (which are mandatory) are actually model names.
 *
 * instance-identifier = 1*("/" (node-identifier *predicate))
 *
 * @param[in] id Identifier to use.
 * @param[out] model Points to the model name.
 * @param[out] mod_len Length of the model name.
 * @param[out] name Points to the node name.
 * @param[out] nam_len Length of the node name.
 * @param[out] has_predicate Flag to mark whether there is a predicate specified.
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
parse_instance_identifier_json(const char *id, const char **model, int *mod_len, const char **name, int *nam_len,
                               int *has_predicate)
{
    int parsed = 0, ret;

    assert(id);
    if (model) {
        *model = NULL;
    }
    if (mod_len) {
        *mod_len = 0;
    }
    if (name) {
        *name = NULL;
    }
    if (nam_len) {
        *nam_len = 0;
    }
    if (has_predicate) {
        *has_predicate = 0;
    }

    if (id[0] != '/') {
        return -parsed;
    }

    ++parsed;
    ++id;

    if ((ret = parse_node_identifier(id, model, mod_len, name, nam_len)) < 1) {
        return -parsed+ret;
    } else if (model && !*model) {
        return -parsed;
    }

    parsed += ret;
    id += ret;

    if ((id[0] == '[') && has_predicate) {
        *has_predicate = 1;
    }

    return parsed;
}

/**
 * @brief Parse predicate (instance-identifier) in JSON format. That means that prefixes
 *        (which are mandatory) are actually model names.
 *
 * predicate           = "[" *WSP (predicate-expr / pos) *WSP "]"
 * predicate-expr      = (node-identifier / ".") *WSP "=" *WSP
 *                       ((DQUOTE string DQUOTE) /
 *                        (SQUOTE string SQUOTE))
 * pos                 = non-negative-integer-value
 *
 * @param[in] id Identifier to use.
 * @param[out] model Points to the model name.
 * @param[out] mod_len Length of the model name.
 * @param[out] name Points to the node name. Can be identifier (from node-identifier), "." or pos.
 * @param[out] nam_len Length of the node name.
 * @param[out] value Value the node-identifier must have (string from the grammar),
 *                   NULL if there is not any.
 * @param[out] val_len Length of the value, 0 if there is not any.
 * @param[out] has_predicate Flag to mark whether there is a predicate specified.
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
parse_predicate_json(const char *id, const char **model, int *mod_len, const char **name, int *nam_len,
                     const char **value, int *val_len, int *has_predicate)
{
    const char *ptr;
    int parsed = 0, ret;
    char quote;

    assert(id);
    if (model) {
        *model = NULL;
    }
    if (mod_len) {
        *mod_len = 0;
    }
    if (name) {
        *name = NULL;
    }
    if (nam_len) {
        *nam_len = 0;
    }
    if (value) {
        *value = NULL;
    }
    if (val_len) {
        *val_len = 0;
    }
    if (has_predicate) {
        *has_predicate = 0;
    }

    if (id[0] != '[') {
        return -parsed;
    }

    ++parsed;
    ++id;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    /* pos */
    if (isdigit(id[0])) {
        if (name) {
            *name = id;
        }

        if (id[0] == '0') {
            ++parsed;
            ++id;

            if (isdigit(id[0])) {
                return -parsed;
            }
        }

        while (isdigit(id[0])) {
            ++parsed;
            ++id;
        }

        if (nam_len) {
            *nam_len = id-(*name);
        }

    /* "." */
    } else if (id[0] == '.') {
        if (name) {
            *name = id;
        }
        if (nam_len) {
            *nam_len = 1;
        }

        ++parsed;
        ++id;

    /* node-identifier */
    } else {
        if ((ret = parse_node_identifier(id, model, mod_len, name, nam_len)) < 1) {
            return -parsed+ret;
        } else if (model && !*model) {
            return -parsed;
        }

        parsed += ret;
        id += ret;
    }

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    if (id[0] != '=') {
        return -parsed;
    }

    ++parsed;
    ++id;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    /* ((DQUOTE string DQUOTE) / (SQUOTE string SQUOTE)) */
    if ((id[0] == '\"') || (id[0] == '\'')) {
        quote = id[0];

        ++parsed;
        ++id;

        if ((ptr = strchr(id, quote)) == NULL) {
            return -parsed;
        }
        ret = ptr-id;

        if (value) {
            *value = id;
        }
        if (val_len) {
            *val_len = ret;
        }

        parsed += ret+1;
        id += ret+1;
    } else {
        return -parsed;
    }

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    if (id[0] != ']') {
        return -parsed;
    }

    ++parsed;
    ++id;

    if ((id[0] == '[') && has_predicate) {
        *has_predicate = 1;
    }

    return parsed;
}

/**
 * @brief Parse schema-nodeid.
 *
 * schema-nodeid       = absolute-schema-nodeid /
 *                       descendant-schema-nodeid
 * absolute-schema-nodeid = 1*("/" node-identifier)
 * descendant-schema-nodeid =
 *                       node-identifier
 *                       absolute-schema-nodeid
 *
 * @param[in] id Identifier to use.
 * @param[out] prefix Points to the prefix, NULL if there is not any.
 * @param[out] pref_len Length of the prefix, 0 if there is not any.
 * @param[out] name Points to the node name. Can be identifier (from node-identifier), "." or pos.
 * @param[out] nam_len Length of the node name.
 * @param[out] is_relative Flag to mark whether the nodeid is absolute or descendant. Must be -1
 *                         on the first call, must not be changed between consecutive calls.
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
parse_schema_nodeid(const char *id, const char **prefix, int *pref_len, const char **name, int *nam_len,
                    int *is_relative)
{
    int parsed = 0, ret;

    assert(id);
    assert(is_relative);
    if (prefix) {
        *prefix = NULL;
    }
    if (pref_len) {
        *pref_len = 0;
    }
    if (name) {
        *name = NULL;
    }
    if (nam_len) {
        *nam_len = 0;
    }

    if (id[0] != '/') {
        if (*is_relative != -1) {
            return -parsed;
        } else {
            *is_relative = 1;
        }
    } else {
        if (*is_relative == -1) {
            *is_relative = 0;
        }
        ++parsed;
        ++id;
    }

    if ((ret = parse_node_identifier(id, prefix, pref_len, name, nam_len)) < 1) {
        return -parsed+ret;
    }

    return parsed+ret;
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
            if (!strncmp(sub_mod->imp[j].prefix, prefix, pref_len) && !sub_mod->imp[j].prefix[pref_len]) {
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

    assert(prefix && pref_len);

    if (!mod) {
        return NULL;
    }

    /* module itself */
    if (!strncmp(mod->prefix, prefix, pref_len) && (mod->prefix[pref_len] == '\0')) {
        return mod;
    }

    /* imported modules */
    for (i = 0; i < mod->imp_size; i++) {
        if (!strncmp(mod->imp[i].prefix, prefix, pref_len) && (mod->imp[i].prefix[pref_len] == '\0')) {
            return mod->imp[i].module;
        }
    }

    /* imports in includes */
    return resolve_import_in_includes_recursive(mod, prefix, pref_len);
}

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
 * @return EXIT_SUCCESS on succes, -1 on error.
 */
int
resolve_len_ran_interval(const char *str_restr, struct lys_type *type, int superior_restr,
                         struct len_ran_intv** local_intv)
{
    /* 0 - unsigned, 1 - signed, 2 - floating point */
    int kind, rc = EXIT_SUCCESS;
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
        return -1;
    }

    /* process superior types */
    if (type->der && superior_restr) {
        if (resolve_len_ran_interval(NULL, &type->der->type, superior_restr, &intv)) {
            LOGINT;
            return -1;
        }
        assert(!intv || (intv->kind == kind));
    }

    if (!str_restr) {
        /* we are validating data and not have any restriction, but a superior type might have */
        if (type->der && !superior_restr && !intv) {
            if (resolve_len_ran_interval(NULL, &type->der->type, superior_restr, &intv)) {
                LOGINT;
                return -1;
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
            rc = -1;
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
                rc = -1;
                goto cleanup;
            }
        } else {
            LOGINT;
            rc = -1;
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
                        rc = -1;
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
                        rc = -1;
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
                        rc = -1;
                        goto cleanup;
                    }
                }
            }

            tmp_intv = tmp_intv->next;
        }

        /* some interval left uncovered -> fail */
        if (tmp_local_intv) {
            rc = -1;
        }

    }

cleanup:
    while (intv) {
        tmp_intv = intv->next;
        free(intv);
        intv = tmp_intv;
    }

    /* fail */
    if (rc) {
        while (*local_intv) {
            tmp_local_intv = (*local_intv)->next;
            free(*local_intv);
            *local_intv = tmp_local_intv;
        }
    }

    return rc;
}

/**
 * @brief Resolve a typedef. Does not log.
 *
 * @param[in] name Typedef name.
 * @param[in] mod_name Typedef name module name.
 * @param[in] module Main module.
 * @param[in] parent Parent of the resolved type definition.
 * @param[out] ret Pointer to the resolved typedef. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
int
resolve_superior_type(const char *name, const char *mod_name, struct lys_module *module, struct lys_node *parent,
                      struct lys_tpdf **ret)
{
    int i, j;
    struct lys_tpdf *tpdf;
    int tpdf_size;

    if (!mod_name) {
        /* no prefix, try built-in types */
        for (i = 1; i < LY_DATA_TYPE_COUNT; i++) {
            if (!strcmp(ly_types[i].def->name, name)) {
                if (ret) {
                    *ret = ly_types[i].def;
                }
                return EXIT_SUCCESS;
            }
        }
    } else {
        if (!strcmp(mod_name, module->name)) {
            /* prefix refers to the current module, ignore it */
            mod_name = NULL;
        }
    }

    if (!mod_name && parent) {
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
                    if (ret) {
                        *ret = &tpdf[i];
                    }
                    return EXIT_SUCCESS;
                }
            }

            parent = parent->parent;
        }
    } else if (mod_name) {
        /* get module where to search */
        module = ly_ctx_get_module(module->ctx, mod_name, NULL);
        if (!module) {
            return -1;
        }
    }

    /* search in top level typedefs */
    for (i = 0; i < module->tpdf_size; i++) {
        if (!strcmp(module->tpdf[i].name, name)) {
            if (ret) {
                *ret = &module->tpdf[i];
            }
            return EXIT_SUCCESS;
        }
    }

    /* search in submodules */
    for (i = 0; i < module->inc_size; i++) {
        for (j = 0; j < module->inc[i].submodule->tpdf_size; j++) {
            if (!strcmp(module->inc[i].submodule->tpdf[j].name, name)) {
                if (ret) {
                    *ret = &module->inc[i].submodule->tpdf[j];
                }
                return EXIT_SUCCESS;
            }
        }
    }

    return EXIT_FAILURE;
}

/**
 * @brief Check the default \p value of the \p type. Logs directly.
 *
 * @param[in] type Type definition to use.
 * @param[in] value Default value to check.
 * @param[in] first Whether this is the first resolution try. Affects logging.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
check_default(struct lys_type *type, const char *value, int first, uint32_t line)
{
    struct lyd_node_leaf_list node;
    struct lys_type *iter;
    int ret = EXIT_SUCCESS, found;

    /* dummy leaf */
    node.value_str = value;
    node.value_type = type->base;
    node.schema = calloc(1, sizeof *node.schema);
    node.schema->name = strdup("default");

    if (type->base == LY_TYPE_UNION) {
        found = 0;
        iter = lyp_get_next_union_type(type, NULL, &found);
        while (iter) {
            node.value_type = iter->base;

            /* special case with too much effort required and almost no added value */
            if ((iter->base == LY_TYPE_IDENT) || (iter->base == LY_TYPE_INST)) {
                LOGVAL(LYE_SPEC, line,
                       "Union with \"instance-identifier\" or \"identityref\" and a default value is not supported!");
                ret = -1;
                goto finish;
            }

            if (!lyp_parse_value(&node, iter, 1, NULL, UINT_MAX)) {
                break;
            }

            found = 0;
            iter = lyp_get_next_union_type(type, iter, &found);
        }

        if (!iter) {
            if (!first) {
                LOGVAL(LYE_INVAL, line, node.value_str, "default");
            }
            ret = EXIT_FAILURE;
            goto finish;
        }

    } else if (type->base == LY_TYPE_LEAFREF) {
        if (!type->info.lref.target) {
            ret = EXIT_FAILURE;
            goto finish;
        }
        ret = check_default(&type->info.lref.target->type, value, first, line);

    } else if ((type->base == LY_TYPE_INST) || (type->base == LY_TYPE_IDENT)) {
        /* it was converted to JSON format before, nothing else sensible we can do */

    } else {
        ret = lyp_parse_value(&node, type, 1, NULL, (first ? UINT_MAX : line));
    }

finish:
    if (node.value_type == LY_TYPE_BITS) {
        free(node.value.bit);
    }
    free((char *)node.schema->name);
    free(node.schema);

    return ret;
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
 * @return EXIT_SUCCESS on success, -1 on error.
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
        return -1;
    }

    /* uniqueness */
    for (j = index - 1; j >= 0; j--) {
        if (list[index] == list[j]) {
            LOGVAL(LYE_KEY_DUP, line, key->name);
            return -1;
        }
    }

    /* key is a leaf */
    if (key->nodetype != LYS_LEAF) {
        LOGVAL(LYE_KEY_NLEAF, line, key->name);
        return -1;
    }

    /* type of the leaf is not built-in empty */
    if (key->type.base == LY_TYPE_EMPTY) {
        LOGVAL(LYE_KEY_TYPE, line, key->name);
        return -1;
    }

    /* config attribute is the same as of the list */
    if ((flags & LYS_CONFIG_MASK) != (key->flags & LYS_CONFIG_MASK)) {
        LOGVAL(LYE_KEY_CONFIG, line, key->name);
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve (fill) a unique. Logs directly.
 *
 * @param[in] parent The parent node of the unique structure.
 * @param[in] uniq_str The value of the unique node.
 * @param[in] uniq_s The unique structure to use.
 * @param[in] first Whether this is the first resolution try. Affects logging.
 * @param[in] line The line in the input file.
 *
 * @return EXIT_SUCCESS on succes, EXIT_FAILURE on forward reference, -1 on error.
 */
int
resolve_unique(struct lys_node *parent, const char *uniq_str, struct lys_unique *uniq_s, int first, uint32_t line)
{
    char *uniq_val, *uniq_begin, *start;
    int i, j, rc;

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

        rc = resolve_schema_nodeid(start, parent->child, parent->module, LYS_LEAF,
                                   (struct lys_node **)&uniq_s->leafs[i]);
        if (rc) {
            if ((rc == -1) || !first) {
                LOGVAL(LYE_INARG, line, start, "unique");
                if (rc == EXIT_FAILURE) {
                    LOGVAL(LYE_SPEC, 0, "Target leaf not found.");
                }
            }
            goto error;
        }
        if (uniq_s->leafs[i]->nodetype != LYS_LEAF) {
            LOGVAL(LYE_INARG, line, start, "unique");
            LOGVAL(LYE_SPEC, 0, "Target is not a leaf.");
            rc = -1;
            goto error;
        }

        for (j = 0; j < i; j++) {
            if (uniq_s->leafs[j] == uniq_s->leafs[i]) {
                LOGVAL(LYE_INARG, line, start, "unique");
                LOGVAL(LYE_SPEC, 0, "The identifier is not unique");
                rc = -1;
                goto error;
            }
        }
    }

    free(uniq_begin);
    return EXIT_SUCCESS;

error:

    free(uniq_s->leafs);
    free(uniq_begin);

    return rc;
}

/**
 * @brief Resolve (fill) a grouping in an uses. Logs directly.
 *
 * @param[in] uses The uses to use.
 * @param[in] first Whether this is the first resolution try. Affects logging.
 * @param[in] line The line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_grouping(struct lys_node_uses *uses, int first, uint32_t line)
{
    struct lys_module *module;
    const char *prefix, *name;
    int i, pref_len, nam_len;
    struct lys_node *start;

    /* parse the identifier, it must be parsed on one call */
    if ((i = parse_node_identifier(uses->name, &prefix, &pref_len, &name, &nam_len)) < 1) {
        LOGVAL(LYE_INCHAR, line, uses->name[-i], &uses->name[-i]);
        return -1;
    } else if (uses->name[i]) {
        LOGVAL(LYE_INCHAR, line, uses->name[i], &uses->name[i]);
        return -1;
    }

    if (prefix) {
        module = resolve_prefixed_module(uses->module, prefix, pref_len);
        if (!module) {
            LOGVAL(LYE_INMOD_LEN, line, pref_len, prefix);
            return -1;
        }
        start = module->data;
    } else {
        start = (struct lys_node *)uses;
    }

    uses->grp = lys_find_grouping_up(name, start, 1);
    if (uses->grp) {
        return EXIT_SUCCESS;
    }

    if (prefix || !first) {
        LOGVAL(LYE_INRESOLV, line, "grouping", uses->name);
    }
    /* import must now be fully resolved */
    if (prefix) {
        return -1;
    }
    return EXIT_FAILURE;
}

/**
 * @brief Resolve (find) a feature definition. Logs directly.
 *
 * @param[in] name Feature name.
 * @param[in] module Module to search in.
 * @param[in] first Whether this is the first resolution try. Affects logging.
 * @param[in] line The line in the input file.
 * @param[out] ret Pointer to the resolved feature. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_feature(const char *id, struct lys_module *module, int first, uint32_t line, struct lys_feature **ret)
{
    const char *prefix, *name;
    int pref_len, nam_len, i, j;

    assert(id);
    assert(module);

    /* check prefix */
    if ((i = parse_node_identifier(id, &prefix, &pref_len, &name, &nam_len)) < 1) {
        LOGVAL(LYE_INCHAR, line, id[-i], &id[-i]);
        return -1;
    }

    if (prefix) {
        /* search in imported modules */
        module = resolve_prefixed_module(module, prefix, pref_len);
        if (!module) {
            /* identity refers unknown data model */
            LOGVAL(LYE_INMOD_LEN, line, pref_len, prefix);
            return -1;
        }
    } else {
        /* search in submodules */
        for (i = 0; i < module->inc_size; i++) {
            for (j = 0; j < module->inc[i].submodule->features_size; j++) {
                if (!strcmp(name, module->inc[i].submodule->features[j].name)) {
                    if (ret) {
                        *ret = &(module->inc[i].submodule->features[j]);
                    }
                    return EXIT_SUCCESS;
                }
            }
        }
    }

    /* search in the identified module */
    for (j = 0; j < module->features_size; j++) {
        if (!strcmp(name, module->features[j].name)) {
            if (ret) {
                *ret = &module->features[j];
            }
            return EXIT_SUCCESS;
        }
    }

    /* not found */
    if (!first) {
        LOGVAL(LYE_INRESOLV, line, "feature", id);
    }
    return EXIT_FAILURE;
}

/**
 * @brief Resolve (find) a valid sibling. Does not log.
 *
 * Valid child means a schema pointer to a node that is part of
 * the data meaning uses are skipped. Includes module comparison
 * (can handle augments). Module is adjusted based on the prefix.
 * Includes are also searched if siblings are top-level nodes.
 *
 * @param[in] mod Main module. Prefix is considered to be from this module.
 * @param[in] siblings Siblings to consider. They are first adjusted to
 *                     point to the first sibling.
 * @param[in] prefix Node prefix.
 * @param[in] pref_len Node prefix length.
 * @param[in] name Node name.
 * @param[in] nam_len Node name length.
 * @param[in] type ORed desired type of the node. 0 means any type.
 *                 Returns only schema data nodes (no uses, grouping, augment, choice, case).
 * @param[out] ret Pointer to the node of the desired type. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
int
resolve_sibling(struct lys_module *mod, struct lys_node *siblings, const char *prefix, int pref_len, const char *name,
                int nam_len, LYS_NODE type, struct lys_node **ret)
{
    struct lys_node *node, *old_siblings = NULL;
    struct lys_module *prefix_mod, *cur_mod;
    int in_submod;

    assert(siblings && name);
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
            return -1;
        }
        cur_mod = prefix_mod;
        /* it is not our module */
        if (cur_mod != mod) {
            old_siblings = siblings;
            siblings = cur_mod->data;
        }
    } else {
        if (mod) {
            prefix_mod = mod;
        } else {
            prefix_mod = siblings->module;
        }
        if (prefix_mod->type) {
            prefix_mod = ((struct lys_submodule *)prefix_mod)->belongsto;
        }
        cur_mod = prefix_mod;
    }

    while (1) {
        /* try to find the node */
        node = NULL;
        while ((node = lys_getnext(node, siblings->parent, cur_mod, 0))) {
            if (!type || (node->nodetype & type)) {
                /* module check */
                if (!node->module->type) {
                    if (cur_mod != node->module) {
                        continue;
                    }
                } else {
                    /* both are submodules */
                    if (cur_mod->type) {
                        if (cur_mod != node->module) {
                            continue;
                        }
                    } else {
                        if (cur_mod != ((struct lys_submodule *)node->module)->belongsto) {
                            continue;
                        }
                    }
                }

                /* direct name check */
                if (node->name == name || (!strncmp(node->name, name, nam_len) && !node->name[nam_len])) {
                    if (ret) {
                        *ret = node;
                    }
                    return EXIT_SUCCESS;
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

    return EXIT_FAILURE;
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
 * @param[in] mod Module to use.
 * @param[in] node_type Decides how to modify the search.
 * @param[out] ret Pointer to the matching node. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
int
resolve_schema_nodeid(const char *id, struct lys_node *start, struct lys_module *mod, LYS_NODE node_type,
                      struct lys_node **ret)
{
    const char *name, *prefix;
    struct lys_node *sibling;
    int i, nam_len, pref_len, is_relative = -1;
    struct lys_module *prefix_mod, *start_mod;
    /* 0 - in module, 1 - in 1st submodule, 2 - in 2nd submodule, ... */
    uint8_t in_submod = 0;

    assert(mod);
    assert(id);

    if ((i = parse_schema_nodeid(id, &prefix, &pref_len, &name, &nam_len, &is_relative)) < 1) {
        return -1;
    }
    id += i;

    if (!is_relative && (node_type & (LYS_USES | LYS_CHOICE | LYS_LEAF))) {
        return -1;
    }

    /* absolute-schema-nodeid */
    if (!is_relative) {
        if (prefix) {
            start_mod = resolve_prefixed_module(mod, prefix, pref_len);
            if (!start_mod) {
                return -1;
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
                    && (!(sibling->nodetype & (LYS_RPC | LYS_NOTIF)) || (node_type == LYS_AUGMENT))
                    && ((sibling->name && !strncmp(name, sibling->name, nam_len) && !sibling->name[nam_len])
                    || (!strncmp(name, "input", 5) && (nam_len == 5) && (sibling->nodetype == LYS_INPUT))
                    || (!strncmp(name, "output", 6) && (nam_len == 6) && (sibling->nodetype == LYS_OUTPUT)))) {

                /* prefix match check */
                if (prefix) {
                    prefix_mod = resolve_prefixed_module(mod, prefix, pref_len);
                    if (!prefix_mod) {
                        return -1;
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
                    if (ret) {
                        *ret = sibling;
                    }
                    return EXIT_SUCCESS;
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
            return -1;
        }

        /* no match */
        if (!sibling) {
            /* are we done with the included submodules as well? */
            if (in_submod == start_mod->inc_size) {
                return EXIT_FAILURE;
            }

            /* we aren't, check the next one */
            ++in_submod;
            start = start_mod->inc[in_submod-1].submodule->data;
            continue;
        }

        /* we found our submodule */
        if (in_submod) {
            start_mod = (struct lys_module *)start_mod->inc[in_submod-1].submodule;
            in_submod = 0;
        }

        if ((i = parse_schema_nodeid(id, &prefix, &pref_len, &name, &nam_len, &is_relative)) < 1) {
            return -1;
        }
        id += i;
    }

    /* cannot get here */
    LOGINT;
    return -1;
}

/* ignores line */
static void
unres_data_del(struct unres_data *unres, uint32_t i)
{
    /* there are items after the one deleted */
    if (i+1 < unres->count) {
        /* we only move the data, memory is left allocated, why bother */
        memmove(&unres->node[i], &unres->node[i+1], (unres->count-(i+1)) * sizeof *unres->node);

    /* deleting the last item */
    } else if (i == 0) {
        free(unres->node);
        unres->node = NULL;
    }

    /* if there are no items after and it is not the last one, just move the counter */
    --unres->count;
}

/**
 * @brief Resolve (find) a data node from a specific module. Does not log.
 *
 * @param[in] mod Module to search in.
 * @param[in] name Name of the data node.
 * @param[in] nam_len Length of the name.
 * @param[in] start Data node to start the search from.
 * @param[in,out] parents Resolved nodes. If there are some parents,
 *                        they are replaced (!!) with the resolvents.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference.
 */
static int
resolve_data(struct lys_module *mod, const char *name, int nam_len, struct lyd_node *start, struct unres_data *parents)
{
    struct lyd_node *node;
    int flag;
    uint32_t i;

    if (!parents->count) {
        parents->count = 1;
        parents->node = malloc(sizeof *parents->node);
        parents->node[0] = NULL;
    }
    for (i = 0; i < parents->count;) {
        if (parents->node[i] && (parents->node[i]->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML))) {
            /* skip */
            ++i;
            continue;
        }
        flag = 0;
        LY_TREE_FOR(parents->node[i] ? parents->node[i]->child : start, node) {
            if (node->schema->module == mod && !strncmp(node->schema->name, name, nam_len)
                    && node->schema->name[nam_len] == '\0') {
                /* matching target */
                if (!flag) {
                    /* put node instead of the current parent */
                    parents->node[i] = node;
                    flag = 1;
                } else {
                    /* multiple matching, so create a new node */
                    ++parents->count;
                    parents->node = realloc(parents->node, parents->count * sizeof *parents->node);
                    parents->node[parents->count-1] = node;
                    ++i;
                }
            }
        }

        if (!flag) {
            /* remove item from the parents list */
            unres_data_del(parents, i);
        } else {
            ++i;
        }
    }

    return parents->count ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * @brief Resolve (find) a data node. Does not log.
 *
 * @param[in] prefix Prefix of the data node.
 * @param[in] pref_len Length of the prefix.
 * @param[in] name Name of the data node.
 * @param[in] nam_len Length of the name.
 * @param[in] start Data node to start the search from.
 * @param[in,out] parents Resolved nodes. If there are some parents,
 *                        they are replaced (!!) with the resolvents.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 otherwise.
 */
static int
resolve_data_nodeid(const char *prefix, int pref_len, const char *name, int name_len, struct lyd_node *start,
                    struct unres_data *parents)
{
    struct lys_module *mod;

    assert(start);

    if (prefix) {
        /* we have prefix, find appropriate module */
        mod = resolve_prefixed_module(start->schema->module, prefix, pref_len);
        if (!mod) {
            /* invalid prefix */
            return -1;
        }
    } else {
        /* no prefix, module is the same as of current node */
        mod = start->schema->module;
    }

    return resolve_data(mod, name, name_len, start, parents);
}

/**
 * @brief Resolve a path predicate (leafref) in data context. Logs directly
 *        only specific errors, general no-resolvent error is left to the caller,
 *        but line fail is always displayed.
 *
 * @param[in] pred Predicate to use.
 * @param[in] first Whether this is the first resolution try. Affects logging.
 * @param[in] line Line in the input file.
 * @param[in,out] node_match Nodes satisfying the restriction
 *                           without the predicate. Nodes not
 *                           satisfying the predicate are removed.
 * @param[out] parsed Number of characters parsed, negative on error.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_path_predicate_data(const char *pred, int first, uint32_t line, struct unres_data *node_match, int *parsed)
{
    /* ... /node[source = destination] ... */
    struct unres_data source_match, dest_match;
    const char *path_key_expr, *source, *sour_pref, *dest, *dest_pref;
    int pke_len, sour_len, sour_pref_len, dest_len, dest_pref_len, parsed_loc = 0, pke_parsed = 0;
    int has_predicate, dest_parent_times, i, rc;
    uint32_t j;

    source_match.count = 1;
    source_match.node = malloc(sizeof *source_match.node);
    dest_match.count = 1;
    dest_match.node = malloc(sizeof *dest_match.node);

    do {
        if ((i = parse_path_predicate(pred, &sour_pref, &sour_pref_len, &source, &sour_len, &path_key_expr,
                                      &pke_len, &has_predicate)) < 1) {
            LOGVAL(LYE_INCHAR, line, pred[-i], &pred[-i]);
            rc = -1;
            goto error;
        }
        parsed_loc += i;
        pred += i;

        for (j = 0; j < node_match->count;) {
            /* source */
            source_match.node[0] = node_match->node[j];

            /* must be leaf (key of a list) */
            if ((rc = resolve_data_nodeid(sour_pref, sour_pref_len, source, sour_len, node_match->node[j],
                    &source_match)) || (source_match.count != 1) || (source_match.node[0]->schema->nodetype != LYS_LEAF)) {
                /* general error, the one written later will suffice */
                if ((rc == -1) || !first) {
                    LOGVAL(LYE_LINE, line);
                }
                i = 0;
                goto error;
            }

            /* destination */
            dest_match.node[0] = node_match->node[j];
            dest_parent_times = 0;
            if ((i = parse_path_key_expr(path_key_expr, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                            &dest_parent_times)) < 1) {
                LOGVAL(LYE_INCHAR, line, path_key_expr[-i], &path_key_expr[-i]);
                rc = -1;
                goto error;
            }
            pke_parsed = i;
            for (i = 0; i < dest_parent_times; ++i) {
                dest_match.node[0] = dest_match.node[0]->parent;
                if (!dest_match.node[0]) {
                    /* general error, the one written later will suffice */
                    if (!first) {
                        LOGVAL(LYE_LINE, line);
                    }
                    i = 0;
                    rc = EXIT_FAILURE;
                    goto error;
                }
            }
            while (1) {
                if ((rc = resolve_data_nodeid(dest_pref, dest_pref_len, dest, dest_len, dest_match.node[0],
                        &dest_match)) || (dest_match.count != 1)) {
                    /* general error, the one written later will suffice */
                    if ((rc == -1) || !first) {
                        LOGVAL(LYE_LINE, line);
                    }
                    i = 0;
                    goto error;
                }

                if (pke_len == pke_parsed) {
                    break;
                }
                if ((i = parse_path_key_expr(path_key_expr+pke_parsed, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                             &dest_parent_times)) < 1) {
                    LOGVAL(LYE_INCHAR, line, path_key_expr[-i], &path_key_expr[-i]);
                    rc = -1;
                    goto error;
                }
                pke_parsed += i;
            }

            /* check match between source and destination nodes */
            if (((struct lys_node_leaf *)source_match.node[0]->schema)->type.base
                    != ((struct lys_node_leaf *)dest_match.node[0]->schema)->type.base) {
                goto remove_leafref;
            }

            if (((struct lyd_node_leaf_list *)source_match.node[0])->value_str
                    != ((struct lyd_node_leaf_list *)dest_match.node[0])->value_str) {
                goto remove_leafref;
            }

            /* leafref is ok, continue check with next leafref */
            ++j;
            continue;

remove_leafref:
            /* does not fulfill conditions, remove leafref record */
            unres_data_del(node_match, j);
        }
    } while (has_predicate);

    free(source_match.node);
    free(dest_match.node);
    if (parsed) {
        *parsed = parsed_loc;
    }
    return EXIT_SUCCESS;

error:

    if (source_match.count) {
        free(source_match.node);
    }
    if (dest_match.count) {
        free(dest_match.node);
    }
    if (parsed) {
        *parsed = -parsed_loc+i;
    }
    return rc;
}

/**
 * @brief Resolve a path (leafref) in data context. Logs directly.
 *
 * @param[in] node Leafref data node.
 * @param[in] path Path of the leafref.
 * @param[in] first Whether this is the first resolution try. Affects logging.
 * @param[in] line Line in the input file.
 * @param[out] ret Matching nodes. Expects an empty, but allocated structure. Lines left untouched.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 otherwise.
 */
static int
resolve_path_arg_data(struct lyd_node *node, const char *path, int first, uint32_t line, struct unres_data *ret)
{
    struct lyd_node *data = NULL;
    const char *prefix, *name;
    int pref_len, nam_len, has_predicate, parent_times, i, parsed, rc;
    uint32_t j;

    assert(node && path && ret && !ret->count);

    parent_times = 0;
    parsed = 0;

    /* searching for nodeset */
    do {
        if ((i = parse_path_arg(path, &prefix, &pref_len, &name, &nam_len, &parent_times, &has_predicate)) < 1) {
            LOGVAL(LYE_INCHAR, line, path[-i], &path[-i]);
            rc = -1;
            goto error;
        }
        path += i;
        parsed += i;

        if (!ret->count) {
            if (parent_times != -1) {
                ret->count = 1;
                ret->node = calloc(1, sizeof *ret->node);
            }
            for (i = 0; i < parent_times; ++i) {
                /* relative path */
                if (!ret->count) {
                    /* error, too many .. */
                    LOGVAL(LYE_INVAL, line, path, node->schema->name);
                    rc = -1;
                    goto error;
                } else if (!ret->node[0]) {
                    /* first .. */
                    data = ret->node[0] = node->parent;
                } else if (!ret->node[0]->parent) {
                    /* we are in root */
                    ret->count = 0;
                    free(ret->node);
                    ret->node = NULL;
                } else {
                    /* multiple .. */
                    data = ret->node[0] = ret->node[0]->parent;
                }
            }

            /* absolute path */
            if (parent_times == -1) {
                for (data = node; data->parent; data = data->parent);
                /* we're still parsing it and the pointer is not correct yet */
                if (data->prev) {
                    for (; data->prev->next; data = data->prev);
                }
            }
        }

        /* node identifier */
        if ((rc = resolve_data_nodeid(prefix, pref_len, name, nam_len, data, ret))) {
            if ((rc == -1) || !first) {
                LOGVAL(LYE_INELEM_LEN, line, nam_len, name);
            }
            goto error;
        }

        if (has_predicate) {
            /* we have predicate, so the current results must be lists */
            for (j = 0; j < ret->count;) {
                if (ret->node[j]->schema->nodetype == LYS_LIST &&
                        ((struct lys_node_list *)ret->node[0]->schema)->keys) {
                    /* leafref is ok, continue check with next leafref */
                    ++j;
                    continue;
                }

                /* does not fulfill conditions, remove leafref record */
                unres_data_del(ret, j);
            }
            if ((rc = resolve_path_predicate_data(path, first, line, ret, &i))) {
                /* line was already displayed */
                if ((rc == -1) || !first) {
                    LOGVAL(LYE_NORESOLV, 0, path);
                }
                goto error;
            }
            path += i;
            parsed += i;

            if (!ret->count) {
                if (!first) {
                    LOGVAL(LYE_NORESOLV, line, path-parsed);
                }
                rc = EXIT_FAILURE;
                goto error;
            }
        }
    } while (path[0] != '\0');

    return EXIT_SUCCESS;

error:

    free(ret->node);
    ret->node = NULL;
    ret->count = 0;

    return rc;
}

/**
 * @brief Resolve a path (leafref) predicate in schema context. Logs directly.
 *
 * @param[in] path Path to use.
 * @param[in] mod Schema module.
 * @param[in] source_node Left operand node.
 * @param[in] dest_node Right ooperand node.
 * @param[in] first Whether this is the first resolution try. Affects logging.
 * @param[in] line Line in the input file.
 *
 * @return 0 on forward reference, otherwise the number
 *         of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
resolve_path_predicate_schema(const char *path, struct lys_module *mod, struct lys_node *source_node,
                              struct lys_node *dest_node, int first, uint32_t line)
{
    struct lys_node *src_node, *dst_node;
    const char *path_key_expr, *source, *sour_pref, *dest, *dest_pref;
    int pke_len, sour_len, sour_pref_len, dest_len, dest_pref_len, parsed = 0, pke_parsed = 0;
    int has_predicate, dest_parent_times = 0, i, rc;

    do {
        if ((i = parse_path_predicate(path, &sour_pref, &sour_pref_len, &source, &sour_len, &path_key_expr,
                                      &pke_len, &has_predicate)) < 1) {
            LOGVAL(LYE_INCHAR, line, path[-i], path-i);
            return -parsed+i;
        }
        parsed += i;
        path += i;

        /* source (must be leaf) */
        rc = resolve_sibling(mod, source_node->child, sour_pref, sour_pref_len, source, sour_len, LYS_LEAF, &src_node);
        if (rc) {
            if ((rc == -1) || !first) {
                LOGVAL(LYE_NORESOLV, line, path-parsed);
            }
            return 0;
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
                if (!first) {
                    LOGVAL(LYE_NORESOLV, line, path_key_expr);
                }
                return 0;
            }
        }
        while (1) {
            rc = resolve_sibling(mod, dst_node->child, dest_pref, dest_pref_len, dest, dest_len,
                                 LYS_CONTAINER | LYS_LIST | LYS_LEAF, &dst_node);
            if (rc) {
                if ((rc == -1) || !first) {
                    LOGVAL(LYE_NORESOLV, line, path_key_expr);
                }
                return 0;
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
        if (dst_node->nodetype != LYS_LEAF) {
            LOGVAL(LYE_NORESOLV, line, path-parsed);
            LOGVAL(LYE_SPEC, 0, "Destination node not a leaf, but %s.", strnodetype(dst_node->nodetype));
            return -parsed;
        }
    } while (has_predicate);

    return parsed;
}

/**
 * @brief Resolve a path (leafref) in schema context. Logs directly.
 *
 * @param[in] mod Module to use.
 * @param[in] path Path to use.
 * @param[in] parent_node Parent of the leafref.
 * @param[in] first Whether this is the first resolution try. Affects logging.
 * @param[in] line Line in the input file.
 * @param[out] ret Pointer to the resolved schema node. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_path_arg_schema(struct lys_module *mod, const char *path, struct lys_node *parent_node, int first,
                        uint32_t line, struct lys_node **ret)
{
    struct lys_node *node;
    const char *id, *prefix, *name;
    int pref_len, nam_len, parent_times, has_predicate;
    int i, first_iter, rc;

    first_iter = 1;
    parent_times = 0;
    id = path;

    do {
        if ((i = parse_path_arg(id, &prefix, &pref_len, &name, &nam_len, &parent_times, &has_predicate)) < 1) {
            LOGVAL(LYE_INCHAR, line, id[-i], &id[-i]);
            return -1;
        }
        id += i;

        if (first_iter) {
            if (parent_times == -1) {
                node = mod->data;
                if (!node) {
                    if (!first) {
                        LOGVAL(LYE_NORESOLV, line, path);
                    }
                    return EXIT_FAILURE;
                }
            } else if (parent_times > 0) {
                /* node is the parent already, skip one ".." */
                node = parent_node;
                i = 0;
                while (1) {
                    if (!node) {
                        if (!first) {
                            LOGVAL(LYE_NORESOLV, line, path);
                        }
                        return EXIT_FAILURE;
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
                return -1;
            }
            first_iter = 0;
        } else {
            node = node->child;
        }

        rc = resolve_sibling(mod, node, prefix, pref_len, name, nam_len, LYS_ANY & (~LYS_USES), &node);
        if (rc) {
            if ((rc == -1) || !first) {
                LOGVAL(LYE_NORESOLV, line, path);
            }
            return rc;
        }

        if (has_predicate) {
            /* we have predicate, so the current result must be list */
            if (node->nodetype != LYS_LIST) {
                LOGVAL(LYE_NORESOLV, line, path);
                return -1;
            }

            i = resolve_path_predicate_schema(id, mod, node, parent_node, first, line);
            if (!i) {
                return EXIT_FAILURE;
            } else if (i < 0) {
                return -1;
            }
            id += i;
        }
    } while (id[0]);

    /* the target must be leaf or leaf-list */
    if (!(node->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
        LOGVAL(LYE_NORESOLV, line, path);
        return -1;
    }

    if (ret) {
        *ret = node;
    }
    return EXIT_SUCCESS;
}

/**
 * @brief Resolve instance-identifier predicate. Does not log.
 *
 * @param[in] pred Predicate to use.
 * @param[in,out] node_match Nodes matching the restriction without
 *                           the predicate. Nodes not satisfying
 *                           the predicate are removed.
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
resolve_predicate_json(const char *pred, struct unres_data *node_match)
{
    /* ... /node[target = value] ... */
    struct unres_data target_match;
    struct ly_ctx *ctx;
    struct lys_module *mod;
    const char *model, *name, *value;
    char *str;
    int mod_len, nam_len, val_len, i, has_predicate, cur_idx, idx, parsed;
    uint32_t j;

    assert(pred && node_match->count);

    ctx = node_match->node[0]->schema->module->ctx;
    idx = -1;
    parsed = 0;

    do {
        if ((i = parse_predicate_json(pred, &model, &mod_len, &name, &nam_len, &value, &val_len, &has_predicate)) < 1) {
            return -parsed+i;
        }
        parsed += i;
        pred += i;

        /* pos */
        if (isdigit(name[0])) {
            idx = atoi(name);
        }

        for (cur_idx = 0, j = 0; j < node_match->count; ++cur_idx) {
            /* target */
            memset(&target_match, 0, sizeof target_match);
            if ((name[0] == '.') || !value) {
                target_match.count = 1;
                target_match.node = malloc(sizeof *target_match.node);
                target_match.node[0] = node_match->node[j];
            } else {
                str = strndup(model, mod_len);
                mod = ly_ctx_get_module(ctx, str, NULL);
                free(str);

                if (resolve_data(mod, name, nam_len, node_match->node[j], &target_match)) {
                    goto remove_instid;
                }
            }

            /* check that we have the correct type */
            if (name[0] == '.') {
                if (node_match->node[j]->schema->nodetype != LYS_LEAFLIST) {
                    goto remove_instid;
                }
            } else if (value) {
                if (node_match->node[j]->schema->nodetype != LYS_LIST) {
                    goto remove_instid;
                }
            }

            if ((value && (strncmp(((struct lyd_node_leaf_list *)target_match.node[0])->value_str, value, val_len)
                    || ((struct lyd_node_leaf_list *)target_match.node[0])->value_str[val_len]))
                    || (!value && (idx != cur_idx))) {
                goto remove_instid;
            }

            free(target_match.node);

            /* leafref is ok, continue check with next leafref */
            ++j;
            continue;

remove_instid:
            free(target_match.node);

            /* does not fulfill conditions, remove leafref record */
            unres_data_del(node_match, j);
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
static struct lyd_node *
resolve_instid_json(struct lyd_node *data, const char *path, int line)
{
    int i = 0, j;
    struct lyd_node *result = NULL;
    struct lys_module *mod = NULL;
    struct ly_ctx *ctx = data->schema->module->ctx;
    const char *model, *name;
    char *str;
    int mod_len, name_len, has_predicate;
    struct unres_data node_match;
    uint32_t k;

    memset(&node_match, 0, sizeof node_match);

    /* we need root to resolve absolute path */
    for (; data->parent; data = data->parent);
    /* we're still parsing it and the pointer is not correct yet */
    if (data->prev) {
        for (; data->prev->next; data = data->prev);
    }

    /* search for the instance node */
    while (path[i]) {
        j = parse_instance_identifier_json(&path[i], &model, &mod_len, &name, &name_len, &has_predicate);
        if (j <= 0) {
            LOGVAL(LYE_INCHAR, line, path[i-j], &path[i-j]);
            goto error;
        }
        i += j;

        str = strndup(model, mod_len);
        mod = ly_ctx_get_module(ctx, str, NULL);
        free(str);

        if (!mod) {
            /* no instance exists */
            return NULL;
        }

        if (resolve_data(mod, name, name_len, data, &node_match)) {
            /* no instance exists */
            return NULL;
        }

        if (has_predicate) {
            /* we have predicate, so the current results must be list or leaf-list */
            for (k = 0; k < node_match.count;) {
                if ((node_match.node[k]->schema->nodetype == LYS_LIST &&
                        ((struct lys_node_list *)node_match.node[k]->schema)->keys)
                        || (node_match.node[k]->schema->nodetype == LYS_LEAFLIST)) {
                    /* instid is ok, continue check with next instid */
                    ++k;
                    continue;
                }

                /* does not fulfill conditions, remove inst record */
                unres_data_del(&node_match, k);
            }

            j = resolve_predicate_json(&path[i], &node_match);
            if (j < 1) {
                LOGVAL(LYE_INPRED, line, &path[i-j]);
                goto error;
            }
            i += j;

            if (!node_match.count) {
                /* no instance exists */
                return NULL;
            }
        }
    }

    if (!node_match.count) {
        /* no instance exists */
        return NULL;
    } else if (node_match.count > 1) {
        /* instance identifier must resolve to a single node */
        LOGVAL(LYE_TOOMANY, line, path, "data tree");

        /* cleanup */
        free(node_match.node);

        return NULL;
    } else {
        /* we have required result, remember it and cleanup */
        result = node_match.node[0];
        free(node_match.node);

        return result;
    }

error:

    /* cleanup */
    free(node_match.node);

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
 * @brief Resolve augment target. Does not log.
 *
 * @param[in] aug Augment to use.
 * @param[in] siblings Nodes where to start the search in.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
int
resolve_augment(struct lys_node_augment *aug, struct lys_node *siblings)
{
    int rc;
    struct lys_node *sub;

    assert(aug);

    /* resolve target node */
    rc = resolve_schema_nodeid(aug->target_name, siblings, aug->module, LYS_AUGMENT, &aug->target);
    if (rc) {
        return rc;
    }

    if (!aug->child) {
        /* nothing to do */
        LOGWRN("Augment \"%s\" without children.", aug->target_name);
        return EXIT_SUCCESS;
    }

    /* inherit config information from parent, augment does not have
     * config property, but we need to keep the information for subelements
     */
    aug->flags |= aug->target->flags & LYS_CONFIG_MASK;
    LY_TREE_FOR(aug->child, sub) {
        inherit_config_flag(sub);
    }

    /* check identifier uniquness as in lys_node_addchild() */
    LY_TREE_FOR(aug->child, sub) {
        if (lys_check_id(sub, aug->parent, aug->module)) {
            return -1;
        }
    }
    /* reconnect augmenting data into the target - add them to the target child list */
    if (aug->target->child) {
        sub = aug->target->child->prev; /* remember current target's last node */
        sub->next = aug->child;         /* connect augmenting data after target's last node */
        aug->target->child->prev = aug->child->prev; /* new target's last node is last augmenting node */
        aug->child->prev = sub;         /* finish connecting of both child lists */
    } else {
        aug->target->child = aug->child;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve uses, apply augments, refines. Logs directly.
 *
 * @param[in] uses Uses to use.
 * @param[in,out] unres List of unresolved items.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
resolve_uses(struct lys_node_uses *uses, struct unres_schema *unres, uint32_t line)
{
    struct ly_ctx *ctx;
    struct lys_node *node = NULL, *node_aux;
    struct lys_refine *rfn;
    struct lys_restr *must, **old_must;
    int i, j, rc;
    uint8_t size, *old_size;

    assert(uses->grp);
    /* HACK just check that the grouing is resolved */
    assert(!uses->grp->nacm);

    /* copy the data nodes from grouping into the uses context */
    LY_TREE_FOR(uses->grp->child, node) {
        node_aux = lys_node_dup(uses->module, node, uses->flags, uses->nacm, 1, unres);
        if (!node_aux) {
            LOGVAL(LYE_SPEC, line, "Copying data from grouping failed.");
            return -1;
        }
        if (lys_node_addchild((struct lys_node *)uses, NULL, node_aux)) {
            /* error logged */
            lys_node_free(node_aux);
            return -1;
        }
    }
    ctx = uses->module->ctx;

    /* we managed to copy the grouping, the rest must be possible to resolve */

    /* apply refines */
    for (i = 0; i < uses->refine_size; i++) {
        rfn = &uses->refine[i];
        rc = resolve_schema_nodeid(rfn->target_name, uses->child, uses->module, LYS_LEAF, &node);
        if (rc) {
            LOGVAL(LYE_INARG, line, rfn->target_name, "refine");
            return -1;
        }

        if (rfn->target_type && !(node->nodetype & rfn->target_type)) {
            LOGVAL(LYE_SPEC, line, "Refine substatements not applicable to the target-node.");
            return -1;
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
                rc = resolve_schema_nodeid(rfn->mod.dflt, node->child, node->module, LYS_CHOICE, &((struct lys_node_choice *)node)->dflt);
                if (rc) {
                    LOGVAL(LYE_INARG, line, rfn->mod.dflt, "default");
                    return -1;
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
            switch (node->nodetype) {
            case LYS_LEAF:
                old_size = &((struct lys_node_leaf *)node)->must_size;
                old_must = &((struct lys_node_leaf *)node)->must;
                break;
            case LYS_LEAFLIST:
                old_size = &((struct lys_node_leaflist *)node)->must_size;
                old_must = &((struct lys_node_leaflist *)node)->must;
                break;
            case LYS_LIST:
                old_size = &((struct lys_node_list *)node)->must_size;
                old_must = &((struct lys_node_list *)node)->must;
                break;
            case LYS_CONTAINER:
                old_size = &((struct lys_node_container *)node)->must_size;
                old_must = &((struct lys_node_container *)node)->must;
                break;
            case LYS_ANYXML:
                old_size = &((struct lys_node_anyxml *)node)->must_size;
                old_must = &((struct lys_node_anyxml *)node)->must;
                break;
            default:
                LOGINT;
                return -1;
            }

            size = *old_size + rfn->must_size;
            must = realloc(*old_must, size * sizeof *rfn->must);
            if (!must) {
                LOGMEM;
                return -1;
            }
            for (i = 0, j = *old_size; i < rfn->must_size; i++, j++) {
                must[j].expr = lydict_insert(ctx, rfn->must[i].expr, 0);
                must[j].dsc = lydict_insert(ctx, rfn->must[i].dsc, 0);
                must[j].ref = lydict_insert(ctx, rfn->must[i].ref, 0);
                must[j].eapptag = lydict_insert(ctx, rfn->must[i].eapptag, 0);
                must[j].emsg = lydict_insert(ctx, rfn->must[i].emsg, 0);
            }

            *old_must = must;
            *old_size = size;
        }
    }

    /* apply augments */
    for (i = 0; i < uses->augment_size; i++) {
        rc = resolve_augment(&uses->augment[i], uses->child);
        if (rc) {
            LOGVAL(LYE_INRESOLV, line, "augment", uses->augment[i].target_name);
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve base identity recursively. Does not log.
 *
 * @param[in] module Main module.
 * @param[in] ident Identity to use.
 * @param[in] basename Base name of the identity.
 * @param[out] ret Pointer to the resolved identity. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference.
 */
static int
resolve_base_ident_sub(struct lys_module *module, struct lys_ident *ident, const char *basename,
                       struct lys_ident **ret)
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
                if (ret) {
                    *ret = &module->ident[i];
                }
                return EXIT_SUCCESS;
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
                        if (ret) {
                            *ret = &module->inc[j].submodule->ident[i];
                        }
                        return EXIT_SUCCESS;
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
        if (ret) {
            *ret = ident->base;
        }
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

/**
 * @brief Resolve base identity. Logs directly.
 *
 * @param[in] module Main module.
 * @param[in] ident Identity to use.
 * @param[in] basename Base name of the identity.
 * @param[in] parent Either "type" or "ident".
 * @param[in] first Whether this is the first resolution try. Affects logging.
 * @param[in] line Line in the input file.
 * @param[out] ret Pointer to the resolved identity. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_base_ident(struct lys_module *module, struct lys_ident *ident, const char *basename, const char* parent,
                   int first, uint32_t line, struct lys_ident **ret)
{
    const char *name;
    int i, prefix_len = 0;

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
            LOGVAL(LYE_INMOD, line, basename);
            return -1;
        }
    } else {
        /* search in submodules */
        for (i = 0; i < module->inc_size; i++) {
            if (!resolve_base_ident_sub((struct lys_module *)module->inc[i].submodule, ident, name, ret)) {
                return EXIT_SUCCESS;
            }
        }
    }

    /* search in the identified module */
    if (!resolve_base_ident_sub(module, ident, name, ret)) {
        return EXIT_SUCCESS;
    }

    if (!first) {
        LOGVAL(LYE_INARG, line, basename, parent);
    }
    return EXIT_FAILURE;
}

/**
 * @brief Resolve JSON format identityref. Logs directly.
 *
 * @param[in] base Base identity.
 * @param[in] ident_name Identityref name.
 * @param[in] line Line from the input file.
 *
 * @return Pointer to the identity resolvent, NULL on error.
 */
struct lys_ident *
resolve_identref_json(struct lys_ident *base, const char *ident_name, uint32_t line)
{
    const char *mod_name, *name;
    int mod_name_len, rc;
    struct lys_ident_der *der;

    if (!base || !ident_name) {
        return NULL;
    }

    rc = parse_node_identifier(ident_name, &mod_name, &mod_name_len, &name, NULL);
    if (rc < (signed)strlen(ident_name)) {
        LOGVAL(LYE_INCHAR, line, ident_name[-rc], &ident_name[-rc]);
        return NULL;
    }

    if (!strcmp(base->name, name) && (!mod_name
            || (!strncmp(base->module->name, mod_name, mod_name_len) && !base->module->name[mod_name_len]))) {
        return base;
    }

    for (der = base->der; der; der = der->next) {
        if (!strcmp(der->ident->name, name) && (!mod_name
                || (!strncmp(der->ident->module->name, mod_name, mod_name_len)
                && !der->ident->module->name[mod_name_len]))) {
            /* we have match */
            return der->ident;
        }
    }

    LOGVAL(LYE_INRESOLV, line, "identityref", ident_name);
    return NULL;
}

/**
 * @brief Resolve (find) choice default case. Does not log.
 *
 * @param[in] choic Choice to use.
 * @param[in] dflt Name of the default case.
 *
 * @return Pointer to the default node or NULL.
 */
static struct lys_node *
resolve_choice_dflt(struct lys_node_choice *choic, const char *dflt)
{
    struct lys_node *child, *ret;

    LY_TREE_FOR(choic->child, child) {
        if (child->nodetype == LYS_USES) {
            ret = resolve_choice_dflt((struct lys_node_choice *)child, dflt);
            if (ret) {
                return ret;
            }
        }

        if ((child->name == dflt) && (child->nodetype & (LYS_ANYXML | LYS_CASE
                | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST))) {
            return child;
        }
    }

    return NULL;
}

/**
 * @brief Resolve unresolved uses. Logs directly.
 *
 * @param[in] uses Uses to use.
 * @param[in] unres Specific unres item.
 * @param[in] first Whether this is the first resolution try.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_unres_schema_uses(struct lys_node_uses *uses, struct unres_schema *unres, int first, uint32_t line)
{
    int rc;
    struct lys_node *parent;

    /* HACK change unres uses count if it's in a grouping (nacm field used for it) */
    for (parent = uses->parent; parent && (parent->nodetype != LYS_GROUPING); parent = parent->parent);

    if (!uses->grp) {
        rc = resolve_grouping(uses, first, line);
        if (rc) {
            if (parent && first && (rc == EXIT_FAILURE)) {
                ++parent->nacm;
            }
            return rc;
        }
    }

    if (uses->grp->nacm) {
        if (parent && first) {
            ++parent->nacm;
        }
        return EXIT_FAILURE;
    }

    rc = resolve_uses(uses, unres, line);
    if (!rc) {
        /* decrease unres count only if not first try */
        if (parent && !first) {
            if (!parent->nacm) {
                LOGINT;
                return -1;
            }
            --parent->nacm;
        }
        return EXIT_SUCCESS;
    }

    if (parent && first && (rc == EXIT_FAILURE)) {
        ++parent->nacm;
    }
    return rc;
}

/**
 * @brief Resolve list keys. Logs directly.
 *
 * @param[in] list List to use.
 * @param[in] keys_str Keys node value.
 * @param[in] first Whether this is the first resolution try. Affects logging.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_list_keys(struct lys_module *mod, struct lys_node_list *list, const char *keys_str, int first, uint32_t line)
{
    int i, len, rc;
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

        rc = resolve_sibling(mod, list->child, NULL, 0, keys_str, len, LYS_LEAF, (struct lys_node **)&list->keys[i]);
        if (rc) {
            if ((rc == -1) || !first) {
                LOGVAL(LYE_INRESOLV, line, "list keys", keys_str);
            }
            return rc;
        }

        if (check_key(list->keys[i], list->flags, list->keys, i, keys_str, len, line)) {
            /* check_key logs */
            return -1;
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
 * @brief Resolve (check) all must conditions of \p node.
 * Logs directly.
 *
 * @param[in] node Data node with optional must statements.
 * @param[in] first Whether this is the first resolution to try.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on pass, EXIT_FAILURE on fail, -1 on error.
 */
static int
resolve_must(struct lyd_node *node, int first, uint32_t line)
{
    uint8_t i, must_size;
    struct lys_restr *must;
    struct lyxp_set set;

    assert(node);
    memset(&set, 0, sizeof set);

    switch (node->schema->nodetype) {
    case LYS_CONTAINER:
        must_size = ((struct lys_node_container *)node->schema)->must_size;
        must = ((struct lys_node_container *)node->schema)->must;
        break;
    case LYS_LEAF:
        must_size = ((struct lys_node_leaf *)node->schema)->must_size;
        must = ((struct lys_node_leaf *)node->schema)->must;
        break;
    case LYS_LEAFLIST:
        must_size = ((struct lys_node_leaflist *)node->schema)->must_size;
        must = ((struct lys_node_leaflist *)node->schema)->must;
        break;
    case LYS_LIST:
        must_size = ((struct lys_node_list *)node->schema)->must_size;
        must = ((struct lys_node_list *)node->schema)->must;
        break;
    case LYS_ANYXML:
        must_size = ((struct lys_node_anyxml *)node->schema)->must_size;
        must = ((struct lys_node_anyxml *)node->schema)->must;
        break;
    default:
        must_size = 0;
        break;
    }

    for (i = 0; i < must_size; ++i) {
        if (lyxp_eval(must[i].expr, node, &set, line)) {
            return -1;
        }

        lyxp_set_cast(&set, LYXP_SET_BOOLEAN, node);

        if (!set.value.bool) {
            if (!first) {
                LOGVAL(LYE_NOCOND, line, "Must", must[i].expr);
            }
            return 1;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve (find) when condition context node. Does not log.
 *
 * @param[in] node Data node, whose conditional definition is being decided.
 * @param[in] schema Schema node with a when condition.
 *
 * @return Context node.
 */
static struct lyd_node *
resolve_when_ctx_node(struct lyd_node *node, struct lys_node *schema)
{
    struct lyd_node *parent;
    struct lys_node *sparent;
    uint16_t i, data_depth, schema_depth;

    /* find a not schema-only node */
    while (schema->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE | LYS_AUGMENT | LYS_INPUT | LYS_OUTPUT)) {
        schema = lys_parent(schema);
        if (!schema) {
            return NULL;
        }
    }

    /* get node depths */
    for (parent = node, data_depth = 0; parent; parent = parent->parent, ++data_depth);
    for (sparent = lys_parent(schema), schema_depth = 1; sparent; sparent = lys_parent(sparent)) {
        if (sparent->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYXML | LYS_NOTIF | LYS_RPC)) {
            ++schema_depth;
        }
    }
    if (data_depth < schema_depth) {
        return NULL;
    }

    /* find the corresponding data node */
    for (i = 0; i < data_depth - schema_depth; ++i) {
        node = node->parent;
    }
    if (node->schema != schema) {
        return NULL;
    }

    return node;
}

/**
 * @brief Resolve (check) all when conditions relevant for \p node.
 * Logs directly.
 *
 * @param[in] node Data node, whose conditional reference, if such, is being decided.
 * @param[in] first Whether this is the first resolution to try.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on pass, EXIT_FAILURE on fail, -1 on error.
 */
static int
resolve_when(struct lyd_node *node, int first, uint32_t line)
{
    struct lyd_node *ctx_node = NULL;
    struct lys_node *parent;
    struct lyxp_set set;

    assert(node);
    memset(&set, 0, sizeof set);

    if (!(node->schema->nodetype & (LYS_NOTIF | LYS_RPC)) && (((struct lys_node_container *)node->schema)->when)) {
        if (lyxp_eval(((struct lys_node_container *)node->schema)->when->cond, node, &set, line)) {
            return -1;
        }

        lyxp_set_cast(&set, LYXP_SET_BOOLEAN, node);

        if (!set.value.bool) {
            if (!first) {
                LOGVAL(LYE_NOCOND, line, "When", ((struct lys_node_container *)node->schema)->when->cond);
            }
            return 1;
        }
    }

    parent = node->schema;
    goto check_augment;

    /* check when in every schema node that affects node */
    while (parent && (parent->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE))) {
        if (((struct lys_node_uses *)parent)->when) {
            if (!ctx_node) {
                ctx_node = resolve_when_ctx_node(node, parent);
                if (!ctx_node) {
                    LOGINT;
                    return -1;
                }
            }
            if (lyxp_eval(((struct lys_node_uses *)parent)->when->cond, ctx_node, &set, line)) {
                return -1;
            }

            lyxp_set_cast(&set, LYXP_SET_BOOLEAN, ctx_node);

            if (!set.value.bool) {
                if (!first) {
                    LOGVAL(LYE_NOCOND, line, "When", ((struct lys_node_uses *)parent)->when->cond);
                }
                return 1;
            }
        }

check_augment:
        if ((parent->parent && (parent->parent->nodetype == LYS_AUGMENT) && (((struct lys_node_augment *)parent->parent)->when))) {
            if (!ctx_node) {
                ctx_node = resolve_when_ctx_node(node, parent->parent);
                if (!ctx_node) {
                    LOGINT;
                    return -1;
                }
            }
            if (lyxp_eval(((struct lys_node_augment *)parent->parent)->when->cond, ctx_node, &set, line)) {
                return -1;
            }

            lyxp_set_cast(&set, LYXP_SET_BOOLEAN, ctx_node);

            if (!set.value.bool) {
                if (!first) {
                    LOGVAL(LYE_NOCOND, line, "When", ((struct lys_node_augment *)parent->parent)->when->cond);
                }
                return 1;
            }
        }

        parent = lys_parent(parent);
    }

    return 0;
}

/**
 * @brief Resolve a single unres schema item. Logs indirectly.
 *
 * @param[in] mod Main module.
 * @param[in] item Item to resolve. Type determined by \p type.
 * @param[in] type Type of the unresolved item.
 * @param[in] str_snode String, a schema node, or NULL.
 * @param[in] unres Unres schema structure to use.
 * @param[in] first Whether this is the first resolution try.
 * @param[in] line Line in the input file. 0 skips line print.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_unres_schema_item(struct lys_module *mod, void *item, enum UNRES_ITEM type, void *str_snode,
                          struct unres_schema *unres, int first, uint32_t line)
{
    int rc = -1, has_str = 0;
    struct lys_node *node;
    const char *base_name;

    struct lys_ident *ident;
    struct lys_type *stype;
    struct lys_feature **feat_ptr;
    struct lys_node_choice *choic;
    struct lys_unique *uniq;

    switch (type) {
    case UNRES_IDENT:
        base_name = str_snode;
        ident = item;

        rc = resolve_base_ident(mod, ident, base_name, "ident", first, line, NULL);
        has_str = 1;
        break;
    case UNRES_TYPE_IDENTREF:
        base_name = str_snode;
        stype = item;

        rc = resolve_base_ident(mod, NULL, base_name, "type", first, line, &stype->info.ident.ref);
        has_str = 1;
        break;
    case UNRES_TYPE_LEAFREF:
        node = str_snode;
        stype = item;

        rc = resolve_path_arg_schema(mod, stype->info.lref.path, node, first, line,
                                     (struct lys_node **)&stype->info.lref.target);
        has_str = 0;
        break;
    case UNRES_TYPE_DER:
        base_name = str_snode;
        stype = item;

        /* HACK type->der is temporarily its parent */
        rc = resolve_superior_type(base_name, stype->prefix, mod, (struct lys_node *)stype->der, &stype->der);
        if (rc == -1) {
            LOGVAL(LYE_INMOD, line, stype->module_name);
        } else if (!rc) {
            stype->base = stype->der->type.base;
        }
        has_str = 1;
        break;
    case UNRES_IFFEAT:
        base_name = str_snode;
        feat_ptr = item;

        rc = resolve_feature(base_name, mod, first, line, feat_ptr);
        has_str = 1;
        break;
    case UNRES_USES:
        rc = resolve_unres_schema_uses(item, unres, first, line);
        has_str = 0;
        break;
    case UNRES_TYPE_DFLT:
        base_name = str_snode;
        stype = item;

        rc = check_default(stype, base_name, first, line);
        /* do not remove base_name (dflt), it's in a typedef */
        has_str = 0;
        break;
    case UNRES_CHOICE_DFLT:
        base_name = str_snode;
        choic = item;

        choic->dflt = resolve_choice_dflt(choic, base_name);
        if (choic->dflt) {
            rc = EXIT_SUCCESS;
        } else {
            rc = EXIT_FAILURE;
        }
        has_str = 1;
        break;
    case UNRES_LIST_KEYS:
        rc = resolve_list_keys(mod, item, str_snode, first, line);
        has_str = 1;
        break;
    case UNRES_LIST_UNIQ:
        /* actually the unique string */
        base_name = str_snode;
        uniq = item;

        rc = resolve_unique((struct lys_node *)uniq->leafs, base_name, uniq, first, line);
        has_str = 1;
        break;
    default:
        LOGINT;
        break;
    }

    if (has_str && !rc) {
        lydict_remove(mod->ctx, str_snode);
    }

    return rc;
}

/* logs directly */
static void
print_unres_schema_item_fail(void *item, enum UNRES_ITEM type, void *str_node, uint32_t line)
{
    char line_str[18];

    if (line) {
        sprintf(line_str, " (line %u)", line);
    } else {
        line_str[0] = '\0';
    }

    switch (type) {
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
    default:
        LOGINT;
        break;
    }
}

/**
 * @brief Resolve every unres schema item in the structure. Logs directly.
 *
 * @param[in] mod Main module.
 * @param[in] unres Unres schema structure to use.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
int
resolve_unres_schema(struct lys_module *mod, struct unres_schema *unres)
{
    uint32_t i, resolved, unres_uses, res_uses;
    int rc;

    assert(unres);

    resolved = 0;

    /* uses */
    do {
        unres_uses = 0;
        res_uses = 0;

        for (i = 0; i < unres->count; ++i) {
            if (unres->type[i] != UNRES_USES) {
                continue;
            }

            ++unres_uses;
            rc = resolve_unres_schema_item(mod, unres->item[i], unres->type[i], unres->str_snode[i], unres, 0,
                                           LOGLINE_IDX(unres, i));
            if (!rc) {
                unres->type[i] = UNRES_RESOLVED;
                ++resolved;
                ++res_uses;
            } else if (rc == -1) {
                return -1;
            }
        }
    } while (res_uses && (res_uses < unres_uses));

    if (res_uses < unres_uses) {
        LOGVAL(LYE_SPEC, 0, "There are unresolved uses left.");
        return -1;
    }

    /* the rest */
    for (i = 0; i < unres->count; ++i) {
        if (unres->type[i] == UNRES_RESOLVED) {
            continue;
        }

        rc = resolve_unres_schema_item(mod, unres->item[i], unres->type[i], unres->str_snode[i], unres, 0,
                                       LOGLINE_IDX(unres, i));
        if (rc) {
            return rc;
        }

        unres->type[i] = UNRES_RESOLVED;
        ++resolved;
    }

    if (resolved < unres->count) {
        LOGVAL(LYE_SPEC, 0, "There are unresolved schema items left.");
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Try to resolve an unres schema item with a string argument. Logs indirectly.
 *
 * @param[in] mod Main module.
 * @param[in] unres Unres schema structure to use.
 * @param[in] item Item to resolve. Type determined by \p type.
 * @param[in] type Type of the unresolved item.
 * @param[in] str String argument.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success or storing the item in unres, -1 on error.
 */
int
unres_schema_add_str(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type, const char *str,
                     uint32_t line)
{
    str = lydict_insert(mod->ctx, str, 0);
    return unres_schema_add_node(mod, unres, item, type, (struct lys_node *)str, line);
}

/**
 * @brief Try to resolve an unres schema item with a schema node argument. Logs indirectly.
 *
 * @param[in] mod Main module.
 * @param[in] unres Unres schema structure to use.
 * @param[in] item Item to resolve. Type determined by \p type.
 * @param[in] type Type of the unresolved item.
 * @param[in] snode Schema node argument.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success or storing the item in unres, -1 on error.
 */
int
unres_schema_add_node(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type,
                      struct lys_node *snode, uint32_t line)
{
    int rc;

    assert(unres && item && ((type != UNRES_LEAFREF) && (type != UNRES_INSTID) && (type != UNRES_WHEN)));

    rc = resolve_unres_schema_item(mod, item, type, snode, unres, 1, line);
    if (rc != EXIT_FAILURE) {
        return rc;
    }

    print_unres_schema_item_fail(item, type, snode, line);

    unres->count++;
    unres->item = realloc(unres->item, unres->count*sizeof *unres->item);
    unres->item[unres->count-1] = item;
    unres->type = realloc(unres->type, unres->count*sizeof *unres->type);
    unres->type[unres->count-1] = type;
    unres->str_snode = realloc(unres->str_snode, unres->count*sizeof *unres->str_snode);
    unres->str_snode[unres->count-1] = snode;
#ifndef NDEBUG
    unres->line = realloc(unres->line, unres->count*sizeof *unres->line);
    unres->line[unres->count-1] = line;
#endif

    return EXIT_SUCCESS;
}

/**
 * @brief Duplicate an unres schema item. Logs indirectly.
 *
 * @param[in] mod Main module.
 * @param[in] unres Unres schema structure to use.
 * @param[in] item Old item to be resolved.
 * @param[in] type Type of the old unresolved item.
 * @param[in] new_item New item to use in the duplicate.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
int
unres_schema_dup(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type, void *new_item)
{
    int i;

    assert(item && new_item && ((type != UNRES_LEAFREF) && (type != UNRES_INSTID) && (type != UNRES_WHEN)));

    i = unres_schema_find(unres, item, type);

    if (i == -1) {
        return -1;
    }

    if ((type == UNRES_TYPE_LEAFREF) || (type == UNRES_USES) || (type == UNRES_TYPE_DFLT)) {
        if (unres_schema_add_node(mod, unres, new_item, type, unres->str_snode[i], 0) == -1) {
            LOGINT;
            return -1;
        }
    } else {
        if (unres_schema_add_str(mod, unres, new_item, type, unres->str_snode[i], 0) == -1) {
            LOGINT;
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

/* does not log */
int
unres_schema_find(struct unres_schema *unres, void *item, enum UNRES_ITEM type)
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

/* logs directly */
static void
print_unres_data_item_fail(struct lyd_node *node, enum UNRES_ITEM type, uint32_t line)
{
    struct lys_node_leaf *sleaf;
    char line_str[18];

    if (line) {
        sprintf(line_str, " (line %u)", line);
    } else {
        line_str[0] = '\0';
    }

    sleaf = (struct lys_node_leaf *)node->schema;

    switch (type) {
    case UNRES_LEAFREF:
        LOGVRB("Leafref \"%s\" could not be resolved, it will be attempted later%s.",
               sleaf->type.info.lref.path, line_str);
        break;
    case UNRES_INSTID:
        LOGVRB("Instance-identifier \"%s\" could not be resolved, it will be attempted later%s.",
               ((struct lyd_node_leaf_list *)node)->value_str, line_str);
        break;
    case UNRES_WHEN:
        LOGVRB("There was an unsatisfied when condition, evaluation will be attempted later%s.", line_str);
        break;
    case UNRES_MUST:
        LOGVRB("There was an unsatisfied must condition, evaluation will be attempted later%s.", line_str);
        break;
    default:
        LOGINT;
        break;
    }
}

/**
 * @brief Resolve a single unres data item. Logs directly.
 *
 * @param[in] node Data node to resolve.
 * @param[in] first Whether this is the first resolution try.
 * @param[in] type Type of the unresolved item.
 * @param[in] line Line in the input file. 0 skips line print.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
int
resolve_unres_data_item(struct lyd_node *node, enum UNRES_ITEM type, int first, uint32_t line)
{
    uint32_t i;
    int rc;
    struct lyd_node_leaf_list *leaf;
    struct lys_node_leaf *sleaf;
    struct unres_data matches;

    memset(&matches, 0, sizeof matches);
    leaf = (struct lyd_node_leaf_list *)node;
    sleaf = (struct lys_node_leaf *)leaf->schema;

    switch (type) {
    case UNRES_LEAFREF:
        assert(sleaf->type.base == LY_TYPE_LEAFREF);
        if ((rc = resolve_path_arg_data(node, sleaf->type.info.lref.path, first, line, &matches))) {
            return rc;
        }

        /* check that value matches */
        for (i = 0; i < matches.count; ++i) {
            if (leaf->value_str == ((struct lyd_node_leaf_list *)matches.node[i])->value_str) {
                leaf->value.leafref = matches.node[i];
                break;
            }
        }

        free(matches.node);
        memset(&matches, 0, sizeof matches);

        if (!leaf->value.leafref) {
            /* reference not found */
            if (!first) {
                LOGVAL(LYE_SPEC, line, "Leafref \"%s\" value \"%s\" did not match any node value.",
                       sleaf->type.info.lref.path, leaf->value_str);
            }
            return EXIT_FAILURE;
        }
        break;

    case UNRES_INSTID:
        assert(sleaf->type.base == LY_TYPE_INST);
        ly_errno = 0;
        if (!resolve_instid_json(node, leaf->value_str, line)) {
            if (ly_errno) {
                return -1;
            } else if (sleaf->type.info.inst.req > -1) {
                if (!first) {
                    LOGVAL(LYE_SPEC, line, "There is no instance of \"%s\".", leaf->value_str);
                }
                return EXIT_FAILURE;
            } else {
                LOGVRB("There is no instance of \"%s\", but is not required.", leaf->value_str);
            }
        }
        break;

    case UNRES_WHEN:
        if ((rc = resolve_when(node, first, line))) {
            return rc;
        }
        break;

    case UNRES_MUST:
        if ((rc = resolve_must(node, first, line))) {
            return rc;
        }
        break;

    default:
        LOGINT;
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Try to resolve an unres data item. Logs indirectly.
 *
 * @param[in] unres Unres data structure to use.
 * @param[in] node Data node to use.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success or storing the item in unres, -1 on error.
 */
int
unres_data_add(struct unres_data *unres, struct lyd_node *node, enum UNRES_ITEM type, uint32_t line)
{
    int rc;

    assert(unres && node && ((type == UNRES_LEAFREF) || (type == UNRES_INSTID) || (type == UNRES_WHEN) || (type == UNRES_MUST)));

    rc = resolve_unres_data_item(node, type, 1, line);
    if (rc != EXIT_FAILURE) {
        return rc;
    }

    print_unres_data_item_fail(node, type, line);

    ++unres->count;
    unres->node = realloc(unres->node, unres->count * sizeof *unres->node);
    unres->node[unres->count - 1] = node;
    unres->type = realloc(unres->type, unres->count * sizeof *unres->type);
    unres->type[unres->count - 1] = type;
#ifndef NDEBUG
    unres->line = realloc(unres->line, unres->count * sizeof *unres->line);
    unres->line[unres->count - 1] = line;
#endif

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve every unres data item in the structure. Logs directly.
 *
 * @param[in] unres Unres data structure to use.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
int
resolve_unres_data(struct unres_data *unres)
{
    uint32_t i;
    int rc;

    for (i = 0; i < unres->count; ++i) {
        rc = resolve_unres_data_item(unres->node[i], unres->type[i], 0, LOGLINE_IDX(unres, i));
        if (rc) {
            LOGVAL(LYE_SPEC, 0, "There are unresolved data items left.");
            return -1;
        }
    }

    return EXIT_SUCCESS;
}
