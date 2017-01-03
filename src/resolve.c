/**
 * @file resolve.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang resolve functions
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
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
#include "parser_yang.h"
#include "xml_internal.h"
#include "dict_private.h"
#include "tree_internal.h"

int
parse_range_dec64(const char **str_num, uint8_t dig, int64_t *num)
{
    const char *ptr;
    int minus = 0;
    int64_t ret = 0;
    int8_t str_exp, str_dig = -1, trailing_zeros = 0;

    ptr = *str_num;

    if (ptr[0] == '-') {
        minus = 1;
        ++ptr;
    } else if (ptr[0] == '+') {
        ++ptr;
    }

    if (!isdigit(ptr[0])) {
        /* there must be at least one */
        return 1;
    }

    for (str_exp = 0; isdigit(ptr[0]) || ((ptr[0] == '.') && (str_dig < 0)); ++ptr) {
        if (str_exp > 18) {
            return 1;
        }

        if (ptr[0] == '.') {
            if (ptr[1] == '.') {
                /* it's the next interval */
                break;
            }
            ++str_dig;
        } else {
            ret = ret * 10 + (ptr[0] - '0');
            if (str_dig > -1) {
                ++str_dig;
                if (ptr[0] == '0') {
                    /* possibly trailing zero */
                    trailing_zeros++;
                } else {
                    trailing_zeros = 0;
                }
            }
            ++str_exp;
        }
    }
    if (str_dig == 0) {
        /* no digits after '.' */
        return 1;
    } else if (str_dig == -1) {
        /* there are 0 numbers after the floating point */
        str_dig = 0;
    }
    /* remove trailing zeros */
    if (trailing_zeros) {
        str_dig -= trailing_zeros;
        str_exp -= trailing_zeros;
        ret = ret / dec_pow(trailing_zeros);
    }

    /* it's parsed, now adjust the number based on fraction-digits, if needed */
    if (str_dig < dig) {
        if ((str_exp - 1) + (dig - str_dig) > 18) {
            return 1;
        }
        ret *= dec_pow(dig - str_dig);
    }
    if (str_dig > dig) {
        return 1;
    }

    if (minus) {
        ret *= -1;
    }
    *str_num = ptr;
    *num = ret;

    return 0;
}

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

    assert(id);

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
 * node-identifier     = [module-name ":"] identifier
 *
 * @param[in] id Identifier to use.
 * @param[out] mod_name Points to the module name, NULL if there is not any.
 * @param[out] mod_name_len Length of the module name, 0 if there is not any.
 * @param[out] name Points to the node name.
 * @param[out] nam_len Length of the node name.
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
parse_node_identifier(const char *id, const char **mod_name, int *mod_name_len, const char **name, int *nam_len)
{
    int parsed = 0, ret;

    assert(id);
    if (mod_name) {
        *mod_name = NULL;
    }
    if (mod_name_len) {
        *mod_name_len = 0;
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

    if (mod_name) {
        *mod_name = id;
    }
    if (mod_name_len) {
        *mod_name_len = ret;
    }

    parsed += ret;
    id += ret;

    /* there is prefix */
    if (id[0] == ':') {
        ++parsed;
        ++id;

    /* there isn't */
    } else {
        if (name && mod_name) {
            *name = *mod_name;
        }
        if (mod_name) {
            *mod_name = NULL;
        }

        if (nam_len && mod_name_len) {
            *nam_len = *mod_name_len;
        }
        if (mod_name_len) {
            *mod_name_len = 0;
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
 * @param[in] mod Module of the context node to get correct prefix in case it is not explicitly specified
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
parse_path_arg(struct lys_module *mod, const char *id, const char **prefix, int *pref_len,
               const char **name, int *nam_len, int *parent_times, int *has_predicate)
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
    if (!(*prefix)) {
        /* actually we always need prefix even it is not specified */
        *prefix = lys_main_module(mod)->name;
        *pref_len = strlen(*prefix);
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
 * @brief Parse instance-identifier in JSON data format. That means that prefixes
 *        (which are mandatory for every node-identifier) are actually model names.
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
parse_instance_identifier(const char *id, const char **model, int *mod_len, const char **name, int *nam_len,
                          int *has_predicate)
{
    int parsed = 0, ret;

    if (has_predicate) {
        *has_predicate = 0;
    }

    if (id[0] != '/') {
        return -parsed;
    }

    ++parsed;
    ++id;

    if ((ret = parse_identifier(id)) < 1) {
        return ret;
    }

    *model = id;
    *mod_len = ret;

    parsed += ret;
    id += ret;

    if (id[0] != ':') {
        return -parsed;
    }

    ++parsed;
    ++id;

    if ((ret = parse_identifier(id)) < 1) {
        return ret;
    }

    *name = id;
    *nam_len = ret;

    parsed += ret;
    id += ret;

    if (id[0] == '[' && has_predicate) {
        *has_predicate = 1;
    }

    return parsed;
}

/**
 * @brief Parse predicate (instance-identifier) in JSON data format. That means that prefixes
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
parse_predicate(const char *id, const char **model, int *mod_len, const char **name, int *nam_len,
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
            return -parsed;
        }

        while (isdigit(id[0])) {
            ++parsed;
            ++id;
        }

        if (nam_len) {
            *nam_len = id-(*name);
        }

    /* "." or node-identifier */
    } else {
        if (id[0] == '.') {
            if (name) {
                *name = id;
            }
            if (nam_len) {
                *nam_len = 1;
            }

            ++parsed;
            ++id;

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
 * descendant-schema-nodeid = ["." "/"]
 *                       node-identifier
 *                       absolute-schema-nodeid
 *
 * @param[in] id Identifier to use.
 * @param[out] mod_name Points to the module name, NULL if there is not any.
 * @param[out] mod_name_len Length of the module name, 0 if there is not any.
 * @param[out] name Points to the node name.
 * @param[out] nam_len Length of the node name.
 * @param[out] is_relative Flag to mark whether the nodeid is absolute or descendant. Must be -1
 *                         on the first call, must not be changed between consecutive calls.
 * @param[out] has_predicate Flag to mark whether there is a predicate specified. It cannot be
 *                           based on the grammar, in those cases use NULL.
 *
 * @return Number of characters successfully parsed,
 *         positive on success, negative on failure.
 */
int
parse_schema_nodeid(const char *id, const char **mod_name, int *mod_name_len, const char **name, int *nam_len,
                    int *is_relative, int *has_predicate)
{
    int parsed = 0, ret;

    assert(id);
    assert(is_relative);
    if (mod_name) {
        *mod_name = NULL;
    }
    if (mod_name_len) {
        *mod_name_len = 0;
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
        if (*is_relative != -1) {
            return -parsed;
        } else {
            *is_relative = 1;
        }
        if (!strncmp(id, "./", 2)) {
            parsed += 2;
            id += 2;
        }
    } else {
        if (*is_relative == -1) {
            *is_relative = 0;
        }
        ++parsed;
        ++id;
    }

    if ((ret = parse_node_identifier(id, mod_name, mod_name_len, name, nam_len)) < 1) {
        return -parsed+ret;
    }

    parsed += ret;
    id += ret;

    if ((id[0] == '[') && has_predicate) {
        *has_predicate = 1;
    }

    return parsed;
}

/**
 * @brief Parse schema predicate (special format internally used).
 *
 * predicate           = "[" *WSP predicate-expr *WSP "]"
 * predicate-expr      = "." / identifier / key-with-value
 * key-with-value      = identifier *WSP "=" *WSP
 *                       ((DQUOTE string DQUOTE) /
 *                        (SQUOTE string SQUOTE))
 *
 * @param[in] id Identifier to use.
 * @param[out] name Points to the list key name.
 * @param[out] nam_len Length of \p name.
 * @param[out] value Points to the key value. If specified, key-with-value is expected.
 * @param[out] val_len Length of \p value.
 * @param[out] has_predicate Flag to mark whether there is another predicate specified.
 */
int
parse_schema_json_predicate(const char *id, const char **name, int *nam_len, const char **value, int *val_len,
                            int *has_predicate)
{
    const char *ptr;
    int parsed = 0, ret;
    char quote;

    assert(id);
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

    /* identifier */
    if (id[0] == '.') {
        ret = 1;
    } else if ((ret = parse_identifier(id)) < 1) {
        return -parsed + ret;
    }
    if (name) {
        *name = id;
    }
    if (nam_len) {
        *nam_len = ret;
    }

    parsed += ret;
    id += ret;

    while (isspace(id[0])) {
        ++parsed;
        ++id;
    }

    /* there is value as well */
    if (id[0] == '=') {
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
            ret = ptr - id;

            if (value) {
                *value = id;
            }
            if (val_len) {
                *val_len = ret;
            }

            parsed += ret + 1;
            id += ret + 1;
        } else {
            return -parsed;
        }

        while (isspace(id[0])) {
            ++parsed;
            ++id;
        }
    } else if (value) {
        /* if value was expected, it's mandatory */
        return -parsed;
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
 * @brief Resolve (find) a feature definition. Logs directly.
 *
 * @param[in] feat_name Feature name to resolve.
 * @param[in] len Length of \p feat_name.
 * @param[in] node Node with the if-feature expression.
 * @param[out] feature Pointer to be set to point to the feature definition, if feature not found
 * (return code 1), the pointer is untouched.
 *
 * @return 0 on success, 1 on forward reference, -1 on error.
 */
static int
resolve_feature(const char *feat_name, uint16_t len, const struct lys_node *node, struct lys_feature **feature)
{
    char *str;
    const char *mod_name, *name;
    int mod_name_len, nam_len, i, j;
    const struct lys_module *module;

    assert(feature);

    /* check prefix */
    if ((i = parse_node_identifier(feat_name, &mod_name, &mod_name_len, &name, &nam_len)) < 1) {
        LOGVAL(LYE_INCHAR, LY_VLOG_NONE, NULL, feat_name[-i], &feat_name[-i]);
        return -1;
    }

    module = lys_get_import_module(lys_node_module(node), NULL, 0, mod_name, mod_name_len);
    if (!module) {
        /* identity refers unknown data model */
        LOGVAL(LYE_INMOD_LEN, LY_VLOG_NONE, NULL, mod_name_len, mod_name);
        return -1;
    }

    if (module != node->module && module == lys_node_module(node)) {
        /* first, try to search directly in submodule where the feature was mentioned */
        for (j = 0; j < node->module->features_size; j++) {
            if (!strncmp(name, node->module->features[j].name, nam_len) && !node->module->features[j].name[nam_len]) {
                /* check status */
                if (lyp_check_status(node->flags, lys_node_module(node), node->name, node->module->features[j].flags,
                                     node->module->features[j].module, node->module->features[j].name, NULL)) {
                    return -1;
                }
                *feature = &node->module->features[j];
                return 0;
            }
        }
    }

    /* search in the identified module ... */
    for (j = 0; j < module->features_size; j++) {
        if (!strncmp(name, module->features[j].name, nam_len) && !module->features[j].name[nam_len]) {
            /* check status */
            if (lyp_check_status(node->flags, lys_node_module(node), node->name, module->features[j].flags,
                                 module->features[j].module, module->features[j].name, NULL)) {
                return -1;
            }
            *feature = &module->features[j];
            return 0;
        }
    }
    /* ... and all its submodules */
    for (i = 0; i < module->inc_size; i++) {
        if (!module->inc[i].submodule) {
            /* not yet resolved */
            continue;
        }
        for (j = 0; j < module->inc[i].submodule->features_size; j++) {
            if (!strncmp(name, module->inc[i].submodule->features[j].name, nam_len)
                    && !module->inc[i].submodule->features[j].name[nam_len]) {
                /* check status */
                if (lyp_check_status(node->flags, lys_node_module(node), node->name,
                                     module->inc[i].submodule->features[j].flags,
                                     module->inc[i].submodule->features[j].module,
                                     module->inc[i].submodule->features[j].name, NULL)) {
                    return -1;
                }
                *feature = &module->inc[i].submodule->features[j];
                return 0;
            }
        }
    }

    /* not found */
    str = strndup(feat_name, len);
    LOGVAL(LYE_INRESOLV, LY_VLOG_NONE, NULL, "feature", str);
    free(str);
    return 1;
}

/*
 * @return
 *  -  1 if enabled
 *  -  0 if disabled
 *  - -1 if not usable by its if-feature expression
 */
static int
resolve_feature_value(const struct lys_feature *feat)
{
    int i;

    for (i = 0; i < feat->iffeature_size; i++) {
        if (!resolve_iffeature(&feat->iffeature[i])) {
            return -1;
        }
    }

    return feat->flags & LYS_FENABLED ? 1 : 0;
}

static int
resolve_iffeature_recursive(struct lys_iffeature *expr, int *index_e, int *index_f)
{
    uint8_t op;
    int rc, a, b;

    op = iff_getop(expr->expr, *index_e);
    (*index_e)++;

    switch (op) {
    case LYS_IFF_F:
        /* resolve feature */
        return resolve_feature_value(expr->features[(*index_f)++]);
    case LYS_IFF_NOT:
        rc = resolve_iffeature_recursive(expr, index_e, index_f);
        if (rc == -1) {
            /* one of the referenced feature is hidden by its if-feature,
             * so this if-feature expression is always false */
            return -1;
        } else {
            /* invert result */
            return rc ? 0 : 1;
        }
    case LYS_IFF_AND:
    case LYS_IFF_OR:
        a = resolve_iffeature_recursive(expr, index_e, index_f);
        b = resolve_iffeature_recursive(expr, index_e, index_f);
        if (a == -1 || b == -1) {
            /* one of the referenced feature is hidden by its if-feature,
             * so this if-feature expression is always false */
            return -1;
        } else if (op == LYS_IFF_AND) {
            return a && b;
        } else { /* LYS_IFF_OR */
            return a || b;
        }
    }

    return -1;
}

int
resolve_iffeature(struct lys_iffeature *expr)
{
    int rc = -1;
    int index_e = 0, index_f = 0;

    if (expr->expr) {
        rc = resolve_iffeature_recursive(expr, &index_e, &index_f);
    }
    return (rc == 1) ? 1 : 0;
}

struct iff_stack {
    int size;
    int index;     /* first empty item */
    uint8_t *stack;
};

static int
iff_stack_push(struct iff_stack *stack, uint8_t value)
{
    if (stack->index == stack->size) {
        stack->size += 4;
        stack->stack = ly_realloc(stack->stack, stack->size * sizeof *stack->stack);
        if (!stack->stack) {
            LOGMEM;
            stack->size = 0;
            return EXIT_FAILURE;
        }
    }

    stack->stack[stack->index++] = value;
    return EXIT_SUCCESS;
}

static uint8_t
iff_stack_pop(struct iff_stack *stack)
{
    stack->index--;
    return stack->stack[stack->index];
}

static void
iff_stack_clean(struct iff_stack *stack)
{
    stack->size = 0;
    free(stack->stack);
}

static void
iff_setop(uint8_t *list, uint8_t op, int pos)
{
    uint8_t *item;
    uint8_t mask = 3;

    assert(pos >= 0);
    assert(op <= 3); /* max 2 bits */

    item = &list[pos / 4];
    mask = mask << 2 * (pos % 4);
    *item = (*item) & ~mask;
    *item = (*item) | (op << 2 * (pos % 4));
}

uint8_t
iff_getop(uint8_t *list, int pos)
{
    uint8_t *item;
    uint8_t mask = 3, result;

    assert(pos >= 0);

    item = &list[pos / 4];
    result = (*item) & (mask << 2 * (pos % 4));
    return result >> 2 * (pos % 4);
}

#define LYS_IFF_LP 0x04 /* ( */
#define LYS_IFF_RP 0x08 /* ) */

/* internal structure for passing data for UNRES_IFFEAT */
struct unres_iffeat_data {
    struct lys_node *node;
    const char *fname;
    int infeature;
};

void
resolve_iffeature_getsizes(struct lys_iffeature *iffeat, unsigned int *expr_size, unsigned int *feat_size)
{
    unsigned int e = 0, f = 0, r = 0;
    uint8_t op;

    assert(iffeat);

    if (!iffeat->expr) {
        goto result;
    }

    do {
        op = iff_getop(iffeat->expr, e++);
        switch (op) {
        case LYS_IFF_NOT:
            if (!r) {
                r += 1;
            }
            break;
        case LYS_IFF_AND:
        case LYS_IFF_OR:
            if (!r) {
                r += 2;
            } else {
                r += 1;
            }
            break;
        case LYS_IFF_F:
            f++;
            if (r) {
                r--;
            }
            break;
        }
    } while(r);

result:
    if (expr_size) {
        *expr_size = e;
    }
    if (feat_size) {
        *feat_size = f;
    }
}

int
resolve_iffeature_compile(struct lys_iffeature *iffeat_expr, const char *value, struct lys_node *node,
                          int infeature, struct unres_schema *unres)
{
    const char *c = value;
    int r, rc = EXIT_FAILURE;
    int i, j, last_not, checkversion = 0;
    unsigned int f_size = 0, expr_size = 0, f_exp = 1;
    uint8_t op;
    struct iff_stack stack = {0, 0, NULL};
    struct unres_iffeat_data *iff_data;

    assert(c);

    if (isspace(c[0])) {
        LOGVAL(LYE_INCHAR, LY_VLOG_NONE, NULL, c[0], c);
        return EXIT_FAILURE;
    }

    /* pre-parse the expression to get sizes for arrays, also do some syntax checks of the expression */
    for (i = j = last_not = 0; c[i]; i++) {
        if (c[i] == '(') {
            checkversion = 1;
            j++;
            continue;
        } else if (c[i] == ')') {
            j--;
            continue;
        } else if (isspace(c[i])) {
            continue;
        }

        if (!strncmp(&c[i], "not", r = 3) || !strncmp(&c[i], "and", r = 3) || !strncmp(&c[i], "or", r = 2)) {
            if (c[i + r] == '\0') {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "if-feature");
                return EXIT_FAILURE;
            } else if (!isspace(c[i + r])) {
                /* feature name starting with the not/and/or */
                last_not = 0;
                f_size++;
            } else if (c[i] == 'n') { /* not operation */
                if (last_not) {
                    /* double not */
                    expr_size = expr_size - 2;
                    last_not = 0;
                } else {
                    last_not = 1;
                }
            } else { /* and, or */
                f_exp++;
                /* not a not operation */
                last_not = 0;
            }
            i += r;
        } else {
            f_size++;
            last_not = 0;
        }
        expr_size++;

        while (!isspace(c[i])) {
            if (!c[i] || c[i] == ')') {
                i--;
                break;
            }
            i++;
        }
    }
    if (j || f_exp != f_size) {
        /* not matching count of ( and ) */
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "if-feature");
        return EXIT_FAILURE;
    }

    if (checkversion || expr_size > 1) {
        /* check that we have 1.1 module */
        if (node->module->version != 2) {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "if-feature");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "YANG 1.1 if-feature expression found in 1.0 module.");
            return EXIT_FAILURE;
        }
    }

    /* allocate the memory */
    iffeat_expr->expr = calloc((j = (expr_size / 4) + ((expr_size % 4) ? 1 : 0)), sizeof *iffeat_expr->expr);
    iffeat_expr->features = calloc(f_size, sizeof *iffeat_expr->features);
    stack.size = expr_size;
    stack.stack = malloc(expr_size * sizeof *stack.stack);
    if (!stack.stack || !iffeat_expr->expr || !iffeat_expr->features) {
        LOGMEM;
        goto error;
    }
    f_size--; expr_size--; /* used as indexes from now */

    for (i--; i >= 0; i--) {
        if (c[i] == ')') {
            /* push it on stack */
            iff_stack_push(&stack, LYS_IFF_RP);
            continue;
        } else if (c[i] == '(') {
            /* pop from the stack into result all operators until ) */
            while((op = iff_stack_pop(&stack)) != LYS_IFF_RP) {
                iff_setop(iffeat_expr->expr, op, expr_size--);
            }
            continue;
        } else if (isspace(c[i])) {
            continue;
        }

        /* end operator or operand -> find beginning and get what is it */
        j = i + 1;
        while (i >= 0 && !isspace(c[i]) && c[i] != '(') {
            i--;
        }
        i++; /* get back by one step */

        if (!strncmp(&c[i], "not ", 4)) {
            if (stack.index && stack.stack[stack.index - 1] == LYS_IFF_NOT) {
                /* double not */
                iff_stack_pop(&stack);
            } else {
                /* not has the highest priority, so do not pop from the stack
                 * as in case of AND and OR */
                iff_stack_push(&stack, LYS_IFF_NOT);
            }
        } else if (!strncmp(&c[i], "and ", 4)) {
            /* as for OR - pop from the stack all operators with the same or higher
             * priority and store them to the result, then push the AND to the stack */
            while (stack.index && stack.stack[stack.index - 1] <= LYS_IFF_AND) {
                op = iff_stack_pop(&stack);
                iff_setop(iffeat_expr->expr, op, expr_size--);
            }
            iff_stack_push(&stack, LYS_IFF_AND);
        } else if (!strncmp(&c[i], "or ", 3)) {
            while (stack.index && stack.stack[stack.index - 1] <= LYS_IFF_OR) {
                op = iff_stack_pop(&stack);
                iff_setop(iffeat_expr->expr, op, expr_size--);
            }
            iff_stack_push(&stack, LYS_IFF_OR);
        } else {
            /* feature name, length is j - i */

            /* add it to the result */
            iff_setop(iffeat_expr->expr, LYS_IFF_F, expr_size--);

            /* now get the link to the feature definition. Since it can be
             * forward referenced, we have to keep the feature name in auxiliary
             * structure passed into unres */
            iff_data = malloc(sizeof *iff_data);
            iff_data->node = node;
            iff_data->fname = lydict_insert(node->module->ctx, &c[i], j - i);
            iff_data->infeature = infeature;
            r = unres_schema_add_node(node->module, unres, &iffeat_expr->features[f_size], UNRES_IFFEAT,
                                      (struct lys_node *)iff_data);
            f_size--;

            if (r == -1) {
                free(iff_data);
                goto error;
            }
        }
    }
    while (stack.index) {
        op = iff_stack_pop(&stack);
        iff_setop(iffeat_expr->expr, op, expr_size--);
    }

    if (++expr_size || ++f_size) {
        /* not all expected operators and operands found */
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "if-feature");
        rc = EXIT_FAILURE;
    } else {
        rc = EXIT_SUCCESS;
    }

error:
    /* cleanup */
    iff_stack_clean(&stack);

    return rc;
}

/**
 * @brief Resolve (find) a data node based on a schema-nodeid.
 *
 * Used for resolving unique statements - so id is expected to be relative and local (without reference to a different
 * module).
 *
 */
struct lyd_node *
resolve_data_descendant_schema_nodeid(const char *nodeid, struct lyd_node *start)
{
    char *str, *token, *p;
    struct lyd_node *result = NULL, *iter;
    const struct lys_node *schema = NULL;
    int shorthand = 0;

    assert(nodeid && start);

    if (nodeid[0] == '/') {
        return NULL;
    }

    str = p = strdup(nodeid);
    if (!str) {
        LOGMEM;
        return NULL;
    }

    while (p) {
        token = p;
        p = strchr(p, '/');
        if (p) {
            *p = '\0';
            p++;
        }

        if (p) {
            /* inner node */
            if (resolve_descendant_schema_nodeid(token, schema ? schema->child : start->schema,
                                                 LYS_CONTAINER | LYS_CHOICE | LYS_CASE | LYS_LEAF, 0, 0, &schema)
                    || !schema) {
                result = NULL;
                break;
            }

            if (schema->nodetype & (LYS_CHOICE | LYS_CASE)) {
                continue;
            } else if (lys_parent(schema)->nodetype == LYS_CHOICE) {
                /* shorthand case */
                if (!shorthand) {
                    shorthand = 1;
                    schema = lys_parent(schema);
                    continue;
                } else {
                    shorthand = 0;
                    if (schema->nodetype == LYS_LEAF) {
                        /* should not be here, since we have leaf, which is not a shorthand nor final node */
                        result = NULL;
                        break;
                    }
                }
            }
        } else {
            /* final node */
            if (resolve_descendant_schema_nodeid(token, schema ? schema->child : start->schema, LYS_LEAF,
                                                 shorthand ? 0 : 1, 0, &schema)
                    || !schema) {
                result = NULL;
                break;
            }
        }
        LY_TREE_FOR(result ? result->child : start, iter) {
            if (iter->schema == schema) {
                /* move in data tree according to returned schema */
                result = iter;
                break;
            }
        }
        if (!iter) {
            /* instance not found */
            result = NULL;
            break;
        }
    }
    free(str);

    return result;
}

/*
 *  0 - ok (done)
 *  1 - continue
 *  2 - break
 * -1 - error
 */
static int
schema_nodeid_siblingcheck(const struct lys_node *sibling, int8_t *shorthand, const char *id,
                           const struct lys_module *module, const char *mod_name, int mod_name_len,
                           int implemented_mod, const struct lys_node **start)
{
    const struct lys_module *prefix_mod;

    /* module check */
    prefix_mod = lys_get_import_module(module, NULL, 0, mod_name, mod_name_len);
    if (prefix_mod && implemented_mod) {
        prefix_mod = lys_implemented_module(prefix_mod);
    }
    if (!prefix_mod) {
        return -1;
    }
    if (prefix_mod != lys_node_module(sibling)) {
        return 1;
    }

    /* check for shorthand cases - then 'start' does not change */
    if (lys_parent(sibling) && (lys_parent(sibling)->nodetype == LYS_CHOICE) && (sibling->nodetype != LYS_CASE)) {
        if (*shorthand != -1) {
            *shorthand = *shorthand ? 0 : 1;
        }
    }

    /* the result node? */
    if (!id[0]) {
        if (*shorthand == 1) {
            return 1;
        }
        return 0;
    }

    if (!(*shorthand)) {
        /* move down the tree, if possible */
        if (sibling->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
            return -1;
        }
        *start = sibling->child;
    }

    return 2;
}

/* start - relative, module - absolute, -1 error, EXIT_SUCCESS ok (but ret can still be NULL), >0 unexpected char on ret - 1
 * implement: 0 - do not change the implemented status of the affected modules, 1 - change implemented status of the affected modules
 */
int
resolve_augment_schema_nodeid(const char *nodeid, const struct lys_node *start, const struct lys_module *module,
                              int implement, const struct lys_node **ret)
{
    const char *name, *mod_name, *mod_name_prev, *id;
    const struct lys_node *sibling;
    int r, nam_len, mod_name_len, is_relative = -1;
    int8_t shorthand = 0;
    /* resolved import module from the start module, it must match the next node-name-match sibling */
    const struct lys_module *start_mod, *aux_mod;

    assert(nodeid && (start || module) && !(start && module) && ret);

    id = nodeid;

    if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL)) < 1) {
        return ((id - nodeid) - r) + 1;
    }
    id += r;

    if ((is_relative && !start) || (!is_relative && !module)) {
        return -1;
    }

    /* descendant-schema-nodeid */
    if (is_relative) {
        module = start_mod = start->module;

    /* absolute-schema-nodeid */
    } else {
        start_mod = lys_get_import_module(module, NULL, 0, mod_name, mod_name_len);
        if (start_mod != lys_main_module(module) && start_mod && !start_mod->implemented) {
            /* if the submodule augments the mainmodule (or in general a module augments
             * itself, we don't want to search for the implemented module but augments
             * the module anyway. But when augmenting another module, we need the implemented
             * revision of the module if any */
            aux_mod = lys_implemented_module(start_mod);
            if (!aux_mod->implemented && implement) {
                /* make the found module implemented */
                if (lys_set_implemented(aux_mod)) {
                    return -1;
                }
            }
            start_mod = aux_mod;
            implement++;
        }
        if (!start_mod) {
            return -1;
        }
        start = start_mod->data;
    }

    while (1) {
        sibling = NULL;
        mod_name_prev = mod_name;
        while ((sibling = lys_getnext(sibling, lys_parent(start), start_mod,
                                      LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE | LYS_GETNEXT_WITHINOUT))) {
            /* name match */
            if (sibling->name && !strncmp(name, sibling->name, nam_len) && !sibling->name[nam_len]) {
                r = schema_nodeid_siblingcheck(sibling, &shorthand, id, module, mod_name, mod_name_len,
                                               implement, &start);
                if (r == 0) {
                    *ret = sibling;
                    return EXIT_SUCCESS;
                } else if (r == 1) {
                    continue;
                } else if (r == 2) {
                    break;
                } else {
                    return -1;
                }
            }
        }

        /* no match */
        if (!sibling) {
            *ret = NULL;
            return EXIT_SUCCESS;
        }

        if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL)) < 1) {
            return ((id - nodeid) - r) + 1;
        }
        id += r;

        if ((mod_name && mod_name_prev && strncmp(mod_name, mod_name_prev, mod_name_len + 1)) ||
                (mod_name != mod_name_prev && (!mod_name || !mod_name_prev))) {
            /* we are getting into another module (augment) */
            if (implement) {
                /* we have to check that also target modules are implemented, if not, we have to change it */
                aux_mod = lys_get_import_module(module, NULL, 0, mod_name, mod_name_len);
                if (!aux_mod) {
                    return -1;
                }
                if (!aux_mod->implemented) {
                    aux_mod = lys_implemented_module(aux_mod);
                    if (!aux_mod->implemented) {
                        /* make the found module implemented */
                        if (lys_set_implemented(aux_mod)) {
                            return -1;
                        }
                    }
                }
            } else {
                /* we are not implementing the module itself, so the augments outside the module are ignored */
                *ret = NULL;
                return EXIT_SUCCESS;
            }
        }
    }

    /* cannot get here */
    LOGINT;
    return -1;
}

/* unique, refine,
 * >0  - unexpected char on position (ret - 1),
 *  0  - ok (but ret can still be NULL),
 * -1  - error,
 * -2  - violated no_innerlist  */
int
resolve_descendant_schema_nodeid(const char *nodeid, const struct lys_node *start, int ret_nodetype,
                                 int check_shorthand, int no_innerlist, const struct lys_node **ret)
{
    const char *name, *mod_name, *id;
    const struct lys_node *sibling;
    int r, nam_len, mod_name_len, is_relative = -1;
    int8_t shorthand = check_shorthand ? 0 : -1;
    /* resolved import module from the start module, it must match the next node-name-match sibling */
    const struct lys_module *module;

    assert(nodeid && start && ret);
    assert(!(ret_nodetype & (LYS_USES | LYS_AUGMENT)) && ((ret_nodetype == LYS_GROUPING) || !(ret_nodetype & LYS_GROUPING)));

    id = nodeid;
    module = start->module;

    if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL)) < 1) {
        return ((id - nodeid) - r) + 1;
    }
    id += r;

    if (!is_relative) {
        return -1;
    }

    while (1) {
        sibling = NULL;
        while ((sibling = lys_getnext(sibling, lys_parent(start), module,
                                      LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE))) {
            /* name match */
            if (sibling->name && !strncmp(name, sibling->name, nam_len) && !sibling->name[nam_len]) {
                r = schema_nodeid_siblingcheck(sibling, &shorthand, id, module, mod_name, mod_name_len, 0, &start);
                if (r == 0) {
                    if (!(sibling->nodetype & ret_nodetype)) {
                        /* wrong node type, too bad */
                        continue;
                    }
                    *ret = sibling;
                    return EXIT_SUCCESS;
                } else if (r == 1) {
                    continue;
                } else if (r == 2) {
                    break;
                } else {
                    return -1;
                }
            }
        }

        /* no match */
        if (!sibling) {
            *ret = NULL;
            return EXIT_SUCCESS;
        } else if (no_innerlist && sibling->nodetype == LYS_LIST) {
            *ret = NULL;
            return -2;
        }

        if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL)) < 1) {
            return ((id - nodeid) - r) + 1;
        }
        id += r;
    }

    /* cannot get here */
    LOGINT;
    return -1;
}

/* choice default */
int
resolve_choice_default_schema_nodeid(const char *nodeid, const struct lys_node *start, const struct lys_node **ret)
{
    /* cannot actually be a path */
    if (strchr(nodeid, '/')) {
        return -1;
    }

    return resolve_descendant_schema_nodeid(nodeid, start, LYS_NO_RPC_NOTIF_NODE, 1, 0, ret);
}

/* uses, -1 error, EXIT_SUCCESS ok (but ret can still be NULL), >0 unexpected char on ret - 1 */
static int
resolve_uses_schema_nodeid(const char *nodeid, const struct lys_node *start, const struct lys_node_grp **ret)
{
    const struct lys_module *module;
    const char *mod_prefix, *name;
    int i, mod_prefix_len, nam_len;

    /* parse the identifier, it must be parsed on one call */
    if (((i = parse_node_identifier(nodeid, &mod_prefix, &mod_prefix_len, &name, &nam_len)) < 1) || nodeid[i]) {
        return -i + 1;
    }

    module = lys_get_import_module(start->module, mod_prefix, mod_prefix_len, NULL, 0);
    if (!module) {
        return -1;
    }
    if (module != start->module) {
        start = module->data;
    }

    *ret = lys_find_grouping_up(name, (struct lys_node *)start);

    return EXIT_SUCCESS;
}

int
resolve_absolute_schema_nodeid(const char *nodeid, const struct lys_module *module, int ret_nodetype,
                               const struct lys_node **ret)
{
    const char *name, *mod_name, *id;
    const struct lys_node *sibling, *start;
    int r, nam_len, mod_name_len, is_relative = -1;
    int8_t shorthand = 0;
    const struct lys_module *abs_start_mod;

    assert(nodeid && module && ret);
    assert(!(ret_nodetype & (LYS_USES | LYS_AUGMENT)) && ((ret_nodetype == LYS_GROUPING) || !(ret_nodetype & LYS_GROUPING)));

    id = nodeid;
    start = module->data;

    if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL)) < 1) {
        return ((id - nodeid) - r) + 1;
    }
    id += r;

    if (is_relative) {
        return -1;
    }

    abs_start_mod = lys_get_import_module(module, NULL, 0, mod_name, mod_name_len);
    if (!abs_start_mod) {
        return -1;
    }

    while (1) {
        sibling = NULL;
        while ((sibling = lys_getnext(sibling, lys_parent(start), abs_start_mod, LYS_GETNEXT_WITHCHOICE
                                      | LYS_GETNEXT_WITHCASE | LYS_GETNEXT_WITHINOUT | LYS_GETNEXT_WITHGROUPING))) {
            /* name match */
            if (sibling->name && !strncmp(name, sibling->name, nam_len) && !sibling->name[nam_len]) {
                r = schema_nodeid_siblingcheck(sibling, &shorthand, id, module, mod_name, mod_name_len, 0, &start);
                if (r == 0) {
                    if (!(sibling->nodetype & ret_nodetype)) {
                        /* wrong node type, too bad */
                        continue;
                    }
                    *ret = sibling;
                    return EXIT_SUCCESS;
                } else if (r == 1) {
                    continue;
                } else if (r == 2) {
                    break;
                } else {
                    return -1;
                }
            }
        }

        /* no match */
        if (!sibling) {
            *ret = NULL;
            return EXIT_SUCCESS;
        }

        if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, NULL)) < 1) {
            return ((id - nodeid) - r) + 1;
        }
        id += r;
    }

    /* cannot get here */
    LOGINT;
    return -1;
}

static int
resolve_json_schema_list_predicate(const char *predicate, const struct lys_node_list *list, int *parsed)
{
    const char *name;
    int nam_len, has_predicate, i;

    if (((i = parse_schema_json_predicate(predicate, &name, &nam_len, NULL, NULL, &has_predicate)) < 1)
            || !strncmp(name, ".", nam_len)) {
        LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, predicate[-i], &predicate[-i]);
        return -1;
    }

    predicate += i;
    *parsed += i;

    for (i = 0; i < list->keys_size; ++i) {
        if (!strncmp(list->keys[i]->name, name, nam_len) && !list->keys[i]->name[nam_len]) {
            break;
        }
    }

    if (i == list->keys_size) {
        LOGVAL(LYE_PATH_INKEY, LY_VLOG_NONE, NULL, name);
        return -1;
    }

    /* more predicates? */
    if (has_predicate) {
        return resolve_json_schema_list_predicate(predicate, list, parsed);
    }

    return 0;
}

/* cannot return LYS_GROUPING, LYS_AUGMENT, LYS_USES, logs directly */
const struct lys_node *
resolve_json_nodeid(const char *nodeid, struct ly_ctx *ctx, const struct lys_node *start)
{
    char *module_name = ly_buf(), *buf_backup = NULL, *str;
    const char *name, *mod_name, *id;
    const struct lys_node *sibling;
    int r, nam_len, mod_name_len, is_relative = -1, has_predicate, shorthand = 0;
    /* resolved import module from the start module, it must match the next node-name-match sibling */
    const struct lys_module *prefix_mod, *module, *prev_mod;

    assert(nodeid && (ctx || start));
    if (!ctx) {
        ctx = start->module->ctx;
    }

    id = nodeid;

    if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, &has_predicate)) < 1) {
        LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
        return NULL;
    }
    id += r;

    if (is_relative) {
        assert(start);
        start = start->child;
        if (!start) {
            /* no descendants, fail for sure */
            str = strndup(nodeid, (name + nam_len) - nodeid);
            LOGVAL(LYE_PATH_INNODE, LY_VLOG_STR, str);
            free(str);
            return NULL;
        }
        module = start->module;
    } else {
        if (!mod_name) {
            str = strndup(nodeid, (name + nam_len) - nodeid);
            LOGVAL(LYE_PATH_MISSMOD, LY_VLOG_STR, nodeid);
            free(str);
            return NULL;
        } else if (mod_name_len > LY_BUF_SIZE - 1) {
            LOGINT;
            return NULL;
        }

        if (ly_buf_used && module_name[0]) {
            buf_backup = strndup(module_name, LY_BUF_SIZE - 1);
        }
        ly_buf_used++;

        memmove(module_name, mod_name, mod_name_len);
        module_name[mod_name_len] = '\0';
        module = ly_ctx_get_module(ctx, module_name, NULL);

        if (buf_backup) {
            /* return previous internal buffer content */
            strcpy(module_name, buf_backup);
            free(buf_backup);
            buf_backup = NULL;
        }
        ly_buf_used--;

        if (!module) {
            str = strndup(nodeid, (mod_name + mod_name_len) - nodeid);
            LOGVAL(LYE_PATH_INMOD, LY_VLOG_STR, str);
            free(str);
            return NULL;
        }
        start = module->data;

        /* now it's as if there was no module name */
        mod_name = NULL;
        mod_name_len = 0;
    }

    prev_mod = module;

    while (1) {
        sibling = NULL;
        while ((sibling = lys_getnext(sibling, lys_parent(start), module,
                LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE | LYS_GETNEXT_WITHINOUT))) {
            /* name match */
            if (sibling->name && !strncmp(name, sibling->name, nam_len) && !sibling->name[nam_len]) {
                /* module check */
                if (mod_name) {
                    if (mod_name_len > LY_BUF_SIZE - 1) {
                        LOGINT;
                        return NULL;
                    }

                    if (ly_buf_used && module_name[0]) {
                        buf_backup = strndup(module_name, LY_BUF_SIZE - 1);
                    }
                    ly_buf_used++;

                    memmove(module_name, mod_name, mod_name_len);
                    module_name[mod_name_len] = '\0';
                    /* will also find an augment module */
                    prefix_mod = ly_ctx_get_module(ctx, module_name, NULL);

                    if (buf_backup) {
                        /* return previous internal buffer content */
                        strncpy(module_name, buf_backup, LY_BUF_SIZE - 1);
                        free(buf_backup);
                        buf_backup = NULL;
                    }
                    ly_buf_used--;

                    if (!prefix_mod) {
                        str = strndup(nodeid, (mod_name + mod_name_len) - nodeid);
                        LOGVAL(LYE_PATH_INMOD, LY_VLOG_STR, str);
                        free(str);
                        return NULL;
                    }
                } else {
                    prefix_mod = prev_mod;
                }
                if (prefix_mod != lys_node_module(sibling)) {
                    continue;
                }

                /* do we have some predicates on it? */
                if (has_predicate) {
                    r = 0;
                    if (sibling->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
                        if ((r = parse_schema_json_predicate(id, NULL, NULL, NULL, NULL, &has_predicate)) < 1) {
                            LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
                            return NULL;
                        }
                    } else if (sibling->nodetype == LYS_LIST) {
                        if (resolve_json_schema_list_predicate(id, (const struct lys_node_list *)sibling, &r)) {
                            return NULL;
                        }
                    } else {
                        LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
                        return NULL;
                    }
                    id += r;
                }

                /* check for shorthand cases - then 'start' does not change */
                if (lys_parent(sibling) && (lys_parent(sibling)->nodetype == LYS_CHOICE) && (sibling->nodetype != LYS_CASE)) {
                    shorthand = ~shorthand;
                }

                /* the result node? */
                if (!id[0]) {
                    if (shorthand) {
                        /* wrong path for shorthand */
                        str = strndup(nodeid, (name + nam_len) - nodeid);
                        LOGVAL(LYE_PATH_INNODE, LY_VLOG_STR, str);
                        LOGVAL(LYE_SPEC, LY_VLOG_STR, str, "Schema shorthand case path must include the virtual case statement.");
                        free(str);
                        return NULL;
                    }
                    return sibling;
                }

                if (!shorthand) {
                    /* move down the tree, if possible */
                    if (sibling->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                        LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
                        return NULL;
                    }
                    start = sibling->child;
                }

                /* update prev mod */
                prev_mod = start->module;
                break;
            }
        }

        /* no match */
        if (!sibling) {
            str = strndup(nodeid, (name + nam_len) - nodeid);
            LOGVAL(LYE_PATH_INNODE, LY_VLOG_STR, str);
            free(str);
            return NULL;
        }

        if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, &has_predicate)) < 1) {
            LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
            return NULL;
        }
        id += r;
    }

    /* cannot get here */
    LOGINT;
    return NULL;
}

static int
resolve_partial_json_data_list_predicate(const char *predicate, const char *node_name, struct lyd_node *node, int *parsed)
{
    const char *name, *value, *key_val;
    int nam_len, val_len, has_predicate = 1, r;
    uint16_t i;
    struct lyd_node_leaf_list *key;

    assert(node);
    assert(node->schema->nodetype == LYS_LIST);

    key = (struct lyd_node_leaf_list *)node->child;
    for (i = 0; i < ((struct lys_node_list *)node->schema)->keys_size; ++i) {
        if (!key) {
            /* invalid data */
            LOGINT;
            return -1;
        }

        if (!has_predicate) {
            LOGVAL(LYE_PATH_MISSKEY, LY_VLOG_NONE, NULL, node_name);
            return -1;
        }

        if (((r = parse_schema_json_predicate(predicate, &name, &nam_len, &value, &val_len, &has_predicate)) < 1)
                || !strncmp(name, ".", nam_len)) {
            LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, predicate[-r], &predicate[-r]);
            return -1;
        }

        predicate += r;
        *parsed += r;

        if (strncmp(key->schema->name, name, nam_len) || key->schema->name[nam_len]) {
            LOGVAL(LYE_PATH_INKEY, LY_VLOG_NONE, NULL, name);
            return -1;
        }

        /* make value canonical */
        if ((key->value_type & LY_TYPE_IDENT)
                && !strncmp(key->value_str, lyd_node_module(node)->name, strlen(lyd_node_module(node)->name))
                && (key->value_str[strlen(lyd_node_module(node)->name)] == ':')) {
            key_val = key->value_str + strlen(lyd_node_module(node)->name) + 1;
        } else {
            key_val = key->value_str;
        }

        /* value does not match */
        if (strncmp(key_val, value, val_len) || key_val[val_len]) {
            return 1;
        }

        key = (struct lyd_node_leaf_list *)key->next;
    }

    if (has_predicate) {
        LOGVAL(LYE_PATH_INKEY, LY_VLOG_NONE, NULL, name);
        return -1;
    }

    return 0;
}

/**
 * @brief get the closest parent of the node (or the node itself) identified by the nodeid (path)
 *
 * @param[in] nodeid Node data path to find
 * @param[in] llist_value If the \p nodeid identifies leaf-list, this is expected value of the leaf-list instance.
 * @param[in] options Bitmask of options flags, see @ref pathoptions.
 * @param[out] parsed Number of characters processed in \p id
 * @return The closes parent (or the node itself) from the path
 */
struct lyd_node *
resolve_partial_json_data_nodeid(const char *nodeid, const char *llist_value, struct lyd_node *start, int options,
                                 int *parsed)
{
    char *module_name = ly_buf(), *buf_backup = NULL, *str;
    const char *id, *mod_name, *name, *pred_name, *data_val;
    int r, ret, mod_name_len, nam_len, is_relative = -1;
    int has_predicate, last_parsed, llval_len, pred_name_len, last_has_pred;
    struct lyd_node *sibling, *last_match = NULL;
    struct lyd_node_leaf_list *llist;
    const struct lys_module *prefix_mod, *prev_mod;
    struct ly_ctx *ctx;

    assert(nodeid && start && parsed);

    ctx = start->schema->module->ctx;
    id = nodeid;

    if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, &has_predicate)) < 1) {
        LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
        *parsed = -1;
        return NULL;
    }
    id += r;
    /* add it to parsed only after the data node was actually found */
    last_parsed = r;

    if (is_relative) {
        prev_mod = lyd_node_module(start);
        start = start->child;
    } else {
        for (; start->parent; start = start->parent);
        prev_mod = lyd_node_module(start);
    }

    while (1) {
        LY_TREE_FOR(start, sibling) {
            /* RPC/action data check, return simply invalid argument, because the data tree is invalid */
            if (lys_parent(sibling->schema)) {
                if (options & LYD_PATH_OPT_OUTPUT) {
                    if (lys_parent(sibling->schema)->nodetype == LYS_INPUT) {
                        LOGERR(LY_EINVAL, "Provided data tree includes some RPC input nodes (%s).", sibling->schema->name);
                        *parsed = -1;
                        return NULL;
                    }
                } else {
                    if (lys_parent(sibling->schema)->nodetype == LYS_OUTPUT) {
                        LOGERR(LY_EINVAL, "Provided data tree includes some RPC output nodes (%s).", sibling->schema->name);
                        *parsed = -1;
                        return NULL;
                    }
                }
            }

            /* name match */
            if (!strncmp(name, sibling->schema->name, nam_len) && !sibling->schema->name[nam_len]) {

                /* module check */
                if (mod_name) {
                    if (mod_name_len > LY_BUF_SIZE - 1) {
                        LOGINT;
                        *parsed = -1;
                        return NULL;
                    }

                    if (ly_buf_used && module_name[0]) {
                        buf_backup = strndup(module_name, LY_BUF_SIZE - 1);
                    }
                    ly_buf_used++;

                    memmove(module_name, mod_name, mod_name_len);
                    module_name[mod_name_len] = '\0';
                    /* will also find an augment module */
                    prefix_mod = ly_ctx_get_module(ctx, module_name, NULL);

                    if (buf_backup) {
                        /* return previous internal buffer content */
                        strncpy(module_name, buf_backup, LY_BUF_SIZE - 1);
                        free(buf_backup);
                        buf_backup = NULL;
                    }
                    ly_buf_used--;

                    if (!prefix_mod) {
                        str = strndup(nodeid, (mod_name + mod_name_len) - nodeid);
                        LOGVAL(LYE_PATH_INMOD, LY_VLOG_STR, str);
                        free(str);
                        *parsed = -1;
                        return NULL;
                    }
                } else {
                    prefix_mod = prev_mod;
                }
                if (prefix_mod != lyd_node_module(sibling)) {
                    continue;
                }

                /* leaf-list, did we find it with the correct value or not? */
                if (sibling->schema->nodetype == LYS_LEAFLIST) {
                    llist = (struct lyd_node_leaf_list *)sibling;

                    last_has_pred = 0;
                    if (has_predicate) {
                        if ((r = parse_schema_json_predicate(id, &pred_name, &pred_name_len, &llist_value, &llval_len, &last_has_pred)) < 1) {
                            LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
                            *parsed = -1;
                            return NULL;
                        }
                        if ((pred_name[0] != '.') || (pred_name_len != 1)) {
                            LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[1], id + 1);
                            *parsed = -1;
                            return NULL;
                        }
                    } else {
                        r = 0;
                        if (llist_value) {
                            llval_len = strlen(llist_value);
                        }
                    }

                    /* make value canonical */
                    if ((llist->value_type & LY_TYPE_IDENT)
                            && !strncmp(llist->value_str, lyd_node_module(sibling)->name, strlen(lyd_node_module(sibling)->name))
                            && (llist->value_str[strlen(lyd_node_module(sibling)->name)] == ':')) {
                        data_val = llist->value_str + strlen(lyd_node_module(sibling)->name) + 1;
                    } else {
                        data_val = llist->value_str;
                    }

                    if ((!llist_value && data_val && data_val[0])
                            || (llist_value && (strncmp(llist_value, data_val, llval_len) || data_val[llval_len]))) {
                        continue;
                    }

                    id += r;
                    last_parsed += r;
                    has_predicate = last_has_pred;

                } else if (sibling->schema->nodetype == LYS_LIST) {
                    /* list, we need predicates'n'stuff then */
                    r = 0;
                    if (!has_predicate) {
                        LOGVAL(LYE_PATH_MISSKEY, LY_VLOG_NONE, NULL, name);
                        *parsed = -1;
                        return NULL;
                    }
                    ret = resolve_partial_json_data_list_predicate(id, name, sibling, &r);
                    if (ret == -1) {
                        *parsed = -1;
                        return NULL;
                    } else if (ret == 1) {
                        /* this list instance does not match */
                        continue;
                    }
                    id += r;
                    last_parsed += r;
                }

                *parsed += last_parsed;

                /* the result node? */
                if (!id[0]) {
                    return sibling;
                }

                /* move down the tree, if possible */
                if (sibling->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                    LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[0], id);
                    *parsed = -1;
                    return NULL;
                }
                last_match = sibling;
                prev_mod = lyd_node_module(sibling);
                start = sibling->child;
                break;
            }
        }

        /* no match, return last match */
        if (!sibling) {
            return last_match;
        }

        if ((r = parse_schema_nodeid(id, &mod_name, &mod_name_len, &name, &nam_len, &is_relative, &has_predicate)) < 1) {
            LOGVAL(LYE_PATH_INCHAR, LY_VLOG_NONE, NULL, id[-r], &id[-r]);
            *parsed = -1;
            return NULL;
        }
        id += r;
        last_parsed = r;
    }

    /* cannot get here */
    LOGINT;
    *parsed = -1;
    return NULL;
}

/**
 * @brief Resolves length or range intervals. Does not log.
 * Syntax is assumed to be correct, *ret MUST be NULL.
 *
 * @param[in] str_restr Restriction as a string.
 * @param[in] type Type of the restriction.
 * @param[out] ret Final interval structure that starts with
 * the interval of the initial type, continues with intervals
 * of any superior types derived from the initial one, and
 * finishes with intervals from our \p type.
 *
 * @return EXIT_SUCCESS on succes, -1 on error.
 */
int
resolve_len_ran_interval(const char *str_restr, struct lys_type *type, struct len_ran_intv **ret)
{
    /* 0 - unsigned, 1 - signed, 2 - floating point */
    int kind;
    int64_t local_smin, local_smax, local_fmin, local_fmax;
    uint64_t local_umin, local_umax;
    uint8_t local_fdig;
    const char *seg_ptr, *ptr;
    struct len_ran_intv *local_intv = NULL, *tmp_local_intv = NULL, *tmp_intv, *intv = NULL;

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
        local_fmin = __INT64_C(-9223372036854775807) - __INT64_C(1);
        local_fmax = __INT64_C(9223372036854775807);
        local_fdig = type->info.dec64.dig;

        if (!str_restr && type->info.dec64.range) {
            str_restr = type->info.dec64.range->expr;
        }
        break;
    case LY_TYPE_INT8:
        kind = 1;
        local_smin = __INT64_C(-128);
        local_smax = __INT64_C(127);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_INT16:
        kind = 1;
        local_smin = __INT64_C(-32768);
        local_smax = __INT64_C(32767);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_INT32:
        kind = 1;
        local_smin = __INT64_C(-2147483648);
        local_smax = __INT64_C(2147483647);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_INT64:
        kind = 1;
        local_smin = __INT64_C(-9223372036854775807) - __INT64_C(1);
        local_smax = __INT64_C(9223372036854775807);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT8:
        kind = 0;
        local_umin = __UINT64_C(0);
        local_umax = __UINT64_C(255);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT16:
        kind = 0;
        local_umin = __UINT64_C(0);
        local_umax = __UINT64_C(65535);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT32:
        kind = 0;
        local_umin = __UINT64_C(0);
        local_umax = __UINT64_C(4294967295);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_UINT64:
        kind = 0;
        local_umin = __UINT64_C(0);
        local_umax = __UINT64_C(18446744073709551615);

        if (!str_restr && type->info.num.range) {
            str_restr = type->info.num.range->expr;
        }
        break;
    case LY_TYPE_STRING:
        kind = 0;
        local_umin = __UINT64_C(0);
        local_umax = __UINT64_C(18446744073709551615);

        if (!str_restr && type->info.str.length) {
            str_restr = type->info.str.length->expr;
        }
        break;
    default:
        LOGINT;
        return -1;
    }

    /* process superior types */
    if (type->der) {
        if (resolve_len_ran_interval(NULL, &type->der->type, &intv)) {
            LOGINT;
            return -1;
        }
        assert(!intv || (intv->kind == kind));
    }

    if (!str_restr) {
        /* we do not have any restriction, return superior ones */
        *ret = intv;
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
    tmp_intv = NULL;
    while (1) {
        if (!tmp_local_intv) {
            assert(!local_intv);
            local_intv = malloc(sizeof *local_intv);
            tmp_local_intv = local_intv;
        } else {
            tmp_local_intv->next = malloc(sizeof *tmp_local_intv);
            tmp_local_intv = tmp_local_intv->next;
        }
        if (!tmp_local_intv) {
            LOGMEM;
            goto error;
        }

        tmp_local_intv->kind = kind;
        tmp_local_intv->type = type;
        tmp_local_intv->next = NULL;

        /* min */
        ptr = seg_ptr;
        while (isspace(ptr[0])) {
            ++ptr;
        }
        if (isdigit(ptr[0]) || (ptr[0] == '+') || (ptr[0] == '-')) {
            if (kind == 0) {
                tmp_local_intv->value.uval.min = strtol(ptr, (char **)&ptr, 10);
            } else if (kind == 1) {
                tmp_local_intv->value.sval.min = strtol(ptr, (char **)&ptr, 10);
            } else if (kind == 2) {
                if (parse_range_dec64(&ptr, local_fdig, &tmp_local_intv->value.fval.min)) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, ptr, "range");
                    goto error;
                }
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
            goto error;
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
                    tmp_local_intv->value.uval.max = strtol(ptr, (char **)&ptr, 10);
                } else if (kind == 1) {
                    tmp_local_intv->value.sval.max = strtol(ptr, (char **)&ptr, 10);
                } else if (kind == 2) {
                    if (parse_range_dec64(&ptr, local_fdig, &tmp_local_intv->value.fval.max)) {
                        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, ptr, "range");
                        goto error;
                    }
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
                goto error;
            }
        } else {
            LOGINT;
            goto error;
        }

        /* check min and max in correct order*/
        if (kind == 0) {
            /* current segment */
            if (tmp_local_intv->value.uval.min > tmp_local_intv->value.uval.max) {
                goto error;
            }
            if (tmp_local_intv->value.uval.min < local_umin || tmp_local_intv->value.uval.max > local_umax) {
                goto error;
            }
            /* segments sholud be ascending order */
            if (tmp_intv && (tmp_intv->value.uval.max >= tmp_local_intv->value.uval.min)) {
                goto error;
            }
        } else if (kind == 1) {
            if (tmp_local_intv->value.sval.min > tmp_local_intv->value.sval.max) {
                goto error;
            }
            if (tmp_local_intv->value.sval.min < local_smin || tmp_local_intv->value.sval.max > local_smax) {
                goto error;
            }
            if (tmp_intv && (tmp_intv->value.sval.max >= tmp_local_intv->value.sval.min)) {
                goto error;
            }
        } else if (kind == 2) {
            if (tmp_local_intv->value.fval.min > tmp_local_intv->value.fval.max) {
                goto error;
            }
            if (tmp_local_intv->value.fval.min < local_fmin || tmp_local_intv->value.fval.max > local_fmax) {
                goto error;
            }
            if (tmp_intv && (tmp_intv->value.fval.max >= tmp_local_intv->value.fval.min)) {
                /* fraction-digits value is always the same (it cannot be changed in derived types) */
                goto error;
            }
        }

        /* next segment (next OR) */
        seg_ptr = strchr(seg_ptr, '|');
        if (!seg_ptr) {
            break;
        }
        seg_ptr++;
        tmp_intv = tmp_local_intv;
    }

    /* check local restrictions against superior ones */
    if (intv) {
        tmp_intv = intv;
        tmp_local_intv = local_intv;

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
                        goto error;
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
                        goto error;
                    }
                }
            } else if (kind == 2) {
                local_fmin = tmp_local_intv->value.fval.min;
                local_fmax = tmp_local_intv->value.fval.max;

                 if ((dec64cmp(local_fmin, local_fdig, tmp_intv->value.fval.min, local_fdig) > -1)
                        && (dec64cmp(local_fmin, local_fdig, tmp_intv->value.fval.max, local_fdig) < 1)) {
                    if (dec64cmp(local_fmax, local_fdig, tmp_intv->value.fval.max, local_fdig) < 1) {
                        tmp_local_intv = tmp_local_intv->next;
                        continue;
                    } else {
                        goto error;
                    }
                }
            }

            tmp_intv = tmp_intv->next;
        }

        /* some interval left uncovered -> fail */
        if (tmp_local_intv) {
            goto error;
        }
    }

    /* append the local intervals to all the intervals of the superior types, return it all */
    if (intv) {
        for (tmp_intv = intv; tmp_intv->next; tmp_intv = tmp_intv->next);
        tmp_intv->next = local_intv;
    } else {
        intv = local_intv;
    }
    *ret = intv;

    return EXIT_SUCCESS;

error:
    while (intv) {
        tmp_intv = intv->next;
        free(intv);
        intv = tmp_intv;
    }
    while (local_intv) {
        tmp_local_intv = local_intv->next;
        free(local_intv);
        local_intv = tmp_local_intv;
    }

    return -1;
}

/**
 * @brief Resolve a typedef, return only resolved typedefs if derived. If leafref, it must be
 * resolved for this function to return it. Does not log.
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
resolve_superior_type(const char *name, const char *mod_name, const struct lys_module *module,
                      const struct lys_node *parent, struct lys_tpdf **ret)
{
    int i, j;
    struct lys_tpdf *tpdf, *match;
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
            case LYS_ACTION:
                tpdf_size = ((struct lys_node_rpc_action *)parent)->tpdf_size;
                tpdf = ((struct lys_node_rpc_action *)parent)->tpdf;
                break;

            case LYS_NOTIF:
                tpdf_size = ((struct lys_node_notif *)parent)->tpdf_size;
                tpdf = ((struct lys_node_notif *)parent)->tpdf;
                break;

            case LYS_INPUT:
            case LYS_OUTPUT:
                tpdf_size = ((struct lys_node_inout *)parent)->tpdf_size;
                tpdf = ((struct lys_node_inout *)parent)->tpdf;
                break;

            default:
                parent = lys_parent(parent);
                continue;
            }

            for (i = 0; i < tpdf_size; i++) {
                if (!strcmp(tpdf[i].name, name) && tpdf[i].type.base > 0) {
                    match = &tpdf[i];
                    goto check_leafref;
                }
            }

            parent = lys_parent(parent);
        }
    } else {
        /* get module where to search */
        module = lys_get_import_module(module, NULL, 0, mod_name, 0);
        if (!module) {
            return -1;
        }
    }

    /* search in top level typedefs */
    for (i = 0; i < module->tpdf_size; i++) {
        if (!strcmp(module->tpdf[i].name, name) && module->tpdf[i].type.base > 0) {
            match = &module->tpdf[i];
            goto check_leafref;
        }
    }

    /* search in submodules */
    for (i = 0; i < module->inc_size && module->inc[i].submodule; i++) {
        for (j = 0; j < module->inc[i].submodule->tpdf_size; j++) {
            if (!strcmp(module->inc[i].submodule->tpdf[j].name, name) && module->inc[i].submodule->tpdf[j].type.base > 0) {
                match = &module->inc[i].submodule->tpdf[j];
                goto check_leafref;
            }
        }
    }

    return EXIT_FAILURE;

check_leafref:
    if (ret) {
        *ret = match;
    }
    if (match->type.base == LY_TYPE_LEAFREF) {
        while (!match->type.info.lref.path) {
            match = match->type.der;
            assert(match);
        }
    }
    return EXIT_SUCCESS;
}

/**
 * @brief Check the default \p value of the \p type. Logs directly.
 *
 * @param[in] type Type definition to use.
 * @param[in] value Default value to check.
 * @param[in] module Type module.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
check_default(struct lys_type *type, const char **value, struct lys_module *module)
{
    struct lys_tpdf *base_tpdf = NULL;
    struct lyd_node_leaf_list node;
    const char *dflt = NULL;
    int ret = EXIT_SUCCESS;

    assert(value);

    if (type->base <= LY_TYPE_DER) {
        /* the type was not resolved yet, nothing to do for now */
        return EXIT_FAILURE;
    }

    dflt = *value;
    if (!dflt) {
        /* we do not have a new default value, so is there any to check even, in some base type? */
        for (base_tpdf = type->der; base_tpdf->type.der; base_tpdf = base_tpdf->type.der) {
            if (base_tpdf->dflt) {
                dflt = base_tpdf->dflt;
                break;
            }
        }

        if (!dflt) {
            /* no default value, nothing to check, all is well */
            return EXIT_SUCCESS;
        }

        /* so there is a default value in a base type, but can the default value be no longer valid (did we define some new restrictions)? */
        switch (type->base) {
        case LY_TYPE_IDENT:
        case LY_TYPE_INST:
        case LY_TYPE_LEAFREF:
        case LY_TYPE_BOOL:
        case LY_TYPE_EMPTY:
            /* these have no restrictions, so we would do the exact same work as the unres in the base typedef */
            return EXIT_SUCCESS;
        case LY_TYPE_BITS:
            /* the default value must match the restricted list of values, if the type was restricted */
            if (type->info.bits.count) {
                break;
            }
            return EXIT_SUCCESS;
        case LY_TYPE_ENUM:
            /* the default value must match the restricted list of values, if the type was restricted */
            if (type->info.enums.count) {
                break;
            }
            return EXIT_SUCCESS;
        case LY_TYPE_DEC64:
            if (type->info.dec64.range) {
                break;
            }
            return EXIT_SUCCESS;
        case LY_TYPE_BINARY:
            if (type->info.binary.length) {
                break;
            }
            return EXIT_SUCCESS;
        case LY_TYPE_INT8:
        case LY_TYPE_INT16:
        case LY_TYPE_INT32:
        case LY_TYPE_INT64:
        case LY_TYPE_UINT8:
        case LY_TYPE_UINT16:
        case LY_TYPE_UINT32:
        case LY_TYPE_UINT64:
            if (type->info.num.range) {
                break;
            }
            return EXIT_SUCCESS;
        case LY_TYPE_STRING:
            if (type->info.str.length || type->info.str.patterns) {
                break;
            }
            return EXIT_SUCCESS;
        case LY_TYPE_UNION:
            /* way too much trouble learning whether we need to check the default again, so just do it */
            break;
        default:
            LOGINT;
            return -1;
        }
    } else if (type->base == LY_TYPE_EMPTY) {
        LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, "default", type->parent->name);
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The \"empty\" data type cannot have a default value.");
        return -1;
    }

    /* dummy leaf */
    memset(&node, 0, sizeof node);
    node.value_str = dflt;
    node.value_type = type->base;
    node.schema = calloc(1, sizeof (struct lys_node_leaf));
    if (!node.schema) {
        LOGMEM;
        return -1;
    }
    node.schema->name = strdup("fake-default");
    if (!node.schema->name) {
        LOGMEM;
        free(node.schema);
        return -1;
    }
    node.schema->module = module;
    memcpy(&((struct lys_node_leaf *)node.schema)->type, type, sizeof *type);

    if (type->base == LY_TYPE_LEAFREF) {
        if (!type->info.lref.target) {
            ret = EXIT_FAILURE;
            goto finish;
        }
        ret = check_default(&type->info.lref.target->type, &dflt, module);
        if (!ret) {
            /* adopt possibly changed default value to its canonical form */
            if (*value) {
                *value = dflt;
            }
        }
    } else {
        if (!lyp_parse_value(&((struct lys_node_leaf *)node.schema)->type, &node.value_str, NULL, &node, 1, 1)) {
            /* possible forward reference */
            ret = 1;
            if (base_tpdf) {
                /* default value is defined in some base typedef */
                if ((type->base == LY_TYPE_BITS && type->der->type.der) ||
                        (type->base == LY_TYPE_ENUM && type->der->type.der)) {
                    /* we have refined bits/enums */
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                           "Invalid value \"%s\" of the default statement inherited to \"%s\" from \"%s\" base type.",
                           dflt, type->parent->name, base_tpdf->name);
                }
            }
        } else {
            /* success - adopt canonical form from the node into the default value */
            if (dflt != node.value_str) {
                /* this can happen only if we have non-inherited default value,
                 * inherited default values are already in canonical form */
                assert(dflt == *value);
                *value = node.value_str;
            }
        }
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
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
check_key(struct lys_node_list *list, int index, const char *name, int len)
{
    struct lys_node_leaf *key = list->keys[index];
    char *dup = NULL;
    int j;

    /* existence */
    if (!key) {
        if (name[len] != '\0') {
            dup = strdup(name);
            if (!dup) {
                LOGMEM;
                return -1;
            }
            dup[len] = '\0';
            name = dup;
        }
        LOGVAL(LYE_KEY_MISS, LY_VLOG_LYS, list, name);
        free(dup);
        return -1;
    }

    /* uniqueness */
    for (j = index - 1; j >= 0; j--) {
        if (key == list->keys[j]) {
            LOGVAL(LYE_KEY_DUP, LY_VLOG_LYS, list, key->name);
            return -1;
        }
    }

    /* key is a leaf */
    if (key->nodetype != LYS_LEAF) {
        LOGVAL(LYE_KEY_NLEAF, LY_VLOG_LYS, list, key->name);
        return -1;
    }

    /* type of the leaf is not built-in empty */
    if (key->type.base == LY_TYPE_EMPTY && key->module->version < 2) {
        LOGVAL(LYE_KEY_TYPE, LY_VLOG_LYS, list, key->name);
        return -1;
    }

    /* config attribute is the same as of the list */
    if ((key->flags & LYS_CONFIG_MASK) && (list->flags & LYS_CONFIG_MASK) != (key->flags & LYS_CONFIG_MASK)) {
        LOGVAL(LYE_KEY_CONFIG, LY_VLOG_LYS, list, key->name);
        return -1;
    }

    /* key is not placed from augment */
    if (key->parent->nodetype == LYS_AUGMENT) {
        LOGVAL(LYE_KEY_MISS, LY_VLOG_LYS, key, key->name);
        LOGVAL(LYE_SPEC, LY_VLOG_LYS, key, "Key inserted from augment.");
        return -1;
    }

    /* key is not when/if-feature -conditional */
    j = 0;
    if (key->when || (key->iffeature_size && (j = 1))) {
        LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, key, j ? "if-feature" : "when", "leaf");
        LOGVAL(LYE_SPEC, LY_VLOG_LYS, key, "Key definition cannot depend on a \"%s\" condition.",
               j ? "if-feature" : "when");
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve (test the target exists) unique. Logs directly.
 *
 * @param[in] parent The parent node of the unique structure.
 * @param[in] uniq_str_path One path from the unique string.
 *
 * @return EXIT_SUCCESS on succes, EXIT_FAILURE on forward reference, -1 on error.
 */
int
resolve_unique(struct lys_node *parent, const char *uniq_str_path, uint8_t *trg_type)
{
    int rc;
    const struct lys_node *leaf = NULL;

    rc = resolve_descendant_schema_nodeid(uniq_str_path, parent->child, LYS_LEAF, 1, 1, &leaf);
    if (rc || !leaf) {
        if (rc) {
            LOGVAL(LYE_INARG, LY_VLOG_LYS, parent, uniq_str_path, "unique");
            if (rc > 0) {
                LOGVAL(LYE_INCHAR, LY_VLOG_LYS, parent, uniq_str_path[rc - 1], &uniq_str_path[rc - 1]);
            } else if (rc == -2) {
                LOGVAL(LYE_SPEC, LY_VLOG_LYS, parent, "Unique argument references list.");
            }
            rc = -1;
        } else {
            LOGVAL(LYE_INARG, LY_VLOG_LYS, parent, uniq_str_path, "unique");
            LOGVAL(LYE_SPEC, LY_VLOG_LYS, parent, "Target leaf not found.");
            rc = EXIT_FAILURE;
        }
        goto error;
    }
    if (leaf->nodetype != LYS_LEAF) {
        LOGVAL(LYE_INARG, LY_VLOG_LYS, parent, uniq_str_path, "unique");
        LOGVAL(LYE_SPEC, LY_VLOG_LYS, parent, "Target is not a leaf.");
        return -1;
    }

    /* check status */
    if (lyp_check_status(parent->flags, parent->module, parent->name, leaf->flags, leaf->module, leaf->name, leaf)) {
        return -1;
    }

    /* check that all unique's targets are of the same config type */
    if (*trg_type) {
        if (((*trg_type == 1) && (leaf->flags & LYS_CONFIG_R)) || ((*trg_type == 2) && (leaf->flags & LYS_CONFIG_W))) {
            LOGVAL(LYE_INARG, LY_VLOG_LYS, parent, uniq_str_path, "unique");
            LOGVAL(LYE_SPEC, LY_VLOG_LYS, parent,
                   "Leaf \"%s\" referenced in unique statement is config %s, but previous referenced leaf is config %s.",
                   uniq_str_path, *trg_type == 1 ? "false" : "true", *trg_type == 1 ? "true" : "false");
            return -1;
        }
    } else {
        /* first unique */
        if (leaf->flags & LYS_CONFIG_W) {
            *trg_type = 1;
        } else {
            *trg_type = 2;
        }
    }

    /* set leaf's unique flag */
    ((struct lys_node_leaf *)leaf)->flags |= LYS_UNIQUE;

    return EXIT_SUCCESS;

error:

    return rc;
}

void
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
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_data(const struct lys_module *mod, const char *name, int nam_len, struct lyd_node *start, struct unres_data *parents)
{
    struct lyd_node *node;
    int flag;
    uint32_t i;

    if (!parents->count) {
        parents->count = 1;
        parents->node = malloc(sizeof *parents->node);
        if (!parents->node) {
            LOGMEM;
            return -1;
        }
        parents->node[0] = NULL;
    }
    for (i = 0; i < parents->count;) {
        if (parents->node[i] && (parents->node[i]->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA))) {
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
                    parents->node = ly_realloc(parents->node, parents->count * sizeof *parents->node);
                    if (!parents->node) {
                        return EXIT_FAILURE;
                    }
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
 * @param[in] mod_name Module name of the data node.
 * @param[in] mod_name_len Length of the module name.
 * @param[in] name Name of the data node.
 * @param[in] nam_len Length of the name.
 * @param[in] start Data node to start the search from.
 * @param[in,out] parents Resolved nodes. If there are some parents,
 *                        they are replaced (!!) with the resolvents.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 otherwise.
 */
static int
resolve_data_node(const char *mod_name, int mod_name_len, const char *name, int name_len, struct lyd_node *start,
                    struct unres_data *parents)
{
    const struct lys_module *mod;
    char *str;

    assert(start);

    if (mod_name) {
        /* we have mod_name, find appropriate module */
        str = strndup(mod_name, mod_name_len);
        if (!str) {
            LOGMEM;
            return -1;
        }
        mod = ly_ctx_get_module(start->schema->module->ctx, str, NULL);
        free(str);
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
 * @brief Resolve a path predicate (leafref) in JSON data context. Logs directly
 *        only specific errors, general no-resolvent error is left to the caller.
 *
 * @param[in] pred Predicate to use.
 * @param[in] node Node from which the predicate is being resolved
 * @param[in,out] node_match Nodes satisfying the restriction
 *                           without the predicate. Nodes not
 *                           satisfying the predicate are removed.
 * @param[out] parsed Number of characters parsed, negative on error.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_path_predicate_data(const char *pred, struct lyd_node *node, struct unres_data *node_match,
                            int *parsed)
{
    /* ... /node[source = destination] ... */
    struct unres_data source_match, dest_match;
    const char *path_key_expr, *source, *sour_pref, *dest, *dest_pref;
    int pke_len, sour_len, sour_pref_len, dest_len, dest_pref_len, parsed_loc = 0, pke_parsed = 0;
    int has_predicate, dest_parent_times, i, rc;
    uint32_t j;
    struct lyd_node_leaf_list *leaf_dst, *leaf_src;

    source_match.count = 1;
    source_match.node = malloc(sizeof *source_match.node);
    if (!source_match.node) {
        LOGMEM;
        return -1;
    }
    dest_match.count = 1;
    dest_match.node = malloc(sizeof *dest_match.node);
    if (!dest_match.node) {
        LOGMEM;
        return -1;
    }

    do {
        if ((i = parse_path_predicate(pred, &sour_pref, &sour_pref_len, &source, &sour_len, &path_key_expr,
                                      &pke_len, &has_predicate)) < 1) {
            LOGVAL(LYE_INCHAR, LY_VLOG_LYD, node, pred[-i], &pred[-i]);
            rc = -1;
            goto error;
        }
        parsed_loc += i;
        pred += i;

        for (j = 0; j < node_match->count;) {
            /* source */
            source_match.node[0] = node_match->node[j];

            /* must be leaf (key of a list) */
            if ((rc = resolve_data_node(sour_pref, sour_pref_len, source, sour_len, node_match->node[j],
                    &source_match)) || (source_match.count != 1) || (source_match.node[0]->schema->nodetype != LYS_LEAF)) {
                i = 0;
                goto error;
            }

            /* destination */
            dest_match.node[0] = node;
            dest_parent_times = 0;
            if ((i = parse_path_key_expr(path_key_expr, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                            &dest_parent_times)) < 1) {
                LOGVAL(LYE_INCHAR, LY_VLOG_LYD, node, path_key_expr[-i], &path_key_expr[-i]);
                rc = -1;
                goto error;
            }
            pke_parsed = i;
            for (i = 0; i < dest_parent_times; ++i) {
                dest_match.node[0] = dest_match.node[0]->parent;
                if (!dest_match.node[0]) {
                    i = 0;
                    rc = EXIT_FAILURE;
                    goto error;
                }
            }
            while (1) {
                if ((rc = resolve_data_node(dest_pref, dest_pref_len, dest, dest_len, dest_match.node[0],
                        &dest_match)) || (dest_match.count != 1)) {
                    i = 0;
                    goto error;
                }

                if (pke_len == pke_parsed) {
                    break;
                }
                if ((i = parse_path_key_expr(path_key_expr+pke_parsed, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                             &dest_parent_times)) < 1) {
                    LOGVAL(LYE_INCHAR, LY_VLOG_LYD, node, path_key_expr[-i], &path_key_expr[-i]);
                    rc = -1;
                    goto error;
                }
                pke_parsed += i;
            }

            /* check match between source and destination nodes */
            leaf_dst = (struct lyd_node_leaf_list *)dest_match.node[0];
            while (leaf_dst->value_type == LY_TYPE_LEAFREF) {
                leaf_dst = (struct lyd_node_leaf_list *)leaf_dst->value.leafref;
            }
            leaf_src = (struct lyd_node_leaf_list *)source_match.node[0];
            while (leaf_src->value_type == LY_TYPE_LEAFREF) {
                leaf_src = (struct lyd_node_leaf_list *)leaf_src->value.leafref;
            }
            if (leaf_src->value_type != leaf_dst->value_type) {
                goto remove_leafref;
            }

            if (!ly_strequal(leaf_src->value_str, leaf_dst->value_str, 1)) {
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
 * @brief Resolve a path (leafref) in JSON data context. Logs directly.
 *
 * @param[in] node Leafref data node.
 * @param[in] path Path of the leafref.
 * @param[out] ret Matching nodes. Expects an empty, but allocated structure.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 otherwise.
 */
static int
resolve_path_arg_data(struct lyd_node *node, const char *path, struct unres_data *ret)
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
        if ((i = parse_path_arg(node->schema->module, path, &prefix, &pref_len, &name, &nam_len, &parent_times, &has_predicate)) < 1) {
            LOGVAL(LYE_INCHAR, LY_VLOG_LYD, node, path[-i], &path[-i]);
            rc = -1;
            goto error;
        }
        path += i;
        parsed += i;

        if (!ret->count) {
            if (parent_times > 0) {
                data = node;
                for (i = 1; i < parent_times; ++i) {
                    data = data->parent;
                }
            } else if (!parent_times) {
                data = node->child;
            } else {
                /* absolute path */
                for (data = node; data->parent; data = data->parent);
            }

            /* we may still be parsing it and the pointer is not correct yet */
            if (data->prev) {
                while (data->prev->next) {
                    data = data->prev;
                }
            }
        }

        /* node identifier */
        if ((rc = resolve_data_node(prefix, pref_len, name, nam_len, data, ret))) {
            if (rc == -1) {
                LOGVAL(LYE_INELEM_LEN, LY_VLOG_LYD, node, nam_len, name);
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
            if ((rc = resolve_path_predicate_data(path, node, ret, &i))) {
                if (rc == -1) {
                    LOGVAL(LYE_NORESOLV, LY_VLOG_LYD, node, "leafref", path);
                }
                goto error;
            }
            path += i;
            parsed += i;

            if (!ret->count) {
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

static int
resolve_path_arg_schema_valid_dep_flag(const struct lys_node *op_node, const struct lys_node *first_node, int abs_path)
{
    int dep1, dep2;
    const struct lys_node *node;

    if (lys_parent(op_node)) {
        /* inner operation (notif/action) */
        if (abs_path) {
            return 1;
        } else {
            /* compare depth of both nodes */
            for (dep1 = 0, node = op_node; lys_parent(node); node = lys_parent(node));
            for (dep2 = 0, node = first_node; lys_parent(node); node = lys_parent(node));
            if ((dep2 > dep1) || ((dep2 == dep1) && (op_node != first_node))) {
                return 1;
            }
        }
    } else {
        /* top-level operation (notif/rpc) */
        if (op_node != first_node) {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Resolve a path (leafref) predicate in JSON schema context. Logs directly.
 *
 * @param[in] path Path to use.
 * @param[in] context_node Predicate context node (where the predicate is placed).
 * @param[in] parent Path context node (where the path begins/is placed).
 * @param[in] op_node Optional node if the leafref is in an operation (action/rpc/notif).
 *
 * @return 0 on forward reference, otherwise the number
 *         of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
resolve_path_predicate_schema(const char *path, const struct lys_node *context_node,
                              struct lys_node *parent, const struct lys_node *op_node)
{
    const struct lys_node *src_node, *dst_node;
    const char *path_key_expr, *source, *sour_pref, *dest, *dest_pref;
    int pke_len, sour_len, sour_pref_len, dest_len, dest_pref_len, pke_parsed, parsed = 0;
    int has_predicate, dest_parent_times, i, rc, first_iter;

    do {
        if ((i = parse_path_predicate(path, &sour_pref, &sour_pref_len, &source, &sour_len, &path_key_expr,
                                      &pke_len, &has_predicate)) < 1) {
            LOGVAL(LYE_INCHAR, parent ? LY_VLOG_LYS : LY_VLOG_NONE, parent, path[-i], path-i);
            return -parsed+i;
        }
        parsed += i;
        path += i;

        /* source (must be leaf) */
        if (!sour_pref) {
            sour_pref = context_node->module->name;
        }
        rc = lys_get_sibling(context_node->child, sour_pref, sour_pref_len, source, sour_len,
                             LYS_LEAF | LYS_LEAFLIST | LYS_AUGMENT, &src_node);
        if (rc) {
            LOGVAL(LYE_NORESOLV, parent ? LY_VLOG_LYS : LY_VLOG_NONE, parent, "leafref predicate", path-parsed);
            return 0;
        }

        /* destination */
        dest_parent_times = 0;
        pke_parsed = 0;
        if ((i = parse_path_key_expr(path_key_expr, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                     &dest_parent_times)) < 1) {
            LOGVAL(LYE_INCHAR, parent ? LY_VLOG_LYS : LY_VLOG_NONE, parent, path_key_expr[-i], path_key_expr-i);
            return -parsed;
        }
        pke_parsed += i;

        for (i = 0, dst_node = parent; i < dest_parent_times; ++i) {
            /* path is supposed to be evaluated in data tree, so we have to skip
             * all schema nodes that cannot be instantiated in data tree */
            for (dst_node = lys_parent(dst_node);
                 dst_node && !(dst_node->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_ACTION | LYS_NOTIF | LYS_RPC));
                 dst_node = lys_parent(dst_node));

            if (!dst_node) {
                LOGVAL(LYE_NORESOLV, parent ? LY_VLOG_LYS : LY_VLOG_NONE, parent, "leafref predicate", path_key_expr);
                return 0;
            }
        }
        first_iter = 1;
        while (1) {
            if (!dest_pref) {
                dest_pref = dst_node->module->name;
            }
            rc = lys_get_sibling(dst_node->child, dest_pref, dest_pref_len, dest, dest_len,
                                 LYS_CONTAINER | LYS_LIST | LYS_LEAF | LYS_AUGMENT, &dst_node);
            if (rc) {
                LOGVAL(LYE_NORESOLV, parent ? LY_VLOG_LYS : LY_VLOG_NONE, parent, "leafref predicate", path_key_expr);
                return 0;
            }

            if (first_iter) {
                if (resolve_path_arg_schema_valid_dep_flag(op_node, dst_node, 0)) {
                    parent->flags |= LYS_LEAFREF_DEP;
                }
                first_iter = 0;
            }

            if (pke_len == pke_parsed) {
                break;
            }

            if ((i = parse_path_key_expr(path_key_expr+pke_parsed, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                         &dest_parent_times)) < 1) {
                LOGVAL(LYE_INCHAR, parent ? LY_VLOG_LYS : LY_VLOG_NONE, parent,
                       (path_key_expr+pke_parsed)[-i], (path_key_expr+pke_parsed)-i);
                return -parsed;
            }
            pke_parsed += i;
        }

        /* check source - dest match */
        if (dst_node->nodetype != src_node->nodetype) {
            LOGVAL(LYE_NORESOLV, parent ? LY_VLOG_LYS : LY_VLOG_NONE, parent, "leafref predicate", path-parsed);
            LOGVAL(LYE_SPEC, parent ? LY_VLOG_LYS : LY_VLOG_NONE, parent, "Destination node is not a %s, but a %s.",
                   strnodetype(src_node->nodetype), strnodetype(dst_node->nodetype));
            return -parsed;
        }
    } while (has_predicate);

    return parsed;
}

/**
 * @brief Resolve a path (leafref) in JSON schema context. Logs directly.
 *
 * @param[in] path Path to use.
 * @param[in] parent_node Parent of the leafref.
 * @param[in] parent_tpdf Flag if the parent node is actually typedef, in that case the path
 *            has to contain absolute path
 * @param[out] ret Pointer to the resolved schema node. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_path_arg_schema(const char *path, struct lys_node *parent, int parent_tpdf,
                        const struct lys_node **ret)
{
    const struct lys_node *node, *op_node = NULL;
    const struct lys_module *mod;
    struct lys_module *mod_start;
    const char *id, *prefix, *name;
    int pref_len, nam_len, parent_times, has_predicate;
    int i, first_iter, rc;

    first_iter = 1;
    parent_times = 0;
    id = path;

    /* find operation schema we are in, if applicable */
    if (!parent_tpdf) {
        for (op_node = lys_parent(parent);
             op_node && !(op_node->nodetype & (LYS_ACTION | LYS_NOTIF | LYS_RPC));
             op_node = lys_parent(op_node));
    }

    mod_start = lys_node_module(parent);
    do {
        if ((i = parse_path_arg(mod_start, id, &prefix, &pref_len, &name, &nam_len, &parent_times, &has_predicate)) < 1) {
            LOGVAL(LYE_INCHAR, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent, id[-i], &id[-i]);
            return -1;
        }
        id += i;

        if (first_iter) {
            if (parent_times == -1) {
                /* resolve prefix of the module */
                mod = prefix ? lys_get_import_module(mod_start, NULL, 0, prefix, pref_len) : mod_start;
                if (!mod) {
                    LOGVAL(LYE_NORESOLV, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent,
                           "leafref", path);
                    return EXIT_FAILURE;
                }
                if (!mod->implemented) {
                    mod = lys_implemented_module(mod);
                    if (!mod->implemented) {
                        /* make the found module implemented */
                        if (lys_set_implemented(mod)) {
                            return EXIT_FAILURE;
                        }
                    }
                }
                /* get start node */
                if (!mod->data) {
                    LOGVAL(LYE_NORESOLV, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent,
                           "leafref", path);
                    return EXIT_FAILURE;
                }
                node = mod->data;

            } else if (parent_times > 0) {
                if (parent_tpdf) {
                    /* the path is not allowed to contain relative path since we are in top level typedef */
                    LOGVAL(LYE_NORESOLV, 0, NULL, "leafref", path);
                    return -1;
                }

                /* we are looking for a sibling of a node, node it's parent (that is why parent_times - 1) */
                for (i = 0, node = parent; i < parent_times - 1; i++) {
                    /* path is supposed to be evaluated in data tree, so we have to skip
                     * all schema nodes that cannot be instantiated in data tree */
                    for (node = lys_parent(node);
                         node && !(node->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_ACTION | LYS_NOTIF | LYS_RPC));
                         node = lys_parent(node));

                    if (!node) {
                        LOGVAL(LYE_NORESOLV, LY_VLOG_LYS, parent, "leafref", path);
                        return EXIT_FAILURE;
                    }
                }

                /* now we have to check that if we are going into a node from a different module,
                 * the module is implemented (so its augments are applied) */
                mod = prefix ? lys_get_import_module(mod_start, NULL, 0, prefix, pref_len) : mod_start;
                if (!mod) {
                    LOGVAL(LYE_NORESOLV, LY_VLOG_LYS, parent, "leafref", path);
                    return EXIT_FAILURE;
                }
                if (!mod->implemented) {
                    mod = lys_implemented_module(mod);
                    if (!mod->implemented) {
                        /* make the found module implemented */
                        if (lys_set_implemented(mod)) {
                            return EXIT_FAILURE;
                        }
                    }
                }
            } else {
                LOGINT;
                return -1;
            }
        } else {
            /* we have to first check that the module we are going into is implemented */
            mod = prefix ? lys_get_import_module(mod_start, NULL, 0, prefix, pref_len) : mod_start;
            if (!mod) {
                LOGVAL(LYE_NORESOLV, LY_VLOG_LYS, parent, "leafref", path);
                return EXIT_FAILURE;
            }
            if (!mod->implemented) {
                mod = lys_implemented_module(mod);
                if (!mod->implemented) {
                    /* make the found module implemented */
                    if (lys_set_implemented(mod)) {
                        return EXIT_FAILURE;
                    }
                }
            }

            /* move down the tree, if possible */
            if (node->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                LOGVAL(LYE_INCHAR, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent, name[0], name);
                return -1;
            }
            node = node->child;
            if (!node) {
                LOGVAL(LYE_NORESOLV, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent,
                       "leafref", path);
                return EXIT_FAILURE;
            }
        }

        if (!prefix) {
            prefix = mod_start->name;
        }

        rc = lys_get_sibling(node, prefix, pref_len, name, nam_len, LYS_ANY & ~(LYS_USES | LYS_GROUPING), &node);
        if (rc) {
            LOGVAL(LYE_NORESOLV, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent, "leafref", path);
            return EXIT_FAILURE;
        }

        if (first_iter) {
            /* set external dependency flag, we can decide based on the first found node */
            if (!parent_tpdf && op_node && parent_times &&
                    resolve_path_arg_schema_valid_dep_flag(op_node, node, (parent_times == -1 ? 1 : 0))) {
                parent->flags |= LYS_LEAFREF_DEP;
            }
            first_iter = 0;
        }

        if (has_predicate) {
            /* we have predicate, so the current result must be list */
            if (node->nodetype != LYS_LIST) {
                LOGVAL(LYE_NORESOLV, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent, "leafref", path);
                return -1;
            }

            i = resolve_path_predicate_schema(id, node, parent, op_node);
            if (i <= 0) {
                if (i == 0) {
                    return EXIT_FAILURE;
                } else { /* i < 0 */
                    return -1;
                }
            }
            id += i;
            has_predicate = 0;
        }
    } while (id[0]);

    /* the target must be leaf or leaf-list (in YANG 1.1 only) */
    if ((node->nodetype != LYS_LEAF) && (node->nodetype != LYS_LEAFLIST)) {
        LOGVAL(LYE_NORESOLV, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent, "leafref", path);
        LOGVAL(LYE_SPEC, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent,
               "Leafref target \"%s\" is not a leaf nor a leaf-list.", path);
        return -1;
    }

    /* check status */
    if (lyp_check_status(parent->flags, parent->module, parent->name,
                     node->flags, node->module, node->name, node)) {
        return -1;
    }

    if (ret) {
        *ret = node;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve instance-identifier predicate in JSON data format.
 *        Does not log.
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
resolve_predicate(const char *pred, struct unres_data *node_match)
{
    /* ... /node[target = value] ... */
    struct lyd_node *target;
    const char *model, *name, *value;
    int mod_len, nam_len, val_len, i, has_predicate, cur_idx, idx, parsed, pred_iter, k;
    uint32_t j;

    assert(pred && node_match->count);

    idx = -1;
    parsed = 0;

    pred_iter = -1;
    do {
        if ((i = parse_predicate(pred, &model, &mod_len, &name, &nam_len, &value, &val_len, &has_predicate)) < 1) {
            return -parsed+i;
        }
        parsed += i;
        pred += i;

        if (isdigit(name[0])) {
            /* pos */
            assert(!value);
            idx = atoi(name);
        } else if (name[0] != '.') {
            /* list keys */
            if (pred_iter < 0) {
                pred_iter = 1;
            } else {
                ++pred_iter;
            }
        }

        for (cur_idx = 1, j = 0; j < node_match->count; ++cur_idx) {
            /* target */
            if (name[0] == '.') {
                /* leaf-list value */
                if (node_match->node[j]->schema->nodetype != LYS_LEAFLIST) {
                    goto remove_instid;
                }

                target = node_match->node[j];
                /* check the value */
                if (strncmp(((struct lyd_node_leaf_list *)target)->value_str, value, val_len)
                    || ((struct lyd_node_leaf_list *)target)->value_str[val_len]) {
                    goto remove_instid;
                }

            } else if (!value) {
                /* keyless list position */
                if ((node_match->node[j]->schema->nodetype != LYS_LIST)
                        || ((struct lys_node_list *)node_match->node[j]->schema)->keys) {
                    goto remove_instid;
                }

                if (idx != cur_idx) {
                    goto remove_instid;
                }

            } else {
                /* list key value */
                if (node_match->node[j]->schema->nodetype != LYS_LIST) {
                    goto remove_instid;
                }

                /* key module must match the list module */
                if (strncmp(node_match->node[j]->schema->module->name, model, mod_len)
                        || node_match->node[j]->schema->module->name[mod_len]) {
                    goto remove_instid;
                }
                /* find the key leaf */
                for (k = 1, target = node_match->node[j]->child; target && (k < pred_iter); k++, target = target->next);
                if (!target) {
                    goto remove_instid;
                }
                if ((struct lys_node_leaf *)target->schema !=
                        ((struct lys_node_list *)node_match->node[j]->schema)->keys[pred_iter - 1]) {
                    goto remove_instid;
                }

                /* check the value */
                if (strncmp(((struct lyd_node_leaf_list *)target)->value_str, value, val_len)
                    || ((struct lyd_node_leaf_list *)target)->value_str[val_len]) {
                    goto remove_instid;
                }
            }

            /* instid is ok, continue check with the next one */
            ++j;
            continue;

remove_instid:
            /* does not fulfill conditions, remove instid record */
            unres_data_del(node_match, j);
        }
    } while (has_predicate);

    /* check that all list keys were specified */
    if ((pred_iter > 0) && node_match->count) {
        j = 0;
        while (j < node_match->count) {
            assert(node_match->node[j]->schema->nodetype == LYS_LIST);
            if (pred_iter < ((struct lys_node_list *)node_match->node[j]->schema)->keys_size) {
                /* not enough predicates, just remove the list instance */
                unres_data_del(node_match, j);
            } else {
                ++j;
            }
        }

        if (!node_match->count) {
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Instance identifier is missing some list keys.");
        }
    }

    return parsed;
}

int
lys_check_xpath(struct lys_node *node, int check_place)
{
    struct lys_node *parent, *elem;
    struct lyxp_set set;
    uint32_t i;
    int rc;

    if (check_place) {
        parent = node;
        while (parent) {
            if (parent->nodetype == LYS_GROUPING) {
                /* unresolved grouping, skip for now (will be checked later) */
                return EXIT_SUCCESS;
            }
            if (parent->nodetype == LYS_AUGMENT) {
                if (!((struct lys_node_augment *)parent)->target) {
                    /* unresolved augment */
                    if (parent->module->implemented) {
                        /* skip for now (will be checked later) */
                        return EXIT_FAILURE;
                    } else {
                        /* not implemented augment, skip resolving */
                        return EXIT_SUCCESS;
                    }
                } else {
                    parent = ((struct lys_node_augment *)parent)->target;
                    continue;
                }
            }
            parent = parent->parent;
        }
    }

    rc = lyxp_node_atomize(node, &set);
    if (rc) {
        return rc;
    }

    for (parent = node; parent && !(parent->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)); parent = lys_parent(parent));

    for (i = 0; i < set.used; ++i) {
        /* skip roots'n'stuff */
        if (set.val.snodes[i].type == LYXP_NODE_ELEM) {
            /* XPath expression cannot reference "lower" status than the node that has the definition */
            if (lyp_check_status(node->flags, lys_node_module(node), node->name, set.val.snodes[i].snode->flags,
                    lys_node_module(set.val.snodes[i].snode), set.val.snodes[i].snode->name, node)) {
                return -1;
            }

            if (parent) {
                for (elem = set.val.snodes[i].snode; elem && (elem != parent); elem = lys_parent(elem));
                if (!elem) {
                    /* not in node's RPC or notification subtree, set the flag */
                    node->flags |= LYS_XPATH_DEP;
                    break;
                }
            }
        }
    }

    free(set.val.snodes);
    return EXIT_SUCCESS;
}

static int
check_leafref_config(struct lys_node_leaf *leaf, struct lys_type *type)
{
    int i;

    if (type->base == LY_TYPE_LEAFREF) {
        if ((leaf->flags & LYS_CONFIG_W) && type->info.lref.target && (type->info.lref.target->flags & LYS_CONFIG_R)) {
            LOGVAL(LYE_SPEC, LY_VLOG_LYS, leaf, "The %s is config but refers to a non-config %s.",
                   strnodetype(leaf->nodetype), strnodetype(type->info.lref.target->nodetype));
            return -1;
        }
        /* we can skip the test in case the leafref is not yet resolved. In that case the test is done in the time
         * of leafref resolving (lys_leaf_add_leafref_target()) */
    } else if (type->base == LY_TYPE_UNION) {
        for (i = 0; i < type->info.uni.count; i++) {
            if (check_leafref_config(leaf, &type->info.uni.types[i])) {
                return -1;
            }
        }
    }
    return 0;
}

/**
 * @brief Passes config flag down to children, skips nodes without config flags.
 * Does not log.
 *
 * @param[in] node Siblings and their children to have flags changed.
 * @param[in] clear Flag to clear all config flags if parent is LYS_NOTIF, LYS_INPUT, LYS_OUTPUT, LYS_RPC.
 * @param[in] flags Flags to assign to all the nodes.
 * @param[in,out] unres List of unresolved items.
 *
 * @return 0 on success, -1 on error.
 */
static int
inherit_config_flag(struct lys_node *node, int flags, int clear, struct unres_schema *unres)
{
    struct lys_node_leaf *leaf;

    assert(!(flags ^ (flags & LYS_CONFIG_MASK)));
    LY_TREE_FOR(node, node) {
        if (lys_has_xpath(node) && unres_schema_add_node(node->module, unres, node, UNRES_XPATH, NULL) == -1) {
            return -1;
        }
        if (clear) {
            node->flags &= ~LYS_CONFIG_MASK;
            node->flags &= ~LYS_CONFIG_SET;
        } else {
            if (node->flags & LYS_CONFIG_SET) {
                /* skip nodes with an explicit config value */
                if ((flags & LYS_CONFIG_R) && (node->flags & LYS_CONFIG_W)) {
                    LOGVAL(LYE_INARG, LY_VLOG_LYS, node, "true", "config");
                    LOGVAL(LYE_SPEC, LY_VLOG_LYS, node, "State nodes cannot have configuration nodes as children.");
                    return -1;
                }
                continue;
            }

            if (!(node->nodetype & (LYS_USES | LYS_GROUPING))) {
                node->flags = (node->flags & ~LYS_CONFIG_MASK) | flags;
                /* check that configuration lists have keys */
                if ((node->nodetype == LYS_LIST) && (node->flags & LYS_CONFIG_W)
                        && !((struct lys_node_list *)node)->keys_size) {
                    LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_LYS, node, "key", "list");
                    return -1;
                }
            }
        }
        if (!(node->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA))) {
            if (inherit_config_flag(node->child, flags, clear, unres)) {
                return -1;
            }
        } else if (node->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
            leaf = (struct lys_node_leaf *)node;
            if (check_leafref_config(leaf, &leaf->type)) {
                return -1;
            }
        }
    }

    return 0;
}

/**
 * @brief Resolve augment target. Logs directly.
 *
 * @param[in] aug Augment to use.
 * @param[in] siblings Nodes where to start the search in. If set, uses augment, if not, standalone augment.
 * @param[in,out] unres List of unresolved items.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_augment(struct lys_node_augment *aug, struct lys_node *siblings, struct unres_schema *unres)
{
    int rc, clear_config;
    struct lys_node *sub;
    const struct lys_node *aug_target, *parent;
    struct lys_module *mod;

    assert(aug && !aug->target);
    mod = lys_main_module(aug->module);

    /* resolve target node */
    rc = resolve_augment_schema_nodeid(aug->target_name, siblings, (siblings ? NULL : aug->module), mod->implemented, &aug_target);
    if (rc == -1) {
        return -1;
    } else if (rc > 0) {
        LOGVAL(LYE_INCHAR, LY_VLOG_LYS, aug, aug->target_name[rc - 1], &aug->target_name[rc - 1]);
        return -1;
    } else if (rc == 0 && aug->target) {
        /* augment was resolved as a side effect of setting module implemented when
         * resolving augment schema nodeid, so we are done here */
        return 0;
    }
    if (!aug_target && mod->implemented) {
        LOGVAL(LYE_INRESOLV, LY_VLOG_LYS, aug, "augment", aug->target_name);
        return EXIT_FAILURE;
    }
    /* check that we want to connect augment into its target */
    if (!mod->implemented) {
        /* it must be augment only to the same module,
         * otherwise we do not apply augment in not-implemented
         * module. If the module is set to be implemented in future,
         * the augment is being resolved and checked again */
        if (!aug_target) {
            /* target was not even resolved */
            return EXIT_SUCCESS;
        }
        /* target was resolved, but it may refer another module */
        for (sub = (struct lys_node *)aug_target; sub; sub = lys_parent(sub)) {
            if (lys_node_module(sub) != mod) {
                /* this is not an implemented module and the augment
                 * target some other module, so avoid its connecting
                 * to the target */
                return EXIT_SUCCESS;
            }
        }
    }

    if (!aug->child) {
        /* nothing to do */
        LOGWRN("Augment \"%s\" without children.", aug->target_name);
        goto success;
    }

    /* check for mandatory nodes - if the target node is in another module
     * the added nodes cannot be mandatory
     */
    if (!aug->parent && (lys_node_module((struct lys_node *)aug) != lys_node_module(aug_target))
            && (rc = lyp_check_mandatory_augment(aug, aug_target))) {
        return rc;
    }

    /* check augment target type and then augment nodes type */
    if (aug_target->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_CASE | LYS_INPUT | LYS_OUTPUT | LYS_NOTIF)) {
        LY_TREE_FOR(aug->child, sub) {
            if (!(sub->nodetype & (LYS_ANYDATA | LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_USES | LYS_CHOICE))) {
                LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, aug, strnodetype(sub->nodetype), "augment");
                LOGVAL(LYE_SPEC, LY_VLOG_LYS, aug, "Cannot augment \"%s\" with a \"%s\".",
                       strnodetype(aug_target->nodetype), strnodetype(sub->nodetype));
                return -1;
            }
        }
    } else if (aug_target->nodetype == LYS_CHOICE) {
        LY_TREE_FOR(aug->child, sub) {
            if (!(sub->nodetype & (LYS_CASE | LYS_ANYDATA | LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST))) {
                LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, aug, strnodetype(sub->nodetype), "augment");
                LOGVAL(LYE_SPEC, LY_VLOG_LYS, aug, "Cannot augment \"%s\" with a \"%s\".",
                       strnodetype(aug_target->nodetype), strnodetype(sub->nodetype));
                return -1;
            }
        }
    } else {
        LOGVAL(LYE_INARG, LY_VLOG_LYS, aug, aug->target_name, "target-node");
        LOGVAL(LYE_SPEC, LY_VLOG_LYS, aug, "Invalid augment target node type \"%s\".", strnodetype(aug_target->nodetype));
        return -1;
    }

    /* check identifier uniqueness as in lys_node_addchild() */
    LY_TREE_FOR(aug->child, sub) {
        if (lys_check_id(sub, (struct lys_node *)aug_target, NULL)) {
            return -1;
        }
    }

    /* finally reconnect augmenting data into the target - add them to the target child list,
     * by setting aug->target we know the augment is fully resolved now */
    aug->target = (struct lys_node *)aug_target;
    if (aug->target->child) {
        sub = aug->target->child->prev; /* remember current target's last node */
        sub->next = aug->child;         /* connect augmenting data after target's last node */
        aug->target->child->prev = aug->child->prev; /* new target's last node is last augmenting node */
        aug->child->prev = sub;         /* finish connecting of both child lists */
    } else {
        aug->target->child = aug->child;
    }

    /* inherit config information from actual parent */
    for(parent = aug_target; parent && !(parent->nodetype & (LYS_NOTIF | LYS_INPUT | LYS_OUTPUT | LYS_RPC)); parent = lys_parent(parent));
    clear_config = (parent) ? 1 : 0;
    LY_TREE_FOR(aug->child, sub) {
        if (inherit_config_flag(sub, aug_target->flags & LYS_CONFIG_MASK, clear_config, unres)) {
            return -1;
        }
    }

success:
    if (mod->implemented) {
        /* make target modules also implemented */
        for (sub = aug->target; sub; sub = lys_parent(sub)) {
           if (lys_set_implemented(sub->module)) {
               return -1;
           }
        }
    }

    return EXIT_SUCCESS;
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

        if (ly_strequal(child->name, dflt, 1) && (child->nodetype & (LYS_ANYDATA | LYS_CASE
                | LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_CHOICE))) {
            return child;
        }
    }

    return NULL;
}

/**
 * @brief Resolve uses, apply augments, refines. Logs directly.
 *
 * @param[in] uses Uses to use.
 * @param[in,out] unres List of unresolved items.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
resolve_uses(struct lys_node_uses *uses, struct unres_schema *unres)
{
    struct ly_ctx *ctx = uses->module->ctx; /* shortcut */
    struct lys_node *node = NULL, *next, *iter, **refine_nodes = NULL;
    struct lys_node *node_aux, *parent, *tmp;
    struct lys_node_leaflist *llist;
    struct lys_node_leaf *leaf;
    struct lys_refine *rfn;
    struct lys_restr *must, **old_must;
    struct lys_iffeature *iff, **old_iff;
    int i, j, k, rc;
    uint8_t size, *old_size;
    unsigned int usize, usize1, usize2;

    assert(uses->grp);
    /* HACK just check that the grouping is resolved */
    assert(!uses->grp->nacm);

    if (!uses->grp->child) {
        /* grouping without children, warning was already displayed */
        return EXIT_SUCCESS;
    }

    /* copy the data nodes from grouping into the uses context */
    LY_TREE_FOR(uses->grp->child, node_aux) {
        node = lys_node_dup(uses->module, (struct lys_node *)uses, node_aux, uses->nacm, unres, 0);
        if (!node) {
            LOGVAL(LYE_INARG, LY_VLOG_LYS, uses, uses->grp->name, "uses");
            LOGVAL(LYE_SPEC, LY_VLOG_LYS, uses, "Copying data from grouping failed.");
            goto fail;
        }
        /* test the name of siblings */
        LY_TREE_FOR((uses->parent) ? uses->parent->child : lys_main_module(uses->module)->data, tmp) {
            if (!(tmp->nodetype & (LYS_USES | LYS_GROUPING | LYS_CASE)) && ly_strequal(tmp->name, node_aux->name, 1)) {
                goto fail;
            }
        }
    }

    /* we managed to copy the grouping, the rest must be possible to resolve */

    if (uses->refine_size) {
        refine_nodes = malloc(uses->refine_size * sizeof *refine_nodes);
        if (!refine_nodes) {
            LOGMEM;
            goto fail;
        }
    }

    /* apply refines */
    for (i = 0; i < uses->refine_size; i++) {
        rfn = &uses->refine[i];
        rc = resolve_descendant_schema_nodeid(rfn->target_name, uses->child, LYS_NO_RPC_NOTIF_NODE,
                                              1, 0, (const struct lys_node **)&node);
        if (rc || !node) {
            LOGVAL(LYE_INARG, LY_VLOG_LYS, uses, rfn->target_name, "refine");
            goto fail;
        }

        if (rfn->target_type && !(node->nodetype & rfn->target_type)) {
            LOGVAL(LYE_INARG, LY_VLOG_LYS, uses, rfn->target_name, "refine");
            LOGVAL(LYE_SPEC, LY_VLOG_LYS, uses, "Refine substatements not applicable to the target-node.");
            goto fail;
        }
        refine_nodes[i] = node;

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

        /* config on any nodetype,
         * in case of notification or rpc/action, the config is not applicable (there is no config status) */
        if ((rfn->flags & LYS_CONFIG_MASK) && (node->flags & LYS_CONFIG_MASK)) {
            node->flags &= ~LYS_CONFIG_MASK;
            node->flags |= (rfn->flags & LYS_CONFIG_MASK);
        }

        /* default value ... */
        if (rfn->dflt_size) {
            if (node->nodetype == LYS_LEAF) {
                /* leaf */
                leaf = (struct lys_node_leaf *)node;

                /* replace default value */
                lydict_remove(ctx, leaf->dflt);
                leaf->dflt = lydict_insert(ctx, rfn->dflt[0], 0);

                /* check the default value */
                if (unres_schema_add_node(leaf->module, unres, &leaf->type, UNRES_TYPE_DFLT,
                                          (struct lys_node *)(&leaf->dflt)) == -1) {
                    goto fail;
                }
            } else if (node->nodetype == LYS_LEAFLIST) {
                /* leaf-list */
                llist = (struct lys_node_leaflist *)node;

                /* remove complete set of defaults in target */
                for (i = 0; i < llist->dflt_size; i++) {
                    lydict_remove(ctx, llist->dflt[i]);
                }
                free(llist->dflt);

                /* copy the default set from refine */
                llist->dflt_size = rfn->dflt_size;
                llist->dflt = malloc(llist->dflt_size * sizeof *llist->dflt);
                for (i = 0; i < llist->dflt_size; i++) {
                    llist->dflt[i] = lydict_insert(ctx, rfn->dflt[i], 0);
                }

                /* check default value */
                for (i = 0; i < llist->dflt_size; i++) {
                    if (unres_schema_add_node(llist->module, unres, &llist->type, UNRES_TYPE_DFLT,
                                              (struct lys_node *)(&llist->dflt[i])) == -1) {
                        goto fail;
                    }
                }
            }
        }

        /* mandatory on leaf, anyxml or choice */
        if (rfn->flags & LYS_MAND_MASK) {
            if (node->nodetype & (LYS_LEAF | LYS_ANYDATA | LYS_CHOICE)) {
                /* remove current value */
                node->flags &= ~LYS_MAND_MASK;

                /* set new value */
                node->flags |= (rfn->flags & LYS_MAND_MASK);
            }
            if (rfn->flags & LYS_MAND_TRUE) {
                /* check if node has default value */
                if ((node->nodetype & LYS_LEAF) && ((struct lys_node_leaf *)node)->dflt) {
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The \"mandatory\" statement is forbidden on leaf with \"default\".");
                    goto fail;
                }
                if ((node->nodetype & LYS_CHOICE) && ((struct lys_node_choice *)node)->dflt) {
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "The \"mandatory\" statement is forbidden on choices with \"default\".");
                    goto fail;
                }
            }
        }

        /* presence on container */
        if ((node->nodetype & LYS_CONTAINER) && rfn->mod.presence) {
            lydict_remove(ctx, ((struct lys_node_container *)node)->presence);
            ((struct lys_node_container *)node)->presence = lydict_insert(ctx, rfn->mod.presence, 0);
        }

        /* min/max-elements on list or leaf-list */
        if (node->nodetype == LYS_LIST) {
            if (rfn->flags & LYS_RFN_MINSET) {
                ((struct lys_node_list *)node)->min = rfn->mod.list.min;
            }
            if (rfn->flags & LYS_RFN_MAXSET) {
                ((struct lys_node_list *)node)->max = rfn->mod.list.max;
            }
        } else if (node->nodetype == LYS_LEAFLIST) {
            if (rfn->flags & LYS_RFN_MINSET) {
                ((struct lys_node_leaflist *)node)->min = rfn->mod.list.min;
            }
            if (rfn->flags & LYS_RFN_MAXSET) {
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
            case LYS_ANYDATA:
                old_size = &((struct lys_node_anydata *)node)->must_size;
                old_must = &((struct lys_node_anydata *)node)->must;
                break;
            default:
                LOGINT;
                goto fail;
            }

            size = *old_size + rfn->must_size;
            must = realloc(*old_must, size * sizeof *rfn->must);
            if (!must) {
                LOGMEM;
                goto fail;
            }
            for (k = 0, j = *old_size; k < rfn->must_size; k++, j++) {
                must[j].expr = lydict_insert(ctx, rfn->must[k].expr, 0);
                must[j].dsc = lydict_insert(ctx, rfn->must[k].dsc, 0);
                must[j].ref = lydict_insert(ctx, rfn->must[k].ref, 0);
                must[j].eapptag = lydict_insert(ctx, rfn->must[k].eapptag, 0);
                must[j].emsg = lydict_insert(ctx, rfn->must[k].emsg, 0);
            }

            *old_must = must;
            *old_size = size;

            /* check XPath dependencies again */
            if (unres_schema_add_node(node->module, unres, node, UNRES_XPATH, NULL) == -1) {
                goto fail;
            }
        }

        /* if-feature in leaf, leaf-list, list, container or anyxml */
        if (rfn->iffeature_size) {
            old_size = &node->iffeature_size;
            old_iff = &node->iffeature;

            size = *old_size + rfn->iffeature_size;
            iff = realloc(*old_iff, size * sizeof *rfn->iffeature);
            if (!iff) {
                LOGMEM;
                goto fail;
            }
            for (k = 0, j = *old_size; k < rfn->iffeature_size; k++, j++) {
                resolve_iffeature_getsizes(&rfn->iffeature[k], &usize1, &usize2);
                if (usize1) {
                    /* there is something to duplicate */
                    /* duplicate compiled expression */
                    usize = (usize1 / 4) + (usize1 % 4) ? 1 : 0;
                    iff[j].expr = malloc(usize * sizeof *iff[j].expr);
                    memcpy(iff[j].expr, rfn->iffeature[k].expr, usize * sizeof *iff[j].expr);

                    /* duplicate list of feature pointers */
                    iff[j].features = malloc(usize2 * sizeof *iff[k].features);
                    memcpy(iff[j].features, rfn->iffeature[k].features, usize2 * sizeof *iff[j].features);
                }
            }

            *old_iff = iff;
            *old_size = size;
        }
    }

    /* apply augments */
    for (i = 0; i < uses->augment_size; i++) {
        rc = resolve_augment(&uses->augment[i], uses->child, unres);
        if (rc) {
            goto fail;
        }
    }

    /* check refines */
    for (i = 0; i < uses->refine_size; i++) {
        node = refine_nodes[i];
        rfn = &uses->refine[i];

        /* config on any nodetype */
        if ((rfn->flags & LYS_CONFIG_MASK) && (node->flags & LYS_CONFIG_MASK)) {
            for (parent = lys_parent(node); parent && parent->nodetype == LYS_USES; parent = lys_parent(parent));
            if (parent && parent->nodetype != LYS_GROUPING && (parent->flags & LYS_CONFIG_MASK) &&
                    ((parent->flags & LYS_CONFIG_MASK) != (rfn->flags & LYS_CONFIG_MASK)) &&
                    (rfn->flags & LYS_CONFIG_W)) {
                /* setting config true under config false is prohibited */
                LOGVAL(LYE_INARG, LY_VLOG_LYS, uses, "config", "refine");
                LOGVAL(LYE_SPEC, LY_VLOG_LYS, uses,
                       "changing config from 'false' to 'true' is prohibited while "
                       "the target's parent is still config 'false'.");
                goto fail;
            }

            /* inherit config change to the target children */
            LY_TREE_DFS_BEGIN(node->child, next, iter) {
                if (rfn->flags & LYS_CONFIG_W) {
                    if (iter->flags & LYS_CONFIG_SET) {
                        /* config is set explicitely, go to next sibling */
                        next = NULL;
                        goto nextsibling;
                    }
                } else { /* LYS_CONFIG_R */
                    if ((iter->flags & LYS_CONFIG_SET) && (iter->flags & LYS_CONFIG_W)) {
                        /* error - we would have config data under status data */
                        LOGVAL(LYE_INARG, LY_VLOG_LYS, uses, "config", "refine");
                        LOGVAL(LYE_SPEC, LY_VLOG_LYS, uses,
                               "changing config from 'true' to 'false' is prohibited while the target "
                               "has still a children with explicit config 'true'.");
                        goto fail;
                    }
                }
                /* change config */
                iter->flags &= ~LYS_CONFIG_MASK;
                iter->flags |= (rfn->flags & LYS_CONFIG_MASK);

                /* select next iter - modified LY_TREE_DFS_END */
                if (iter->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                    next = NULL;
                } else {
                    next = iter->child;
                }
nextsibling:
                if (!next) {
                    /* try siblings */
                    next = iter->next;
                }
                while (!next) {
                    /* parent is already processed, go to its sibling */
                    iter = lys_parent(iter);

                    /* no siblings, go back through parents */
                    if (iter == node) {
                        /* we are done, no next element to process */
                        break;
                    }
                    next = iter->next;
                }
            }
        }

        /* default value */
        if (rfn->dflt_size) {
            if (node->nodetype == LYS_CHOICE) {
                /* choice */
                ((struct lys_node_choice *)node)->dflt = resolve_choice_dflt((struct lys_node_choice *)node,
                                                                             rfn->dflt[0]);
                if (!((struct lys_node_choice *)node)->dflt) {
                    LOGVAL(LYE_INARG, LY_VLOG_LYS, uses, rfn->dflt[0], "default");
                    goto fail;
                }
                if (lyp_check_mandatory_choice(node)) {
                    goto fail;
                }
            }
        }

        /* min/max-elements on list or leaf-list */
        if (node->nodetype == LYS_LIST) {
            if (((struct lys_node_list *)node)->min > ((struct lys_node_list *)node)->max) {
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"%s\".", rfn->mod.list.min, "min-elements");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"min-elements\" is bigger than \"max-elements\".");
                goto fail;
            }
        } else if (node->nodetype == LYS_LEAFLIST) {
            if (((struct lys_node_leaflist *)node)->min > ((struct lys_node_leaflist *)node)->max) {
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid value \"%d\" of \"%s\".", rfn->mod.list.min, "min-elements");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "\"min-elements\" is bigger than \"max-elements\".");
                goto fail;
            }
        }

        /* additional checks */
        /* default value with mandatory/min-elements */
        if (node->nodetype == LYS_LEAFLIST) {
            llist = (struct lys_node_leaflist *)node;
            if (llist->dflt_size && llist->min) {
                LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, rfn->dflt_size ? "default" : "min-elements", "refine");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                       "The \"min-elements\" statement with non-zero value is forbidden on leaf-lists with the \"default\" statement.");
                goto fail;
            }
        } else if (node->nodetype == LYS_LEAF) {
            leaf = (struct lys_node_leaf *)node;
            if (leaf->dflt && (leaf->flags & LYS_MAND_TRUE)) {
                LOGVAL(LYE_INCHILDSTMT, LY_VLOG_NONE, NULL, rfn->dflt_size ? "default" : "mandatory", "refine");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                       "The \"mandatory\" statement is forbidden on leafs with the \"default\" statement.");
                goto fail;
            }
        }

        /* check for mandatory node in default case, first find the closest parent choice to the changed node */
        if ((rfn->flags & LYS_MAND_TRUE) || rfn->mod.list.min) {
            for (parent = node->parent;
                 parent && !(parent->nodetype & (LYS_CHOICE | LYS_GROUPING | LYS_ACTION | LYS_USES));
                 parent = parent->parent) {
                if (parent->nodetype == LYS_CONTAINER && ((struct lys_node_container *)parent)->presence) {
                    /* stop also on presence containers */
                    break;
                }
            }
            /* and if it is a choice with the default case, check it for presence of a mandatory node in it */
            if (parent && parent->nodetype == LYS_CHOICE && ((struct lys_node_choice *)parent)->dflt) {
                if (lyp_check_mandatory_choice(parent)) {
                    goto fail;
                }
            }
        }
    }
    free(refine_nodes);

    return EXIT_SUCCESS;

fail:
    LY_TREE_FOR_SAFE(uses->child, next, iter) {
        lys_node_free(iter, NULL, 0);
    }
    free(refine_nodes);
    return -1;
}

static int
identity_backlink_update(struct lys_ident *der, struct lys_ident *base)
{
    int i;

    assert(der && base);

    if (!base->der) {
        /* create a set for backlinks if it does not exist */
        base->der = ly_set_new();
    }
    /* store backlink */
    ly_set_add(base->der, der, LY_SET_OPT_USEASLIST);

    /* do it recursively */
    for (i = 0; i < base->base_size; i++) {
        if (identity_backlink_update(der, base->base[i])) {
            return EXIT_FAILURE;
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
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on crucial error.
 */
static int
resolve_base_ident_sub(const struct lys_module *module, struct lys_ident *ident, const char *basename,
                       struct unres_schema *unres, struct lys_ident **ret)
{
    uint32_t i, j;
    struct lys_ident *base = NULL;

    assert(ret);

    /* search module */
    for (i = 0; i < module->ident_size; i++) {
        if (!strcmp(basename, module->ident[i].name)) {

            if (!ident) {
                /* just search for type, so do not modify anything, just return
                 * the base identity pointer */
                *ret = &module->ident[i];
                return EXIT_SUCCESS;
            }

            base = &module->ident[i];
            goto matchfound;
        }
    }

    /* search submodules */
    for (j = 0; j < module->inc_size && module->inc[j].submodule; j++) {
        for (i = 0; i < module->inc[j].submodule->ident_size; i++) {
            if (!strcmp(basename, module->inc[j].submodule->ident[i].name)) {

                if (!ident) {
                    *ret = &module->inc[j].submodule->ident[i];
                    return EXIT_SUCCESS;
                }

                base = &module->inc[j].submodule->ident[i];
                goto matchfound;
            }
        }
    }

matchfound:
    /* we found it somewhere */
    if (base) {
        /* is it already completely resolved? */
        for (i = 0; i < unres->count; i++) {
            if ((unres->item[i] == base) && (unres->type[i] == UNRES_IDENT)) {
                /* identity found, but not yet resolved, so do not return it in *res and try it again later */

                /* simple check for circular reference,
                 * the complete check is done as a side effect of using only completely
                 * resolved identities (previous check of unres content) */
                if (ly_strequal((const char *)unres->str_snode[i], ident->name, 1)) {
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, basename, "base");
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Circular reference of \"%s\" identity.", basename);
                    return -1;
                }

                return EXIT_FAILURE;
            }
        }

        /* checks done, store the result */
        *ret = base;
        return EXIT_SUCCESS;
    }

    /* base not found (maybe a forward reference) */
    return EXIT_FAILURE;
}

/**
 * @brief Resolve base identity. Logs directly.
 *
 * @param[in] module Main module.
 * @param[in] ident Identity to use.
 * @param[in] basename Base name of the identity.
 * @param[in] parent Either "type" or "identity".
 * @param[in,out] type Type structure where we want to resolve identity. Can be NULL.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_base_ident(const struct lys_module *module, struct lys_ident *ident, const char *basename, const char *parent,
                   struct lys_type *type, struct unres_schema *unres)
{
    const char *name;
    int mod_name_len = 0, rc;
    struct lys_ident *target, **ret;
    uint16_t flags;
    struct lys_module *mod;

    assert((ident && !type) || (!ident && type));

    if (!type) {
        /* have ident to resolve */
        ret = &target;
        flags = ident->flags;
        mod = ident->module;
    } else {
        /* have type to fill */
        ++type->info.ident.count;
        type->info.ident.ref = ly_realloc(type->info.ident.ref, type->info.ident.count * sizeof *type->info.ident.ref);
        if (!type->info.ident.ref) {
            LOGMEM;
            return -1;
        }

        ret = &type->info.ident.ref[type->info.ident.count - 1];
        flags = type->parent->flags;
        mod = type->parent->module;
    }
    *ret = NULL;

    /* search for the base identity */
    name = strchr(basename, ':');
    if (name) {
        /* set name to correct position after colon */
        mod_name_len = name - basename;
        name++;

        if (!strncmp(basename, module->name, mod_name_len) && !module->name[mod_name_len]) {
            /* prefix refers to the current module, ignore it */
            mod_name_len = 0;
        }
    } else {
        name = basename;
    }

    /* get module where to search */
    module = lys_get_import_module(module, NULL, 0, mod_name_len ? basename : NULL, mod_name_len);
    if (!module) {
        /* identity refers unknown data model */
        LOGVAL(LYE_INMOD, LY_VLOG_NONE, NULL, basename);
        return -1;
    }

    /* search in the identified module ... */
    rc = resolve_base_ident_sub(module, ident, name, unres, ret);
    if (!rc) {
        assert(*ret);

        /* check status */
        if (lyp_check_status(flags, mod, ident ? ident->name : "of type",
                             (*ret)->flags, (*ret)->module, (*ret)->name, NULL)) {
            rc = -1;
        } else {
            if (ident) {
                ident->base[ident->base_size++] = *ret;

                /* maintain backlinks to the derived identities */
                rc = identity_backlink_update(ident, *ret) ? -1 : EXIT_SUCCESS;
            }
        }
    } else if (rc == EXIT_FAILURE) {
        LOGVAL(LYE_INRESOLV, LY_VLOG_NONE, NULL, parent, basename);
        if (type) {
            --type->info.ident.count;
        }
    }

    return rc;
}

/**
 * @brief Resolve JSON data format identityref. Logs directly.
 *
 * @param[in] type Identityref type.
 * @param[in] ident_name Identityref name.
 * @param[in] node Node where the identityref is being resolved
 *
 * @return Pointer to the identity resolvent, NULL on error.
 */
struct lys_ident *
resolve_identref(struct lys_type *type, const char *ident_name, struct lyd_node *node)
{
    const char *mod_name, *name, *mod_name_iter;
    int mod_name_len, rc, i;
    unsigned int u;
    struct lys_ident *der, *cur;

    assert(type && ident_name && node);

    if (!type || (!type->info.ident.count && !type->der) || !ident_name) {
        return NULL;
    }

    rc = parse_node_identifier(ident_name, &mod_name, &mod_name_len, &name, NULL);
    if (rc < 1) {
        LOGVAL(LYE_INCHAR, LY_VLOG_LYD, node, ident_name[-rc], &ident_name[-rc]);
        return NULL;
    } else if (rc < (signed)strlen(ident_name)) {
        LOGVAL(LYE_INCHAR, LY_VLOG_LYD, node, ident_name[rc], &ident_name[rc]);
        return NULL;
    }
    if (!mod_name) {
        /* no prefix, identity must be defined in the same module as node */
        mod_name = lys_main_module(node->schema->module)->name;
        mod_name_len = strlen(mod_name);
    }

    /* go through all the bases in all the derived types */
    while (type->der) {
        for (i = 0; i < type->info.ident.count; ++i) {
            cur = type->info.ident.ref[i];
            mod_name_iter = lys_main_module(cur->module)->name;
            if (!strcmp(cur->name, name) &&
                    !strncmp(mod_name_iter, mod_name, mod_name_len) && !mod_name_iter[mod_name_len]) {
                goto match;
            }

            if (cur->der) {
                /* there are also some derived identities */
                for (u = 0; u < cur->der->number; u++) {
                    der = (struct lys_ident *)cur->der->set.g[u]; /* shortcut */
                    mod_name_iter = lys_main_module(der->module)->name;
                    if (!strcmp(der->name, name) &&
                            !strncmp(mod_name_iter, mod_name, mod_name_len) && !mod_name_iter[mod_name_len]) {
                        /* we have match */
                        cur = der;
                        goto match;
                    }
                }
            }
        }
        type = &type->der->type;
    }

    LOGVAL(LYE_INRESOLV, LY_VLOG_LYD, node, "identityref", ident_name);
    return NULL;

match:
    for (i = 0; i < cur->iffeature_size; i++) {
        if (!resolve_iffeature(&cur->iffeature[i])) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, cur->name, node->schema->name);
            LOGVAL(LYE_SPEC, LY_VLOG_LYD, node, "Identity \"%s\" is disabled by its if-feature condition.", cur->name);
            return NULL;
        }
    }
    return cur;
}

/**
 * @brief Resolve unresolved uses. Logs directly.
 *
 * @param[in] uses Uses to use.
 * @param[in] unres Specific unres item.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_unres_schema_uses(struct lys_node_uses *uses, struct unres_schema *unres)
{
    int rc;
    struct lys_node *par_grp;

    /* HACK: when a grouping has uses inside, all such uses have to be resolved before the grouping itself
     *       is used in some uses. When we see such a uses, the grouping's nacm member (not used in grouping)
     *       is used to store number of so far unresolved uses. The grouping cannot be used unless the nacm
     *       value is decreased back to 0. To remember that the uses already increased grouping's nacm, the
     *       LYS_USESGRP flag is used. */
    for (par_grp = lys_parent((struct lys_node *)uses); par_grp && (par_grp->nodetype != LYS_GROUPING); par_grp = lys_parent(par_grp));

    if (!uses->grp) {
        rc = resolve_uses_schema_nodeid(uses->name, (const struct lys_node *)uses, (const struct lys_node_grp **)&uses->grp);
        if (rc == -1) {
            LOGVAL(LYE_INRESOLV, LY_VLOG_LYS, uses, "uses", uses->name);
            return -1;
        } else if (rc > 0) {
            LOGVAL(LYE_INCHAR, LY_VLOG_LYS, uses, uses->name[rc - 1], &uses->name[rc - 1]);
            return -1;
        } else if (!uses->grp) {
            if (par_grp && !(uses->flags & LYS_USESGRP)) {
                /* hack - in contrast to lys_node, lys_node_grp has bigger nacm field
                 * (and smaller flags - it uses only a limited set of flags)
                 */
                ((struct lys_node_grp *)par_grp)->nacm++;
                uses->flags |= LYS_USESGRP;
            }
            LOGVAL(LYE_INRESOLV, LY_VLOG_LYS, uses, "uses", uses->name);
            return EXIT_FAILURE;
        }
    }

    if (uses->grp->nacm) {
        if (par_grp && !(uses->flags & LYS_USESGRP)) {
            ((struct lys_node_grp *)par_grp)->nacm++;
            uses->flags |= LYS_USESGRP;
        } else {
            /* instantiate grouping only when it is completely resolved */
            uses->grp = NULL;
        }
        LOGVAL(LYE_INRESOLV, LY_VLOG_LYS, uses, "uses", uses->name);
        return EXIT_FAILURE;
    }

    rc = resolve_uses(uses, unres);
    if (!rc) {
        /* decrease unres count only if not first try */
        if (par_grp && (uses->flags & LYS_USESGRP)) {
            if (!((struct lys_node_grp *)par_grp)->nacm) {
                LOGINT;
                return -1;
            }
            ((struct lys_node_grp *)par_grp)->nacm--;
            uses->flags &= ~LYS_USESGRP;
        }

        /* check status */
        if (lyp_check_status(uses->flags, uses->module, "of uses",
                         uses->grp->flags, uses->grp->module, uses->grp->name,
                         (struct lys_node *)uses)) {
            return -1;
        }

        return EXIT_SUCCESS;
    }

    return rc;
}

/**
 * @brief Resolve list keys. Logs directly.
 *
 * @param[in] list List to use.
 * @param[in] keys_str Keys node value.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_list_keys(struct lys_node_list *list, const char *keys_str)
{
    int i, len, rc;
    const char *value;

    for (i = 0; i < list->keys_size; ++i) {
        if (!list->child) {
            /* no child, possible forward reference */
            LOGVAL(LYE_INRESOLV, LY_VLOG_LYS, list, "list keys", keys_str);
            return EXIT_FAILURE;
        }
        /* get the key name */
        if ((value = strpbrk(keys_str, " \t\n"))) {
            len = value - keys_str;
            while (isspace(value[0])) {
                value++;
            }
        } else {
            len = strlen(keys_str);
        }

        rc = lys_get_sibling(list->child, lys_main_module(list->module)->name, 0, keys_str, len, LYS_LEAF, (const struct lys_node **)&list->keys[i]);
        if (rc) {
            LOGVAL(LYE_INRESOLV, LY_VLOG_LYS, list, "list keys", keys_str);
            return EXIT_FAILURE;
        }

        if (check_key(list, i, keys_str, len)) {
            /* check_key logs */
            return -1;
        }

        /* check status */
        if (lyp_check_status(list->flags, list->module, list->name,
                             list->keys[i]->flags, list->keys[i]->module, list->keys[i]->name,
                             (struct lys_node *)list->keys[i])) {
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
 * @param[in] inout_parent If set, must in input or output parent of node->schema will be resolved.
 *
 * @return EXIT_SUCCESS on pass, EXIT_FAILURE on fail, -1 on error.
 */
static int
resolve_must(struct lyd_node *node, int inout_parent, int ignore_fail)
{
    uint8_t i, must_size;
    struct lys_node *schema;
    struct lys_restr *must;
    struct lyxp_set set;

    assert(node);
    memset(&set, 0, sizeof set);

    if (inout_parent) {
        for (schema = lys_parent(node->schema);
             schema && (schema->nodetype & (LYS_CHOICE | LYS_CASE | LYS_USES));
             schema = lys_parent(schema));
        if (!schema || !(schema->nodetype & (LYS_INPUT | LYS_OUTPUT))) {
            LOGINT;
            return -1;
        }
        must_size = ((struct lys_node_inout *)schema)->must_size;
        must = ((struct lys_node_inout *)schema)->must;

        /* context node is the RPC/action */
        node = node->parent;
        if (!(node->schema->nodetype & (LYS_RPC | LYS_ACTION))) {
            LOGINT;
            return -1;
        }
    } else {
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
        case LYS_ANYDATA:
            must_size = ((struct lys_node_anydata *)node->schema)->must_size;
            must = ((struct lys_node_anydata *)node->schema)->must;
            break;
        case LYS_NOTIF:
            must_size = ((struct lys_node_notif *)node->schema)->must_size;
            must = ((struct lys_node_notif *)node->schema)->must;
            break;
        default:
            must_size = 0;
            break;
        }
    }

    for (i = 0; i < must_size; ++i) {
        if (lyxp_eval(must[i].expr, node, LYXP_NODE_ELEM, lyd_node_module(node), &set, LYXP_MUST)) {
            return -1;
        }

        lyxp_set_cast(&set, LYXP_SET_BOOLEAN, node, lyd_node_module(node), LYXP_MUST);

        if (!set.val.bool) {
            if (ignore_fail) {
                LOGVRB("Must condition \"%s\" not satisfied, but it is not required.", must[i].expr);
            } else {
                LOGVAL(LYE_NOMUST, LY_VLOG_LYD, node, must[i].expr);
                if (must[i].emsg) {
                    LOGVAL(LYE_SPEC, LY_VLOG_LYD, node, must[i].emsg);
                }
                if (must[i].eapptag) {
                    strncpy(((struct ly_err *)&ly_errno)->apptag, must[i].eapptag, LY_APPTAG_LEN - 1);
                }
                return 1;
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve (find) when condition schema context node. Does not log.
 *
 * @param[in] schema Schema node with the when condition.
 * @param[out] ctx_snode When schema context node.
 * @param[out] ctx_snode_type Schema context node type.
 */
void
resolve_when_ctx_snode(const struct lys_node *schema, struct lys_node **ctx_snode, enum lyxp_node_type *ctx_snode_type)
{
    const struct lys_node *sparent;

    /* find a not schema-only node */
    *ctx_snode_type = LYXP_NODE_ELEM;
    while (schema->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE | LYS_AUGMENT | LYS_INPUT | LYS_OUTPUT)) {
        if (schema->nodetype == LYS_AUGMENT) {
            sparent = ((struct lys_node_augment *)schema)->target;
        } else {
            sparent = schema->parent;
        }
        if (!sparent) {
            /* context node is the document root (fake root in our case) */
            if (schema->flags & LYS_CONFIG_W) {
                *ctx_snode_type = LYXP_NODE_ROOT_CONFIG;
            } else {
                *ctx_snode_type = LYXP_NODE_ROOT;
            }
            /* we need the first top-level sibling, but no uses or groupings */
            schema = lys_getnext(NULL, NULL, lys_node_module(schema), 0);
            break;
        }
        schema = sparent;
    }

    *ctx_snode = (struct lys_node *)schema;
}

/**
 * @brief Resolve (find) when condition context node. Does not log.
 *
 * @param[in] node Data node, whose conditional definition is being decided.
 * @param[in] schema Schema node with the when condition.
 * @param[out] ctx_node Context node.
 * @param[out] ctx_node_type Context node type.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
resolve_when_ctx_node(struct lyd_node *node, struct lys_node *schema, struct lyd_node **ctx_node,
                      enum lyxp_node_type *ctx_node_type)
{
    struct lyd_node *parent;
    struct lys_node *sparent;
    enum lyxp_node_type node_type;
    uint16_t i, data_depth, schema_depth;

    resolve_when_ctx_snode(schema, &schema, &node_type);

    if (node_type == LYXP_NODE_ELEM) {
        /* standard element context node */
        for (parent = node, data_depth = 0; parent; parent = parent->parent, ++data_depth);
        for (sparent = schema, schema_depth = 0;
                sparent;
                sparent = (sparent->nodetype == LYS_AUGMENT ? ((struct lys_node_augment *)sparent)->target : sparent->parent)) {
            if (sparent->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_ANYDATA | LYS_NOTIF | LYS_RPC)) {
                ++schema_depth;
            }
        }
        if (data_depth < schema_depth) {
            return -1;
        }

        /* find the corresponding data node */
        for (i = 0; i < data_depth - schema_depth; ++i) {
            node = node->parent;
        }
        if (node->schema != schema) {
            return -1;
        }
    } else {
        /* root context node */
        while (node->parent) {
            node = node->parent;
        }
        while (node->prev->next) {
            node = node->prev;
        }
    }

    *ctx_node = node;
    *ctx_node_type = node_type;
    return EXIT_SUCCESS;
}

/**
 * @brief Temporarily unlink nodes as per YANG 1.1 RFC section 7.21.5 for when XPath evaluation.
 * The context node is adjusted if needed.
 *
 * @param[in] snode Schema node, whose children instances need to be unlinked.
 * @param[in,out] node Data siblings where to look for the children of \p snode. If it is unlinked,
 * it is moved to point to another sibling still in the original tree.
 * @param[in,out] ctx_node When context node, adjusted if needed.
 * @param[in] ctx_node_type Context node type, just for information to detect invalid situations.
 * @param[out] unlinked_nodes Unlinked siblings. Can be safely appended to \p node afterwards.
 * Ordering may change, but there will be no semantic change.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
resolve_when_unlink_nodes(struct lys_node *snode, struct lyd_node **node, struct lyd_node **ctx_node,
                          enum lyxp_node_type ctx_node_type, struct lyd_node **unlinked_nodes)
{
    struct lyd_node *next, *elem;

    switch (snode->nodetype) {
    case LYS_AUGMENT:
    case LYS_USES:
    case LYS_CHOICE:
    case LYS_CASE:
        LY_TREE_FOR(snode->child, snode) {
            if (resolve_when_unlink_nodes(snode, node, ctx_node, ctx_node_type, unlinked_nodes)) {
                return -1;
            }
        }
        break;
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_LEAF:
    case LYS_LEAFLIST:
    case LYS_ANYXML:
    case LYS_ANYDATA:
        LY_TREE_FOR_SAFE(lyd_first_sibling(*node), next, elem) {
            if (elem->schema == snode) {

                if (elem == *ctx_node) {
                    /* We are going to unlink our context node! This normally cannot happen,
                     * but we use normal top-level data nodes for faking a document root node,
                     * so if this is the context node, we just use the next top-level node.
                     * Additionally, it can even happen that there are no top-level data nodes left,
                     * all were unlinked, so in this case we pass NULL as the context node/data tree,
                     * lyxp_eval() can handle this special situation.
                     */
                    if (ctx_node_type == LYXP_NODE_ELEM) {
                        LOGINT;
                        return -1;
                    }

                    if (elem->prev == elem) {
                        /* unlinking last top-level element, use an empty data tree */
                        *ctx_node = NULL;
                    } else {
                        /* in this case just use the previous/last top-level data node */
                        *ctx_node = elem->prev;
                    }
                } else if (elem == *node) {
                    /* We are going to unlink the currently processed node. This does not matter that
                     * much, but we would lose access to the original data tree, so just move our
                     * pointer somewhere still inside it.
                     */
                    if ((*node)->prev != *node) {
                        *node = (*node)->prev;
                    } else {
                        /* the processed node with sibings were all unlinked, oh well */
                        *node = NULL;
                    }
                }

                /* temporarily unlink the node */
                lyd_unlink(elem);
                if (*unlinked_nodes) {
                    if (lyd_insert_after((*unlinked_nodes)->prev, elem)) {
                        LOGINT;
                        return -1;
                    }
                } else {
                    *unlinked_nodes = elem;
                }

                if (snode->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_ANYDATA)) {
                    /* there can be only one instance */
                    break;
                }
            }
        }
        break;
    default:
        LOGINT;
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Relink the unlinked nodes back.
 *
 * @param[in] node Data node to link the nodes back to. It can actually be the adjusted context node,
 * we simply need a sibling from the original data tree.
 * @param[in] unlinked_nodes Unlinked nodes to relink to \p node.
 * @param[in] ctx_node_type Context node type to distinguish between \p node being the parent
 * or the sibling of \p unlinked_nodes.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
resolve_when_relink_nodes(struct lyd_node *node, struct lyd_node *unlinked_nodes, enum lyxp_node_type ctx_node_type)
{
    struct lyd_node *elem;

    LY_TREE_FOR_SAFE(unlinked_nodes, unlinked_nodes, elem) {
        lyd_unlink(elem);
        if (ctx_node_type == LYXP_NODE_ELEM) {
            if (lyd_insert(node, elem)) {
                return -1;
            }
        } else {
            if (lyd_insert_after(node, elem)) {
                return -1;
            }
        }
    }

    return EXIT_SUCCESS;
}

int
resolve_applies_must(const struct lyd_node *node)
{
    int ret = 0;
    uint8_t must_size;
    struct lys_node *schema, *iter;

    assert(node);

    schema = node->schema;

    /* their own must */
    switch (schema->nodetype) {
    case LYS_CONTAINER:
        must_size = ((struct lys_node_container *)schema)->must_size;
        break;
    case LYS_LEAF:
        must_size = ((struct lys_node_leaf *)schema)->must_size;
        break;
    case LYS_LEAFLIST:
        must_size = ((struct lys_node_leaflist *)schema)->must_size;
        break;
    case LYS_LIST:
        must_size = ((struct lys_node_list *)schema)->must_size;
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        must_size = ((struct lys_node_anydata *)schema)->must_size;
        break;
    case LYS_NOTIF:
        must_size = ((struct lys_node_notif *)schema)->must_size;
        break;
    default:
        must_size = 0;
        break;
    }

    if (must_size) {
        ++ret;
    }

    /* schema may be a direct data child of input/output with must (but it must be first, it needs to be evaluated only once) */
    if (!node->prev->next) {
        for (iter = lys_parent(schema); iter && (iter->nodetype & (LYS_CHOICE | LYS_CASE | LYS_USES)); iter = lys_parent(iter));
        if (iter && (iter->nodetype & (LYS_INPUT | LYS_OUTPUT))) {
            ret += 0x2;
        }
    }

    return ret;
}

int
resolve_applies_when(const struct lys_node *schema, int mode, const struct lys_node *stop)
{
    const struct lys_node *parent;

    assert(schema);

    if (!(schema->nodetype & (LYS_NOTIF | LYS_RPC)) && (((struct lys_node_container *)schema)->when)) {
        return 1;
    }

    parent = schema;
    goto check_augment;

    while (parent) {
        /* stop conditions */
        if (!mode) {
            /* stop on node that can be instantiated in data tree */
            if (!(parent->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE))) {
                break;
            }
        } else {
            /* stop on the specified node */
            if (parent == stop) {
                break;
            }
        }

        if (((const struct lys_node_uses *)parent)->when) {
            return 1;
        }
check_augment:

        if ((parent->parent && (parent->parent->nodetype == LYS_AUGMENT) &&
                (((const struct lys_node_augment *)parent->parent)->when))) {
            return 1;
        }
        parent = lys_parent(parent);
    }

    return 0;
}

/**
 * @brief Resolve (check) all when conditions relevant for \p node.
 * Logs directly.
 *
 * @param[in] node Data node, whose conditional reference, if such, is being decided.
 *
 * @return
 *  -1 - error, ly_errno is set
 *   0 - true "when" statement
 *   0, ly_vecode = LYVE_NOWHEN - false "when" statement
 *   1, ly_vecode = LYVE_INWHEN - nodes needed to resolve are conditional and not yet resolved (under another "when")
 */
int
resolve_when(struct lyd_node *node, int *result, int ignore_fail)
{
    struct lyd_node *ctx_node = NULL, *unlinked_nodes, *tmp_node;
    struct lys_node *sparent;
    struct lyxp_set set;
    enum lyxp_node_type ctx_node_type;
    int rc = 0;

    assert(node);
    memset(&set, 0, sizeof set);

    if (!(node->schema->nodetype & (LYS_NOTIF | LYS_RPC)) && (((struct lys_node_container *)node->schema)->when)) {
        /* make the node dummy for the evaluation */
        node->validity |= LYD_VAL_INUSE;
        rc = lyxp_eval(((struct lys_node_container *)node->schema)->when->cond, node, LYXP_NODE_ELEM, lyd_node_module(node),
                       &set, LYXP_WHEN);
        node->validity &= ~LYD_VAL_INUSE;
        if (rc) {
            if (rc == 1) {
                LOGVAL(LYE_INWHEN, LY_VLOG_LYD, node, ((struct lys_node_container *)node->schema)->when->cond);
            }
            goto cleanup;
        }

        /* set boolean result of the condition */
        lyxp_set_cast(&set, LYXP_SET_BOOLEAN, node, lyd_node_module(node), LYXP_WHEN);
        if (!set.val.bool) {
            node->when_status |= LYD_WHEN_FALSE;
            if (ignore_fail) {
                LOGVRB("When condition \"%s\" is not satisfied, but it is not required.",
                       ((struct lys_node_container *)node->schema)->when->cond);
            } else {
                LOGVAL(LYE_NOWHEN, LY_VLOG_LYD, node, ((struct lys_node_container *)node->schema)->when->cond);
                goto cleanup;
            }
        }

        /* free xpath set content */
        lyxp_set_cast(&set, LYXP_SET_EMPTY, node, lyd_node_module(node), 0);
    }

    sparent = node->schema;
    goto check_augment;

    /* check when in every schema node that affects node */
    while (sparent && (sparent->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE))) {
        if (((struct lys_node_uses *)sparent)->when) {
            if (!ctx_node) {
                rc = resolve_when_ctx_node(node, sparent, &ctx_node, &ctx_node_type);
                if (rc) {
                    LOGINT;
                    goto cleanup;
                }
            }

            unlinked_nodes = NULL;
            /* we do not want our node pointer to change */
            tmp_node = node;
            rc = resolve_when_unlink_nodes(sparent, &tmp_node, &ctx_node, ctx_node_type, &unlinked_nodes);
            if (rc) {
                goto cleanup;
            }

            rc = lyxp_eval(((struct lys_node_uses *)sparent)->when->cond, ctx_node, ctx_node_type, lys_node_module(sparent),
                           &set, LYXP_WHEN);

            if (unlinked_nodes && ctx_node) {
                if (resolve_when_relink_nodes(ctx_node, unlinked_nodes, ctx_node_type)) {
                    rc = -1;
                    goto cleanup;
                }
            }

            if (rc) {
                if (rc == 1) {
                    LOGVAL(LYE_INWHEN, LY_VLOG_LYD, node, ((struct lys_node_uses *)sparent)->when->cond);
                }
                goto cleanup;
            }

            lyxp_set_cast(&set, LYXP_SET_BOOLEAN, ctx_node, lys_node_module(sparent), LYXP_WHEN);
            if (!set.val.bool) {
                node->when_status |= LYD_WHEN_FALSE;
                if (ignore_fail) {
                    LOGVRB("When condition \"%s\" is not satisfied, but it is not required.",
                        ((struct lys_node_uses *)sparent)->when->cond);
                } else {
                    LOGVAL(LYE_NOWHEN, LY_VLOG_LYD, node, ((struct lys_node_uses *)sparent)->when->cond);
                    goto cleanup;
                }
            }

            /* free xpath set content */
            lyxp_set_cast(&set, LYXP_SET_EMPTY, ctx_node, lys_node_module(sparent), 0);
        }

check_augment:
        if ((sparent->parent && (sparent->parent->nodetype == LYS_AUGMENT) && (((struct lys_node_augment *)sparent->parent)->when))) {
            if (!ctx_node) {
                rc = resolve_when_ctx_node(node, sparent->parent, &ctx_node, &ctx_node_type);
                if (rc) {
                    LOGINT;
                    goto cleanup;
                }
            }

            unlinked_nodes = NULL;
            tmp_node = node;
            rc = resolve_when_unlink_nodes(sparent->parent, &tmp_node, &ctx_node, ctx_node_type, &unlinked_nodes);
            if (rc) {
                goto cleanup;
            }

            rc = lyxp_eval(((struct lys_node_augment *)sparent->parent)->when->cond, ctx_node, ctx_node_type,
                           lys_node_module(sparent->parent), &set, LYXP_WHEN);

            /* reconnect nodes, if ctx_node is NULL then all the nodes were unlinked, but linked together,
             * so the tree did not actually change and there is nothing for us to do
             */
            if (unlinked_nodes && ctx_node) {
                if (resolve_when_relink_nodes(ctx_node, unlinked_nodes, ctx_node_type)) {
                    rc = -1;
                    goto cleanup;
                }
            }

            if (rc) {
                if (rc == 1) {
                    LOGVAL(LYE_INWHEN, LY_VLOG_LYD, node, ((struct lys_node_augment *)sparent->parent)->when->cond);
                }
                goto cleanup;
            }

            lyxp_set_cast(&set, LYXP_SET_BOOLEAN, ctx_node, lys_node_module(sparent->parent), LYXP_WHEN);

            if (!set.val.bool) {
                node->when_status |= LYD_WHEN_FALSE;
                if (ignore_fail) {
                    LOGVRB("When condition \"%s\" is not satisfied, but it is not required.",
                        ((struct lys_node_augment *)sparent->parent)->when->cond);
                } else {
                    LOGVAL(LYE_NOWHEN, LY_VLOG_LYD, node, ((struct lys_node_augment *)sparent->parent)->when->cond);
                    goto cleanup;
                }
            }

            /* free xpath set content */
            lyxp_set_cast(&set, LYXP_SET_EMPTY, ctx_node, lys_node_module(sparent->parent), 0);
        }

        sparent = lys_parent(sparent);
    }

    node->when_status |= LYD_WHEN_TRUE;

cleanup:
    /* free xpath set content */
    lyxp_set_cast(&set, LYXP_SET_EMPTY, ctx_node ? ctx_node : node, NULL, 0);

    if (result) {
        if (node->when_status & LYD_WHEN_TRUE) {
            *result = 1;
        } else {
            *result = 0;
        }
    }

    return rc;
}

static int
check_leafref_features(struct lys_type *type)
{
    struct lys_node *iter;
    struct ly_set *src_parents, *trg_parents, *features;
    unsigned int i, j, size, x;
    int ret = EXIT_SUCCESS;

    assert(type->parent);

    src_parents = ly_set_new();
    trg_parents = ly_set_new();
    features = ly_set_new();

    /* get parents chain of source (leafref) */
    for (iter = (struct lys_node *)type->parent; iter; iter = iter->parent) {
        if (iter->nodetype & (LYS_INPUT | LYS_OUTPUT)) {
            continue;
        }
        ly_set_add(src_parents, iter, LY_SET_OPT_USEASLIST);
    }
    /* get parents chain of target */
    for (iter = (struct lys_node *)type->info.lref.target; iter; iter = iter->parent) {
        if (iter->nodetype & (LYS_INPUT | LYS_OUTPUT)) {
            continue;
        }
        ly_set_add(trg_parents, iter, LY_SET_OPT_USEASLIST);
    }

    /* compare the features used in if-feature statements in the rest of both
     * chains of parents. The set of features used for target must be a subset
     * of features used for the leafref. This is not a perfect, we should compare
     * the truth tables but it could require too much resources, so we simplify that */
    for (i = 0; i < src_parents->number; i++) {
        iter = src_parents->set.s[i]; /* shortcut */
        if (!iter->iffeature_size) {
            continue;
        }
        for (j = 0; j < iter->iffeature_size; j++) {
            resolve_iffeature_getsizes(&iter->iffeature[j], NULL, &size);
            for (; size; size--) {
                if (!iter->iffeature[j].features[size - 1]) {
                    /* not yet resolved feature, postpone this check */
                    ret = EXIT_FAILURE;
                    goto cleanup;
                }
                ly_set_add(features, iter->iffeature[j].features[size - 1], 0);
            }
        }
    }
    x = features->number;
    for (i = 0; i < trg_parents->number; i++) {
        iter = trg_parents->set.s[i]; /* shortcut */
        if (!iter->iffeature_size) {
            continue;
        }
        for (j = 0; j < iter->iffeature_size; j++) {
            resolve_iffeature_getsizes(&iter->iffeature[j], NULL, &size);
            for (; size; size--) {
                if (!iter->iffeature[j].features[size - 1]) {
                    /* not yet resolved feature, postpone this check */
                    ret = EXIT_FAILURE;
                    goto cleanup;
                }
                if ((unsigned int)ly_set_add(features, iter->iffeature[j].features[size - 1], 0) >= x) {
                    /* the feature is not present in features set of target's parents chain */
                    LOGVAL(LYE_NORESOLV, LY_VLOG_LYS, type->parent, "leafref", type->info.lref.path);
                    LOGVAL(LYE_SPEC, LY_VLOG_LYS, type->parent,
                           "Leafref is not conditional based on \"%s\" feature as its target.",
                           iter->iffeature[j].features[size - 1]->name);
                    ret = -1;
                    goto cleanup;
                }
            }
        }
    }

cleanup:
    ly_set_free(features);
    ly_set_free(src_parents);
    ly_set_free(trg_parents);

    return ret;
}

/**
 * @brief Resolve a single unres schema item. Logs indirectly.
 *
 * @param[in] mod Main module.
 * @param[in] item Item to resolve. Type determined by \p type.
 * @param[in] type Type of the unresolved item.
 * @param[in] str_snode String, a schema node, or NULL.
 * @param[in] unres Unres schema structure to use.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_unres_schema_item(struct lys_module *mod, void *item, enum UNRES_ITEM type, void *str_snode,
                          struct unres_schema *unres)
{
    /* has_str - whether the str_snode is a string in a dictionary that needs to be freed */
    int rc = -1, has_str = 0, tpdf_flag = 0, i, k;
    unsigned int j;
    struct lys_node *node, *par_grp;
    const char *expr;

    struct ly_set *refs, *procs;
    struct lys_feature *ref, *feat;
    struct lys_ident *ident;
    struct lys_type *stype;
    struct lys_node_choice *choic;
    struct lyxml_elem *yin;
    struct yang_type *yang;
    struct unres_list_uniq *unique_info;
    struct unres_iffeat_data *iff_data;

    switch (type) {
    case UNRES_IDENT:
        expr = str_snode;
        has_str = 1;
        ident = item;

        rc = resolve_base_ident(mod, ident, expr, "identity", NULL, unres);
        break;
    case UNRES_TYPE_IDENTREF:
        expr = str_snode;
        has_str = 1;
        stype = item;

        rc = resolve_base_ident(mod, NULL, expr, "type", stype, unres);
        break;
    case UNRES_TYPE_LEAFREF:
        node = str_snode;
        stype = item;

        /* HACK - when there is no parent, we are in top level typedef and in that
         * case, the path has to contain absolute path, so we let the resolve_path_arg_schema()
         * know it via tpdf_flag */
        if (!node) {
            tpdf_flag = 1;
            node = (struct lys_node *)stype->parent;
        }

        if (!lys_node_module(node)->implemented) {
            /* not implemented module, don't bother with resolving the leafref
             * if the module is set to be implemented, the path will be resolved then */
            rc = 0;
            break;
        }
        rc = resolve_path_arg_schema(stype->info.lref.path, node, tpdf_flag,
                                     (const struct lys_node **)&stype->info.lref.target);
        if (!tpdf_flag && !rc) {
            assert(stype->info.lref.target);
            /* check if leafref and its target are under a common if-features */
            rc = check_leafref_features(stype);
            if (rc) {
                break;
            }

            /* store the backlink from leafref target */
            if (lys_leaf_add_leafref_target(stype->info.lref.target, (struct lys_node *)stype->parent)) {
                rc = -1;
            }
        }

        break;
    case UNRES_TYPE_DER_TPDF:
        tpdf_flag = 1;
        /* no break */
    case UNRES_TYPE_DER:
        /* parent */
        node = str_snode;
        stype = item;

        /* HACK type->der is temporarily unparsed type statement */
        yin = (struct lyxml_elem *)stype->der;
        stype->der = NULL;

        if (yin->flags & LY_YANG_STRUCTURE_FLAG) {
            yang = (struct yang_type *)yin;
            rc = yang_check_type(mod, node, yang, tpdf_flag, unres);

            if (rc) {
                /* may try again later */
                stype->der = (struct lys_tpdf *)yang;
            } else {
                /* we need to always be able to free this, it's safe only in this case */
                lydict_remove(mod->ctx, yang->name);
                free(yang);
            }

        } else {
            rc = fill_yin_type(mod, node, yin, stype, tpdf_flag, unres);
            if (!rc) {
                /* we need to always be able to free this, it's safe only in this case */
                lyxml_free(mod->ctx, yin);
            } else {
                /* may try again later, put all back how it was */
                stype->der = (struct lys_tpdf *)yin;
            }
        }
        if (rc == EXIT_SUCCESS) {
            /* it does not make sense to have leaf-list of empty type */
            if (!tpdf_flag && node->nodetype == LYS_LEAFLIST && stype->base == LY_TYPE_EMPTY) {
                LOGWRN("The leaf-list \"%s\" is of \"empty\" type, which does not make sense.", node->name);
            }
        } else if (rc == EXIT_FAILURE && stype->base != LY_TYPE_ERR) {
            /* forward reference - in case the type is in grouping, we have to make the grouping unusable
             * by uses statement until the type is resolved. We do that the same way as uses statements inside
             * grouping - the grouping's nacm member (not used un grouping) is used to increase the number of
             * so far unresolved items (uses and types). The grouping cannot be used unless the nacm value is 0.
             * To remember that the grouping already increased grouping's nacm, the LY_TYPE_ERR is used as value
             * of the type's base member. */
            for (par_grp = node; par_grp && (par_grp->nodetype != LYS_GROUPING); par_grp = lys_parent(par_grp));
            if (par_grp) {
                ((struct lys_node_grp *)par_grp)->nacm++;
                stype->base = LY_TYPE_ERR;
            }
        }
        break;
    case UNRES_IFFEAT:
        iff_data = str_snode;
        rc = resolve_feature(iff_data->fname, strlen(iff_data->fname), iff_data->node, item);
        if (!rc) {
            /* success */
            if (iff_data->infeature) {
                /* store backlink into the target feature to allow reverse changes in case of changing feature status */
                feat = *((struct lys_feature **)item);
                if (!feat->depfeatures) {
                    feat->depfeatures = ly_set_new();
                }
                ly_set_add(feat->depfeatures, iff_data->node, LY_SET_OPT_USEASLIST);
            }
            /* cleanup temporary data */
            lydict_remove(mod->ctx, iff_data->fname);
            free(iff_data);
        }
        break;
    case UNRES_FEATURE:
        feat = (struct lys_feature *)item;

        if (feat->iffeature_size) {
            refs = ly_set_new();
            procs = ly_set_new();
            ly_set_add(procs, feat, 0);

            while (procs->number) {
                ref = procs->set.g[procs->number - 1];
                ly_set_rm_index(procs, procs->number - 1);

                for (i = 0; i < ref->iffeature_size; i++) {
                    resolve_iffeature_getsizes(&ref->iffeature[i], NULL, &j);
                    for (; j > 0 ; j--) {
                        if (ref->iffeature[i].features[j - 1]) {
                            if (ref->iffeature[i].features[j - 1] == feat) {
                                LOGVAL(LYE_CIRC_FEATURES, LY_VLOG_NONE, NULL, feat->name);
                                goto featurecheckdone;
                            }

                            if (ref->iffeature[i].features[j - 1]->iffeature_size) {
                                k = refs->number;
                                if (ly_set_add(refs, ref->iffeature[i].features[j - 1], 0) == k) {
                                    /* not yet seen feature, add it for processing */
                                    ly_set_add(procs, ref->iffeature[i].features[j - 1], 0);
                                }
                            }
                        } else {
                            /* forward reference */
                            rc = EXIT_FAILURE;
                            goto featurecheckdone;
                        }
                    }

                }
            }
            rc = EXIT_SUCCESS;

featurecheckdone:
            ly_set_free(refs);
            ly_set_free(procs);
        }

        break;
    case UNRES_USES:
        rc = resolve_unres_schema_uses(item, unres);
        break;
    case UNRES_TYPE_DFLT:
        stype = item;

        rc = check_default(stype, (const char **)str_snode, mod);
        break;
    case UNRES_CHOICE_DFLT:
        expr = str_snode;
        has_str = 1;
        choic = item;

        if (!choic->dflt) {
            choic->dflt = resolve_choice_dflt(choic, expr);
        }
        if (choic->dflt) {
            rc = lyp_check_mandatory_choice((struct lys_node *)choic);
        } else {
            rc = EXIT_FAILURE;
        }
        break;
    case UNRES_LIST_KEYS:
        rc = resolve_list_keys(item, ((struct lys_node_list *)item)->keys_str);
        break;
    case UNRES_LIST_UNIQ:
        unique_info = (struct unres_list_uniq *)item;
        rc = resolve_unique(unique_info->list, unique_info->expr, unique_info->trg_type);
        break;
    case UNRES_AUGMENT:
        rc = resolve_augment(item, NULL, unres);
        break;
    case UNRES_XPATH:
        node = (struct lys_node *)item;
        rc = lys_check_xpath(node, 1);
        break;
    default:
        LOGINT;
        break;
    }

    if (has_str && !rc) {
        /* the string is no more needed in case of success.
         * In case of forward reference, we will try to resolve the string later */
        lydict_remove(mod->ctx, str_snode);
    }

    return rc;
}

/* logs directly */
static void
print_unres_schema_item_fail(void *item, enum UNRES_ITEM type, void *str_node)
{
    struct lyxml_elem *xml;
    struct lyxml_attr *attr;
    struct unres_iffeat_data *iff_data;
    const char *type_name = NULL;

    switch (type) {
    case UNRES_IDENT:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "identity", (char *)str_node);
        break;
    case UNRES_TYPE_IDENTREF:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "identityref", (char *)str_node);
        break;
    case UNRES_TYPE_LEAFREF:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "leafref",
               ((struct lys_type *)item)->info.lref.path);
        break;
    case UNRES_TYPE_DER_TPDF:
    case UNRES_TYPE_DER:
        xml = (struct lyxml_elem *)((struct lys_type *)item)->der;
        if (xml->flags & LY_YANG_STRUCTURE_FLAG) {
            type_name = ((struct yang_type *)xml)->name;
        } else {
            LY_TREE_FOR(xml->attr, attr) {
                if ((attr->type == LYXML_ATTR_STD) && !strcmp(attr->name, "name")) {
                    type_name = attr->value;
                    break;
                }
            }
            assert(attr);
        }
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "derived type", type_name);
        break;
    case UNRES_IFFEAT:
        iff_data = str_node;
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "if-feature", iff_data->fname);
        break;
    case UNRES_FEATURE:
        LOGVRB("There are unresolved if-features for \"%s\" feature circular dependency check, it will be attempted later",
               ((struct lys_feature *)item)->name);
        break;
    case UNRES_USES:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "uses", ((struct lys_node_uses *)item)->name);
        break;
    case UNRES_TYPE_DFLT:
        if (str_node) {
            LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "type default", (char *)str_node);
        }   /* else no default value in the type itself, but we are checking some restrictions against
             *  possible default value of some base type. The failure is caused by not resolved base type,
             *  so it was already reported */
        break;
    case UNRES_CHOICE_DFLT:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "choice default", (char *)str_node);
        break;
    case UNRES_LIST_KEYS:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "list keys", (char *)str_node);
        break;
    case UNRES_LIST_UNIQ:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "list unique", (char *)str_node);
        break;
    case UNRES_AUGMENT:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "augment target",
               ((struct lys_node_augment *)item)->target_name);
        break;
    case UNRES_XPATH:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "XPath expressions of",
               ((struct lys_node *)item)->name);
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
    uint32_t i, resolved = 0, unres_count, res_count;
    int rc;

    assert(unres);

    LOGVRB("Resolving \"%s\" unresolved schema nodes and their constraints...", mod->name);
    ly_vlog_hide(1);

    /* uses */
    do {
        unres_count = 0;
        res_count = 0;

        for (i = 0; i < unres->count; ++i) {
            /* UNRES_TYPE_LEAFREF must be resolved (for storing leafref target pointers);
             * if-features are resolved here to make sure that we will have all if-features for
             * later check of feature circular dependency */
            if (unres->type[i] > UNRES_IDENT) {
                continue;
            }
            /* processes UNRES_USES, UNRES_IFFEAT, UNRES_TYPE_DER, UNRES_TYPE_DER_TPDF, UNRES_TYPE_LEAFREF,
             * UNRES_AUGMENT, UNRES_CHOICE_DFLT and UNRES_IDENT */

            ++unres_count;
            rc = resolve_unres_schema_item(mod, unres->item[i], unres->type[i], unres->str_snode[i], unres);
            if (!rc) {
                unres->type[i] = UNRES_RESOLVED;
                ++resolved;
                ++res_count;
            } else if (rc == -1) {
                ly_vlog_hide(0);
                /* print the error */
                resolve_unres_schema_item(mod, unres->item[i], unres->type[i], unres->str_snode[i], unres);
                return -1;
            } else {
                /* forward reference, erase ly_errno */
                ly_err_clean(1);
            }
        }
    } while (res_count && (res_count < unres_count));

    if (res_count < unres_count) {
        /* just print the errors */
        ly_vlog_hide(0);

        for (i = 0; i < unres->count; ++i) {
            if (unres->type[i] > UNRES_IDENT) {
                continue;
            }
            resolve_unres_schema_item(mod, unres->item[i], unres->type[i], unres->str_snode[i], unres);
        }
        return -1;
    }

    /* the rest */
    for (i = 0; i < unres->count; ++i) {
        if (unres->type[i] == UNRES_RESOLVED) {
            continue;
        }

        rc = resolve_unres_schema_item(mod, unres->item[i], unres->type[i], unres->str_snode[i], unres);
        if (rc == 0) {
            if (unres->type[i] == UNRES_LIST_UNIQ) {
                /* free the allocated structure */
                free(unres->item[i]);
            }
            unres->type[i] = UNRES_RESOLVED;
            ++resolved;
        } else if (rc == -1) {
            ly_vlog_hide(0);
            /* print the error */
            resolve_unres_schema_item(mod, unres->item[i], unres->type[i], unres->str_snode[i], unres);
            return -1;
        }
    }

    ly_vlog_hide(0);

    if (resolved < unres->count) {
        /* try to resolve the unresolved nodes again, it will not resolve anything, but it will print
         * all the validation errors
         */
        for (i = 0; i < unres->count; ++i) {
            if (unres->type[i] == UNRES_RESOLVED) {
                continue;
            }
            if (unres->type[i] == UNRES_XPATH) {
                /* unresolvable XPaths are actually supposed to be warnings - they may be
                 * unresolved due to the not implemented target module so it shouldn't avoid
                 * parsing the module, but we still want to announce some issue here */
                ly_vlog_hide(0xff);
            }
            resolve_unres_schema_item(mod, unres->item[i], unres->type[i], unres->str_snode[i], unres);
            if (unres->type[i] == UNRES_XPATH && *ly_vlog_hide_location() == 0xff) {
                unres->type[i] = UNRES_RESOLVED;
                resolved++;
                ly_vlog_hide(0);
            }
        }
        if (resolved < unres->count) {
            return -1;
        }
    }

    LOGVRB("All \"%s\" schema nodes and constraints resolved.", mod->name);
    unres->count = 0;
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
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on storing the item in unres, -1 on error.
 */
int
unres_schema_add_str(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type,
                     const char *str)
{
    int rc;
    const char *dictstr;

    dictstr = lydict_insert(mod->ctx, str, 0);
    rc = unres_schema_add_node(mod, unres, item, type, (struct lys_node *)dictstr);

    if (rc == -1) {
        lydict_remove(mod->ctx, dictstr);
    }
    return rc;
}

/**
 * @brief Try to resolve an unres schema item with a schema node argument. Logs indirectly.
 *
 * @param[in] mod Main module.
 * @param[in] unres Unres schema structure to use.
 * @param[in] item Item to resolve. Type determined by \p type.
 * @param[in] type Type of the unresolved item. UNRES_TYPE_DER is handled specially!
 * @param[in] snode Schema node argument.
 *
 * @return EXIT_SUCCESS on success, EXIT_FIALURE on storing the item in unres, -1 on error.
 */
int
unres_schema_add_node(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type,
                      struct lys_node *snode)
{
    int rc, log_hidden;
    uint32_t u;
    struct lyxml_elem *yin;

    assert(unres && item && ((type != UNRES_LEAFREF) && (type != UNRES_INSTID) && (type != UNRES_WHEN)
           && (type != UNRES_MUST)));

    /* check for duplicities in unres */
    for (u = 0; u < unres->count; u++) {
        if (unres->type[u] == type && unres->item[u] == item &&
                unres->str_snode[u] == snode && unres->module[u] == mod) {
            /* duplication, will be resolved later */
            return EXIT_FAILURE;
        }
    }

    if (*ly_vlog_hide_location()) {
        log_hidden = 1;
    } else {
        log_hidden = 0;
        ly_vlog_hide(1);
    }
    rc = resolve_unres_schema_item(mod, item, type, snode, unres);
    if (!log_hidden) {
        ly_vlog_hide(0);
    }

    if (rc != EXIT_FAILURE) {
        if (rc == -1 && ly_errno == LY_EVALID) {
            ly_err_repeat();
        }
        if (type == UNRES_LIST_UNIQ) {
            /* free the allocated structure */
            free(item);
        } else if (rc == -1 && type == UNRES_IFFEAT) {
            /* free the allocated resources */
            free(*((char **)item));
         }
        return rc;
    } else {
        /* erase info about validation errors */
        ly_err_clean(1);
    }

    print_unres_schema_item_fail(item, type, snode);

    /* HACK unlinking is performed here so that we do not do any (NS) copying in vain */
    if (type == UNRES_TYPE_DER || type == UNRES_TYPE_DER_TPDF) {
        yin = (struct lyxml_elem *)((struct lys_type *)item)->der;
        if (!(yin->flags & LY_YANG_STRUCTURE_FLAG)) {
            lyxml_unlink_elem(mod->ctx, yin, 1);
            ((struct lys_type *)item)->der = (struct lys_tpdf *)yin;
        }
    }

    unres->count++;
    unres->item = ly_realloc(unres->item, unres->count*sizeof *unres->item);
    if (!unres->item) {
        LOGMEM;
        return -1;
    }
    unres->item[unres->count-1] = item;
    unres->type = ly_realloc(unres->type, unres->count*sizeof *unres->type);
    if (!unres->type) {
        LOGMEM;
        return -1;
    }
    unres->type[unres->count-1] = type;
    unres->str_snode = ly_realloc(unres->str_snode, unres->count*sizeof *unres->str_snode);
    if (!unres->str_snode) {
        LOGMEM;
        return -1;
    }
    unres->str_snode[unres->count-1] = snode;
    unres->module = ly_realloc(unres->module, unres->count*sizeof *unres->module);
    if (!unres->module) {
        LOGMEM;
        return -1;
    }
    unres->module[unres->count-1] = mod;

    return rc;
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
 * @return EXIT_SUCCESS on success, EXIT_FAILURE if item is not in unres, -1 on error.
 */
int
unres_schema_dup(struct lys_module *mod, struct unres_schema *unres, void *item, enum UNRES_ITEM type, void *new_item)
{
    int i;
    struct unres_list_uniq aux_uniq;
    struct unres_iffeat_data *iff_data;

    assert(item && new_item && ((type != UNRES_LEAFREF) && (type != UNRES_INSTID) && (type != UNRES_WHEN)));

    /* hack for UNRES_LIST_UNIQ, which stores multiple items behind its item */
    if (type == UNRES_LIST_UNIQ) {
        aux_uniq.list = item;
        aux_uniq.expr = ((struct unres_list_uniq *)new_item)->expr;
        item = &aux_uniq;
    }
    i = unres_schema_find(unres, -1, item, type);

    if (i == -1) {
        if (type == UNRES_LIST_UNIQ) {
            free(new_item);
        }
        return EXIT_FAILURE;
    }

    if ((type == UNRES_TYPE_LEAFREF) || (type == UNRES_USES) || (type == UNRES_TYPE_DFLT) ||
            (type == UNRES_FEATURE) || (type == UNRES_LIST_UNIQ)) {
        if (unres_schema_add_node(mod, unres, new_item, type, unres->str_snode[i]) == -1) {
            LOGINT;
            return -1;
        }
    } else if (type == UNRES_IFFEAT) {
        /* duplicate unres_iffeature_data */
        iff_data = malloc(sizeof *iff_data);
        iff_data->fname = lydict_insert(mod->ctx, ((struct unres_iffeat_data *)unres->str_snode[i])->fname, 0);
        iff_data->node = ((struct unres_iffeat_data *)unres->str_snode[i])->node;
        if (unres_schema_add_node(mod, unres, new_item, type, (struct lys_node *)iff_data) == -1) {
            LOGINT;
            return -1;
        }
    } else {
        if (unres_schema_add_str(mod, unres, new_item, type, unres->str_snode[i]) == -1) {
            LOGINT;
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

/* does not log */
int
unres_schema_find(struct unres_schema *unres, int start_on_backwards, void *item, enum UNRES_ITEM type)
{
    int i;
    struct unres_list_uniq *aux_uniq1, *aux_uniq2;

    if (start_on_backwards > 0) {
        i = start_on_backwards;
    } else {
        i = unres->count - 1;
    }
    for (; i > -1; i--) {
        if (unres->type[i] != type) {
            continue;
        }
        if (type != UNRES_LIST_UNIQ) {
            if (unres->item[i] == item) {
                break;
            }
        } else {
            aux_uniq1 = (struct unres_list_uniq *)unres->item[i - 1];
            aux_uniq2 = (struct unres_list_uniq *)item;
            if ((aux_uniq1->list == aux_uniq2->list) && ly_strequal(aux_uniq1->expr, aux_uniq2->expr, 0)) {
                break;
            }
        }
    }

    return i;
}

static void
unres_schema_free_item(struct ly_ctx *ctx, struct unres_schema *unres, uint32_t i)
{
    struct lyxml_elem *yin;
    struct yang_type *yang;
    struct unres_iffeat_data *iff_data;

    switch (unres->type[i]) {
    case UNRES_TYPE_DER_TPDF:
    case UNRES_TYPE_DER:
        yin = (struct lyxml_elem *)((struct lys_type *)unres->item[i])->der;
        if (yin->flags & LY_YANG_STRUCTURE_FLAG) {
            yang =(struct yang_type *)yin;
            ((struct lys_type *)unres->item[i])->base = yang->base;
            lydict_remove(ctx, yang->name);
            free(yang);
            if (((struct lys_type *)unres->item[i])->base == LY_TYPE_UNION) {
                yang_free_type_union(ctx, (struct lys_type *)unres->item[i]);
            }
        } else {
            lyxml_free(ctx, yin);
        }
        break;
    case UNRES_IFFEAT:
        iff_data = (struct unres_iffeat_data *)unres->str_snode[i];
        lydict_remove(ctx, iff_data->fname);
        free(unres->str_snode[i]);
        break;
    case UNRES_IDENT:
    case UNRES_TYPE_IDENTREF:
    case UNRES_CHOICE_DFLT:
    case UNRES_LIST_KEYS:
        lydict_remove(ctx, (const char *)unres->str_snode[i]);
        break;
    case UNRES_LIST_UNIQ:
        free(unres->item[i]);
        break;
    default:
        break;
    }
    unres->type[i] = UNRES_RESOLVED;
}

void
unres_schema_free(struct lys_module *module, struct unres_schema **unres)
{
    uint32_t i;
    unsigned int unresolved = 0;

    if (!unres || !(*unres)) {
        return;
    }

    assert(module || (*unres)->count == 0);

    for (i = 0; i < (*unres)->count; ++i) {
        if ((*unres)->module[i] != module) {
            if ((*unres)->type[i] != UNRES_RESOLVED) {
                unresolved++;
            }
            continue;
        }

        /* free heap memory for the specific item */
        unres_schema_free_item(module->ctx, *unres, i);
    }

    /* free it all */
    if (!module || (!unresolved && !module->type)) {
        free((*unres)->item);
        free((*unres)->type);
        free((*unres)->str_snode);
        free((*unres)->module);
        free((*unres));
        (*unres) = NULL;
    }
}

/**
 * @brief Resolve instance-identifier in JSON data format. Logs directly.
 *
 * @param[in] data Data node where the path is used
 * @param[in] path Instance-identifier node value.
 * @param[in,out] ret Resolved instance or NULL.
 *
 * @return 0 on success (even if unresolved and \p ret is NULL), -1 on error.
 */
static int
resolve_instid(struct lyd_node *data, const char *path, int req_inst, struct lyd_node **ret)
{
    int i = 0, j;
    const struct lys_module *mod;
    struct ly_ctx *ctx = data->schema->module->ctx;
    const char *model, *name;
    char *str;
    int mod_len, name_len, has_predicate;
    struct unres_data node_match;

    memset(&node_match, 0, sizeof node_match);
    *ret = NULL;

    /* we need root to resolve absolute path */
    for (; data->parent; data = data->parent);
    /* we're still parsing it and the pointer is not correct yet */
    if (data->prev) {
        for (; data->prev->next; data = data->prev);
    }

    /* search for the instance node */
    while (path[i]) {
        j = parse_instance_identifier(&path[i], &model, &mod_len, &name, &name_len, &has_predicate);
        if (j <= 0) {
            LOGVAL(LYE_INCHAR, LY_VLOG_LYD, data, path[i-j], &path[i-j]);
            goto error;
        }
        i += j;

        str = strndup(model, mod_len);
        if (!str) {
            LOGMEM;
            goto error;
        }
        mod = ly_ctx_get_module(ctx, str, NULL);
        free(str);

        if (resolve_data(mod, name, name_len, data, &node_match)) {
            /* no instance exists */
            break;
        }

        if (has_predicate) {
            /* we have predicate, so the current results must be list or leaf-list */
            j = resolve_predicate(&path[i], &node_match);
            if (j < 1) {
                LOGVAL(LYE_INPRED, LY_VLOG_LYD, data, &path[i-j]);
                goto error;
            }
            i += j;

            if (!node_match.count) {
                /* no instance exists */
                break;
            }
        }
    }

    if (!node_match.count) {
        /* no instance exists */
        if (req_inst > -1) {
            LOGVAL(LYE_NOREQINS, LY_VLOG_NONE, NULL, path);
            return EXIT_FAILURE;
        }
        LOGVRB("There is no instance of \"%s\", but it is not required.", path);
        return EXIT_SUCCESS;
    } else if (node_match.count > 1) {
        /* instance identifier must resolve to a single node */
        LOGVAL(LYE_TOOMANY, LY_VLOG_LYD, data, path, "data tree");
        goto error;
    } else {
        /* we have required result, remember it and cleanup */
        *ret = node_match.node[0];
        free(node_match.node);
        return EXIT_SUCCESS;
    }

error:
    /* cleanup */
    free(node_match.node);
    return -1;
}

static int
resolve_leafref(struct lyd_node_leaf_list *leaf, const char *path, int req_inst, struct lyd_node **ret)
{
    struct unres_data matches;
    uint32_t i;

    /* init */
    memset(&matches, 0, sizeof matches);
    *ret = NULL;

    /* EXIT_FAILURE return keeps leaf->value.lefref NULL, handled later */
    if (resolve_path_arg_data((struct lyd_node *)leaf, path, &matches) == -1) {
        return -1;
    }

    /* check that value matches */
    for (i = 0; i < matches.count; ++i) {
        /* not that the value is already in canonical form since the parsers does the conversion,
         * so we can simply compare just the values */
        if (ly_strequal(leaf->value_str, ((struct lyd_node_leaf_list *)matches.node[i])->value_str, 1)) {
            /* we have the match */
            *ret = matches.node[i];
            break;
        }
    }

    free(matches.node);

    if (!*ret) {
        /* reference not found */
        if (req_inst > -1) {
            LOGVAL(LYE_NOLEAFREF, LY_VLOG_LYD, leaf, path, leaf->value_str);
            return EXIT_FAILURE;
        } else {
            LOGVRB("There is no leafref \"%s\" with the value \"%s\", but it is not required.", path, leaf->value_str);
        }
    }

    return EXIT_SUCCESS;
}

/* ignore fail because we are parsing edit-config, get, or get-config - but only if the union includes leafref or instid */
static int
resolve_union(struct lyd_node_leaf_list *leaf, struct lys_type *type, int ignore_fail)
{
    struct lys_type *t;
    struct lyd_node *ret, *par, *op_node;
    int found, hidden, success = 0;
    const char *json_val = NULL;

    assert(type->base == LY_TYPE_UNION);

    if ((leaf->value_type == LY_TYPE_UNION) || (leaf->value_type == (LY_TYPE_INST | LY_TYPE_INST_UNRES))) {
        /* either NULL or instid previously converted to JSON */
        json_val = leaf->value.string;
    }
    memset(&leaf->value, 0, sizeof leaf->value);

    /* turn logging off, we are going to try to validate the value with all the types in order */
    hidden = *ly_vlog_hide_location();
    ly_vlog_hide(1);

    t = NULL;
    found = 0;
    while ((t = lyp_get_next_union_type(type, t, &found))) {
        found = 0;

        switch (t->base) {
        case LY_TYPE_LEAFREF:
            if (!resolve_leafref(leaf, t->info.lref.path, (ignore_fail ? -1 : t->info.lref.req), &ret)) {
                if (ret && !(leaf->schema->flags & LYS_LEAFREF_DEP)) {
                    /* valid resolved */
                    leaf->value.leafref = ret;
                    leaf->value_type = LY_TYPE_LEAFREF;
                } else {
                    /* valid unresolved */
                    if (!lyp_parse_value(t, &leaf->value_str, NULL, leaf, 1, 0)) {
                        return -1;
                    }
                }

                success = 1;
            }
            break;
        case LY_TYPE_INST:
            if (!resolve_instid((struct lyd_node *)leaf, (json_val ? json_val : leaf->value_str),
                                (ignore_fail ? -1 : t->info.inst.req), &ret)) {
                if (ret) {
                    for (op_node = (struct lyd_node *)leaf;
                         op_node && !(op_node->schema->nodetype & (LYS_RPC | LYS_NOTIF | LYS_ACTION));
                         op_node = op_node->parent);
                    if (op_node) {
                        /* this is an RPC/notif/action */
                        for (par = ret->parent; par && (par != op_node); par = par->parent);
                        if (!par) {
                            /* target instance is outside the operation - do not store the pointer */
                            ret = NULL;
                        }
                    }
                }
                if (ret) {
                    /* valid resolved */
                    leaf->value.instance = ret;
                    leaf->value_type = LY_TYPE_INST;

                    if (json_val) {
                        lydict_remove(leaf->schema->module->ctx, leaf->value_str);
                        leaf->value_str = json_val;
                        json_val = NULL;
                    }
                } else {
                    /* valid unresolved */
                    if (json_val) {
                        /* put the JSON val back */
                        leaf->value.string = json_val;
                        json_val = NULL;
                    } else {
                        leaf->value.instance = NULL;
                    }
                    leaf->value_type = LY_TYPE_INST | LY_TYPE_INST_UNRES;
                }

                success = 1;
            }
            break;
        default:
            if (lyp_parse_value(t, &leaf->value_str, NULL, leaf, 1, 0)) {
                success = 1;
            }
            break;
        }

        if (success) {
            break;
        }

        /* erase information about errors - they are false or irrelevant
         * and will be replaced by a single error messages */
        ly_err_clean(1);

        /* erase possible present and invalid value data */
        if (t->base == LY_TYPE_BITS) {
            free(leaf->value.bit);
        }
        memset(&leaf->value, 0, sizeof leaf->value);
    }

    /* turn logging back on */
    if (!hidden) {
        ly_vlog_hide(0);
    }

    if (json_val) {
        if (!success) {
            /* put the value back for now */
            assert(leaf->value_type == LY_TYPE_UNION);
            leaf->value.string = json_val;
        } else {
            /* value was ultimately useless, but we could not have known */
            lydict_remove(leaf->schema->module->ctx, json_val);
        }
    }

    if (!success && (!ignore_fail || !type->info.uni.has_ptr_type)) {
        /* not found and it is required */
        LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, leaf->value_str ? leaf->value_str : "", leaf->schema->name);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}

/**
 * @brief Resolve a single unres data item. Logs directly.
 *
 * @param[in] node Data node to resolve.
 * @param[in] type Type of the unresolved item.
 * @param[in] ignore_fails Flag whether to ignore any false condition or unresolved nodes (e.g., for LYD_OPT_EDIT).
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
int
resolve_unres_data_item(struct lyd_node *node, enum UNRES_ITEM type, int ignore_fail)
{
    int rc, req_inst;
    struct lyd_node_leaf_list *leaf;
    struct lyd_node *ret, *op_node, *par;
    struct lys_node_leaf *sleaf;

    leaf = (struct lyd_node_leaf_list *)node;
    sleaf = (struct lys_node_leaf *)leaf->schema;

    switch (type) {
    case UNRES_LEAFREF:
        assert(sleaf->type.base == LY_TYPE_LEAFREF);
        assert(leaf->validity & LYD_VAL_LEAFREF);
        req_inst = (ignore_fail ? -1 : sleaf->type.info.lref.req);
        rc = resolve_leafref(leaf, sleaf->type.info.lref.path, req_inst, &ret);
        if (!rc) {
            if (ret && !(leaf->schema->flags & LYS_LEAFREF_DEP)) {
                /* valid resolved */
                leaf->value.leafref = ret;
                leaf->value_type = LY_TYPE_LEAFREF;
            } else {
                /* valid unresolved */
                if (!(leaf->value_type & LY_TYPE_LEAFREF_UNRES)) {
                    if (!lyp_parse_value(&sleaf->type, &leaf->value_str, NULL, leaf, 1, 0)) {
                        return -1;
                    }
                }
            }
            leaf->validity &= ~LYD_VAL_LEAFREF;
        } else {
            return rc;
        }
        break;

    case UNRES_INSTID:
        assert(sleaf->type.base == LY_TYPE_INST);
        req_inst = (ignore_fail ? -1 : sleaf->type.info.inst.req);
        rc = resolve_instid(node, leaf->value_str, req_inst, &ret);
        if (!rc) {
            if (ret) {
                for (op_node = (struct lyd_node *)leaf;
                     op_node && !(op_node->schema->nodetype & (LYS_RPC | LYS_NOTIF | LYS_ACTION));
                     op_node = op_node->parent);
                if (op_node) {
                    /* this is an RPC/notif/action */
                    for (par = ret->parent; par && (par != op_node); par = par->parent);
                    if (!par) {
                        /* target instance is outside the operation - do not store the pointer */
                        ret = NULL;
                    }
                }
            }
            if (ret) {
                /* valid resolved */
                leaf->value.instance = ret;
                leaf->value_type = LY_TYPE_INST;
            } else {
                /* valid unresolved */
                leaf->value.instance = NULL;
                leaf->value_type = LY_TYPE_INST | LY_TYPE_INST_UNRES;
            }
        } else {
            return rc;
        }
        break;

    case UNRES_UNION:
        assert(sleaf->type.base == LY_TYPE_UNION);
        return resolve_union(leaf, &sleaf->type, ignore_fail);

    case UNRES_WHEN:
        if ((rc = resolve_when(node, NULL, ignore_fail))) {
            return rc;
        }
        break;

    case UNRES_MUST:
        if ((rc = resolve_must(node, 0, ignore_fail))) {
            return rc;
        }
        break;

    case UNRES_MUST_INOUT:
        if ((rc = resolve_must(node, 1, ignore_fail))) {
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
 * @brief add data unres item
 *
 * @param[in] unres Unres data structure to use.
 * @param[in] node Data node to use.
 *
 * @return 0 on success, -1 on error.
 */
int
unres_data_add(struct unres_data *unres, struct lyd_node *node, enum UNRES_ITEM type)
{
    assert(unres && node);
    assert((type == UNRES_LEAFREF) || (type == UNRES_INSTID) || (type == UNRES_WHEN) || (type == UNRES_MUST)
           || (type == UNRES_MUST_INOUT) || (type == UNRES_UNION));

    unres->count++;
    unres->node = ly_realloc(unres->node, unres->count * sizeof *unres->node);
    if (!unres->node) {
        LOGMEM;
        return -1;
    }
    unres->node[unres->count - 1] = node;
    unres->type = ly_realloc(unres->type, unres->count * sizeof *unres->type);
    if (!unres->type) {
        LOGMEM;
        return -1;
    }
    unres->type[unres->count - 1] = type;

    if (type == UNRES_WHEN) {
        /* remove previous result */
        node->when_status = LYD_WHEN;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve every unres data item in the structure. Logs directly.
 *
 * If options includes LYD_OPT_TRUSTED, the data are considered trusted (when, must conditions are not expected,
 * unresolved leafrefs/instids are accepted).
 *
 * If options includes LYD_OPT_NOAUTODEL, the false resulting when condition on non-default nodes, the error is raised.
 *
 * @param[in] unres Unres data structure to use.
 * @param[in,out] root Root node of the data tree, can be changed due to autodeletion.
 * @param[in] options Data options as described above.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
int
resolve_unres_data(struct unres_data *unres, struct lyd_node **root, int options)
{
    uint32_t i, j, first = 1, resolved = 0, del_items = 0, when_stmt = 0;
    int rc, progress, ignore_fails;
    struct lyd_node *parent;
    //struct lyd_node_leaf_list *leaf;

    assert(root);
    assert(unres);

    if (!unres->count) {
        return EXIT_SUCCESS;
    }

    if (options & (LYD_OPT_TRUSTED | LYD_OPT_NOTIF_FILTER | LYD_OPT_GET | LYD_OPT_GETCONFIG | LYD_OPT_EDIT)) {
        ignore_fails = 1;
    } else {
        ignore_fails = 0;
    }

    LOGVRB("Resolving unresolved data nodes and their constraints...");
    ly_vlog_hide(1);

    /* when-stmt first */
    do {
        ly_err_clean(1);
        progress = 0;
        for (i = 0; i < unres->count; i++) {
            if (unres->type[i] != UNRES_WHEN) {
                continue;
            }
            if (first) {
                /* count when-stmt nodes in unres list */
                when_stmt++;
            }

            /* resolve when condition only when all parent when conditions are already resolved */
            for (parent = unres->node[i]->parent;
                 parent && LYD_WHEN_DONE(parent->when_status);
                 parent = parent->parent) {
                if (!parent->parent && (parent->when_status & LYD_WHEN_FALSE)) {
                    /* the parent node was already unlinked, do not resolve this node,
                     *  it will be removed anyway, so just mark it as resolved
                     */
                    unres->node[i]->when_status |= LYD_WHEN_FALSE;
                    unres->type[i] = UNRES_RESOLVED;
                    resolved++;
                    break;
                }
            }
            if (parent) {
                continue;
            }

            rc = resolve_unres_data_item(unres->node[i], unres->type[i], ignore_fails);
            if (!rc) {
                if (unres->node[i]->when_status & LYD_WHEN_FALSE) {
                    if ((options & LYD_OPT_NOAUTODEL) && !unres->node[i]->dflt) {
                        /* false when condition */
                        ly_vlog_hide(0);
                        ly_err_repeat();
                        return -1;
                    } /* follows else */

                    /* only unlink now, the subtree can contain another nodes stored in the unres list */
                    /* if it has parent non-presence containers that would be empty, we should actually
                     * remove the container
                     */
                    for (parent = unres->node[i];
                            parent->parent && parent->parent->schema->nodetype == LYS_CONTAINER;
                            parent = parent->parent) {
                        if (((struct lys_node_container *)parent->parent->schema)->presence) {
                            /* presence container */
                            break;
                        }
                        if (parent->next || parent->prev != parent) {
                            /* non empty (the child we are in and we are going to remove is not the only child) */
                            break;
                        }
                    }
                    unres->node[i] = parent;

                    /* auto-delete */
                    LOGVRB("auto-delete node \"%s\" due to when condition (%s)", ly_errpath(),
                                    ((struct lys_node_leaf *)unres->node[i]->schema)->when->cond);
                    if (*root && *root == unres->node[i]) {
                        *root = (*root)->next;
                    }

                    lyd_unlink(unres->node[i]);
                    unres->type[i] = UNRES_DELETE;
                    del_items++;

                    /* update the rest of unres items */
                    for (j = 0; j < unres->count; j++) {
                        if (unres->type[j] == UNRES_RESOLVED || unres->type[j] == UNRES_DELETE) {
                            continue;
                        }

                        /* test if the node is in subtree to be deleted */
                        for (parent = unres->node[j]; parent; parent = parent->parent) {
                            if (parent == unres->node[i]) {
                                /* yes, it is */
                                unres->type[j] = UNRES_RESOLVED;
                                resolved++;
                                break;
                            }
                        }
                    }
                } else {
                    unres->type[i] = UNRES_RESOLVED;
                }
                ly_err_clean(1);
                resolved++;
                progress = 1;
            } else if (rc == -1) {
                ly_vlog_hide(0);
                /* print only this last error */
                resolve_unres_data_item(unres->node[i], unres->type[i], ignore_fails);
                return -1;
            } /* else forward reference */
        }
        first = 0;
    } while (progress && resolved < when_stmt);

    /* do we have some unresolved when-stmt? */
    if (when_stmt > resolved) {
        ly_vlog_hide(0);
        ly_err_repeat();
        return -1;
    }

    for (i = 0; del_items && i < unres->count; i++) {
        /* we had some when-stmt resulted to false, so now we have to sanitize the unres list */
        if (unres->type[i] != UNRES_DELETE) {
            continue;
        }
        if (!unres->node[i]) {
            unres->type[i] = UNRES_RESOLVED;
            del_items--;
            continue;
        }

        /* really remove the complete subtree */
        lyd_free(unres->node[i]);
        unres->type[i] = UNRES_RESOLVED;
        del_items--;
    }
    ly_vlog_hide(0);

    /* rest */
    for (i = 0; i < unres->count; ++i) {
        if (unres->type[i] == UNRES_RESOLVED) {
            continue;
        }
        assert(!(options & LYD_OPT_TRUSTED) || ((unres->type[i] != UNRES_MUST) && (unres->type[i] != UNRES_MUST_INOUT)));

        rc = resolve_unres_data_item(unres->node[i], unres->type[i], ignore_fails);
        if (rc) {
            /* since when was already resolved, a forward reference is an error */
            //ly_vlog_hide(0);
            /* print only this last error */
            //resolve_unres_data_item(unres->node[i], unres->type[i], ignore_fails);
            return -1;
        }

        unres->type[i] = UNRES_RESOLVED;
        //resolved++;
    }

    //ly_vlog_hide(0);
    //if (resolved < unres->count) {
        /* try to resolve the unresolved data again, it will not resolve anything, but it will print
         * all the validation errors
         */
        //for (i = 0; i < unres->count; ++i) {
            //if (unres->type[i] == UNRES_UNION) {
                /* does not make sense to print specific errors for all
                 * the data types, just print that the value is invalid */
                /*leaf = (struct lyd_node_leaf_list *)unres->node[i];
                LOGVAL(LYE_INVAL, LY_VLOG_LYD, unres->node[i], (leaf->value_str ? leaf->value_str : ""),
                       leaf->schema->name);
            } else if (unres->type[i] != UNRES_RESOLVED) {
                resolve_unres_data_item(unres->node[i], unres->type[i], ignore_fails);
            }
        }
        return -1;
    }*/

    LOGVRB("All data nodes and constraints resolved.");
    unres->count = 0;
    return EXIT_SUCCESS;
}
