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

    if (((id[0] == 'x') || (id[0] == 'X'))
            && (id[0] && ((id[1] == 'm') || (id[0] == 'M')))
            && (id[1] && ((id[2] == 'l') || (id[2] == 'L')))) {
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
 * @brief Parse instance-identifier in JSON data format. That means that prefixes
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
parse_instance_identifier(const char *id, const char **model, int *mod_len, const char **name, int *nam_len,
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
        return -parsed + ret;
    }

    parsed += ret;
    id += ret;

    if ((id[0] == '[') && has_predicate) {
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
                                     node->module->features[j].module, node->module->features[j].name, node)) {
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
                                 module->features[j].module, module->features[j].name, node)) {
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
                                     module->inc[i].submodule->features[j].name, node)) {
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
                          struct unres_schema *unres)
{
    const char *c = value;
    int r, rc = EXIT_FAILURE;
    int i, j, last_not, checkversion = 0;
    unsigned int f_size = 0, expr_size = 0, f_exp = 1;
    uint8_t op;
    struct iff_stack stack = {0, 0, NULL};

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
    iffeat_expr->features = malloc(f_size * sizeof *iffeat_expr->features);
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
             * forward referenced, we have hack for unres - until resolved,
             * the feature name is stored instead of link to the lys_feature */
            iffeat_expr->features[f_size] = (void*)strndup(&c[i], j - i);
            r = unres_schema_add_node(node->module, unres, &iffeat_expr->features[f_size], UNRES_IFFEAT, node);
            f_size--;

            if (r == -1) {
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
                           const struct lys_node **start)
{
    const struct lys_module *prefix_mod;
    int sh = 0;

    /* module check */
    prefix_mod = lys_get_import_module(module, NULL, 0, mod_name, mod_name_len);
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
        sh = 1;
    }

    /* the result node? */
    if (!id[0]) {
        if (*shorthand == 1) {
            return 1;
        }
        return 0;
    }

    if (!sh) {
        /* move down the tree, if possible */
        if (sibling->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
            return -1;
        }
        *start = sibling->child;
    }

    return 2;
}

/* start - relative, module - absolute, -1 error, EXIT_SUCCESS ok (but ret can still be NULL), >0 unexpected char on ret - 1 */
int
resolve_augment_schema_nodeid(const char *nodeid, const struct lys_node *start, const struct lys_module *module,
                              const struct lys_node **ret)
{
    const char *name, *mod_name, *id;
    const struct lys_node *sibling;
    int r, nam_len, mod_name_len, is_relative = -1;
    int8_t shorthand = 0;
    /* resolved import module from the start module, it must match the next node-name-match sibling */
    const struct lys_module *start_mod;

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
        if (!start_mod) {
            return -1;
        }
        start = start_mod->data;
    }

    while (1) {
        sibling = NULL;
        while ((sibling = lys_getnext(sibling, lys_parent(start), start_mod,
                                      LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE | LYS_GETNEXT_WITHINOUT))) {
            /* name match */
            if (sibling->name && !strncmp(name, sibling->name, nam_len) && !sibling->name[nam_len]) {
                r = schema_nodeid_siblingcheck(sibling, &shorthand, id, module, mod_name, mod_name_len, &start);
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
                r = schema_nodeid_siblingcheck(sibling, &shorthand, id, module, mod_name, mod_name_len, &start);
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
                r = schema_nodeid_siblingcheck(sibling, &shorthand, id, module, mod_name, mod_name_len, &start);
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

/* cannot return LYS_GROUPING, LYS_AUGMENT, LYS_USES, logs directly
 * data_nodeid - 0 schema nodeid, 1 - data nodeid with RPC input, 2 - data nodeid with RPC output */
const struct lys_node *
resolve_json_nodeid(const char *nodeid, struct ly_ctx *ctx, const struct lys_node *start, int data_nodeid)
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
        while ((sibling = lys_getnext(sibling, lys_parent(start), module, (data_nodeid ?
                0 : LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHCASE | LYS_GETNEXT_WITHINOUT)))) {
            /* name match */
            if (sibling->name && !strncmp(name, sibling->name, nam_len) && !sibling->name[nam_len]) {

                /* data RPC input/output check */
                if ((data_nodeid == 1) && lys_parent(sibling) && (lys_parent(sibling)->nodetype == LYS_OUTPUT)) {
                    continue;
                } else if ((data_nodeid == 2) && lys_parent(sibling) && (lys_parent(sibling)->nodetype == LYS_INPUT)) {
                    continue;
                }

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
                if (!data_nodeid && lys_parent(sibling) && (lys_parent(sibling)->nodetype == LYS_CHOICE) && (sibling->nodetype != LYS_CASE)) {
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

                if (data_nodeid || !shorthand) {
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
    const char *name, *value;
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

        /* value does not match */
        if (strncmp(key->value_str, value, val_len) || key->value_str[val_len]) {
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
    const char *id, *mod_name, *name;
    int r, ret, mod_name_len, nam_len, is_relative = -1, has_predicate, last_parsed;
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
        prev_mod = start->schema->module;
        start = start->child;
    } else {
        for (; start->parent; start = start->parent);
        prev_mod = start->schema->module;
    }

    while (1) {
        LY_TREE_FOR(start, sibling) {
            /* RPC data check, return simply invalid argument, because the data tree is invalid */
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
                if (prefix_mod != lys_node_module(sibling->schema)) {
                    continue;
                }

                /* leaf-list, did we find it with the correct value or not? */
                if (sibling->schema->nodetype == LYS_LEAFLIST) {
                    llist = (struct lyd_node_leaf_list *)sibling;
                    if ((!llist_value && llist->value_str && llist->value_str[0])
                            || (llist_value && strcmp(llist_value, llist->value_str))) {
                        continue;
                    }
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
                start = sibling->child;
                if (start) {
                    prev_mod = start->schema->module;
                }
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
    int64_t local_smin, local_smax;
    uint64_t local_umin, local_umax;
    long double local_fmin, local_fmax;
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
        local_fmin = ((long double)-9223372036854775808.0) / type->info.dec64.div;
        local_fmax = ((long double)9223372036854775807.0) / type->info.dec64.div;

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
                goto error;
            }
        } else {
            LOGINT;
            goto error;
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

                 if ((local_fmin >= tmp_intv->value.fval.min) && (local_fmin <= tmp_intv->value.fval.max)) {
                    if (local_fmax <= tmp_intv->value.fval.max) {
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
check_default(struct lys_type *type, const char *value, struct lys_module *module)
{
    struct lys_tpdf *base_tpdf = NULL;
    struct lyd_node_leaf_list node;
    int ret = EXIT_SUCCESS;

    if (type->base <= LY_TYPE_DER) {
        /* the type was not resolved yet, nothing to do for now */
        return EXIT_FAILURE;
    }

    if (!value) {
        /* we do not have a new default value, so is there any to check even, in some base type? */
        for (base_tpdf = type->der; base_tpdf->type.der; base_tpdf = base_tpdf->type.der) {
            if (base_tpdf->dflt) {
                value = base_tpdf->dflt;
                break;
            }
        }

        if (!value) {
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
    }

    /* dummy leaf */
    memset(&node, 0, sizeof node);
    node.value_str = value;
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
        ret = check_default(&type->info.lref.target->type, value, module);

    } else if ((type->base == LY_TYPE_INST) || (type->base == LY_TYPE_IDENT)) {
        /* it was converted to JSON format before, nothing else sensible we can do */

    } else {
        if (lyp_parse_value(&node, NULL, 1)) {
            ret = -1;
            if (base_tpdf) {
                /* default value was is defined in some base typedef */
                if ((type->base == LY_TYPE_BITS && type->der->type.der) ||
                        (type->base == LY_TYPE_ENUM && type->der->type.der)) {
                    /* we have refined bits/enums */
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                           "Invalid value \"%s\" of the default statement inherited to \"%s\" from \"%s\" base type.",
                           value, type->parent->name, base_tpdf->name);
                }
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
    if (key->type.base == LY_TYPE_EMPTY) {
        LOGVAL(LYE_KEY_TYPE, LY_VLOG_LYS, list, key->name);
        return -1;
    }

    /* config attribute is the same as of the list */
    if ((list->flags & LYS_CONFIG_MASK) != (key->flags & LYS_CONFIG_MASK)) {
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
        if ((i = parse_path_arg(path, &prefix, &pref_len, &name, &nam_len, &parent_times, &has_predicate)) < 1) {
            LOGVAL(LYE_INCHAR, LY_VLOG_LYD, node, path[-i], &path[-i]);
            rc = -1;
            goto error;
        }
        path += i;
        parsed += i;

        if (!ret->count) {
            if (parent_times != -1) {
                ret->count = 1;
                ret->node = calloc(1, sizeof *ret->node);
                if (!ret->node) {
                    LOGMEM;
                    rc = -1;
                    goto error;
                }
            }
            for (i = 0; i < parent_times; ++i) {
                /* relative path */
                if (!ret->count) {
                    /* error, too many .. */
                    LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, path, node->schema->name);
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

/**
 * @brief Resolve a path (leafref) predicate in JSON schema context. Logs directly.
 *
 * @param[in] path Path to use.
 * @param[in] context_node Predicate context node (where the predicate is placed).
 * @param[in] parent Path context node (where the path begins/is placed).
 *
 * @return 0 on forward reference, otherwise the number
 *         of characters successfully parsed,
 *         positive on success, negative on failure.
 */
static int
resolve_path_predicate_schema(const char *path, const struct lys_node *context_node,
                              struct lys_node *parent)
{
    const struct lys_node *src_node, *dst_node;
    const char *path_key_expr, *source, *sour_pref, *dest, *dest_pref;
    int pke_len, sour_len, sour_pref_len, dest_len, dest_pref_len, parsed = 0, pke_parsed = 0;
    int has_predicate, dest_parent_times = 0, i, rc;

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
                             LYS_LEAF | LYS_AUGMENT, &src_node);
        if (rc) {
            LOGVAL(LYE_NORESOLV, parent ? LY_VLOG_LYS : LY_VLOG_NONE, parent, "leafref predicate", path-parsed);
            return 0;
        }

        /* destination */
        if ((i = parse_path_key_expr(path_key_expr, &dest_pref, &dest_pref_len, &dest, &dest_len,
                                     &dest_parent_times)) < 1) {
            LOGVAL(LYE_INCHAR, parent ? LY_VLOG_LYS : LY_VLOG_NONE, parent, path_key_expr[-i], path_key_expr-i);
            return -parsed;
        }
        pke_parsed += i;

        for (i = 0, dst_node = parent; i < dest_parent_times; ++i) {
            if (!dst_node) {
                LOGVAL(LYE_NORESOLV, parent ? LY_VLOG_LYS : LY_VLOG_NONE, parent, "leafref predicate", path_key_expr);
                return 0;
            }
            /* path is supposed to be evaluated in data tree, so we have to skip
             * all schema nodes that cannot be instantiated in data tree */
            for (dst_node = lys_parent(dst_node);
                 dst_node && !(dst_node->nodetype & (LYS_CONTAINER | LYS_LIST));
                 dst_node = lys_parent(dst_node));
        }
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
        if (dst_node->nodetype != LYS_LEAF) {
            LOGVAL(LYE_NORESOLV, parent ? LY_VLOG_LYS : LY_VLOG_NONE, parent, "leafref predicate", path-parsed);
            LOGVAL(LYE_SPEC, parent ? LY_VLOG_LYS : LY_VLOG_NONE, parent,
                   "Destination node is not a leaf, but %s.", strnodetype(dst_node->nodetype));
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
    const struct lys_node *node;
    const struct lys_module *mod;
    const char *id, *prefix, *name;
    int pref_len, nam_len, parent_times, has_predicate;
    int i, first_iter, rc;

    first_iter = 1;
    parent_times = 0;
    id = path;

    do {
        if ((i = parse_path_arg(id, &prefix, &pref_len, &name, &nam_len, &parent_times, &has_predicate)) < 1) {
            LOGVAL(LYE_INCHAR, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent, id[-i], &id[-i]);
            return -1;
        }
        id += i;

        if (first_iter) {
            if (parent_times == -1) {
                /* resolve prefix of the module */
                mod = lys_get_import_module(parent->module, NULL, 0, prefix, pref_len);
                /* get start node */
                node = mod ? mod->data : NULL;
                if (!node) {
                    LOGVAL(LYE_NORESOLV, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent,
                           "leafref", path);
                    return EXIT_FAILURE;
                }
            } else if (parent_times > 0) {
                if (parent_tpdf) {
                    /* the path is not allowed to contain relative path since we are in top level typedef */
                    LOGVAL(LYE_NORESOLV, 0, NULL, "leafref", path);
                    return -1;
                }

                /* node is the parent already, skip one ".." */
                for (i = 1, node = parent; i < parent_times; i++) {
                    /* path is supposed to be evaluated in data tree, so we have to skip
                     * all schema nodes that cannot be instantiated in data tree */
                    for (node = lys_parent(node);
                         node && !(node->nodetype & (LYS_CONTAINER | LYS_LIST));
                         node = lys_parent(node));

                    if (!node) {
                        LOGVAL(LYE_NORESOLV, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent,
                               "leafref", path);
                        return EXIT_FAILURE;
                    }

                }
            } else {
                LOGINT;
                return -1;
            }

            first_iter = 0;
        } else {
            /* move down the tree, if possible */
            if (node->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                LOGVAL(LYE_INCHAR, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent, name[0], name);
                return -1;
            }
            node = node->child;
        }

        if (!prefix) {
            prefix = lys_node_module(parent)->name;
        }

        rc = lys_get_sibling(node, prefix, pref_len, name, nam_len, LYS_ANY & ~(LYS_USES | LYS_GROUPING), &node);
        if (rc) {
            LOGVAL(LYE_NORESOLV, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent, "leafref", path);
            return EXIT_FAILURE;
        }

        if (has_predicate) {
            /* we have predicate, so the current result must be list */
            if (node->nodetype != LYS_LIST) {
                LOGVAL(LYE_NORESOLV, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent, "leafref", path);
                return -1;
            }

            i = resolve_path_predicate_schema(id, node, parent);
            if (!i) {
                return EXIT_FAILURE;
            } else if (i < 0) {
                return -1;
            }
            id += i;
        }
    } while (id[0]);

    /* the target must be leaf or leaf-list (in YANG 1.1 only) */
    if ((node->nodetype != LYS_LEAF) && ((lys_node_module(parent)->version != 2) || (node->nodetype != LYS_LEAFLIST))) {
        LOGVAL(LYE_NORESOLV, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent, "leafref", path);
        LOGVAL(LYE_SPEC, parent_tpdf ? LY_VLOG_NONE : LY_VLOG_LYS, parent_tpdf ? NULL : parent,
               "Leafref target \"%s\" is not a leaf%s.", path,
               lys_node_module(parent)->version != 2 ? "" : " nor a leaf-list");
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
    struct unres_data target_match;
    struct ly_ctx *ctx;
    const struct lys_module *mod;
    const char *model, *name, *value;
    char *str;
    int mod_len, nam_len, val_len, i, has_predicate, cur_idx, idx, parsed;
    uint32_t j;

    assert(pred && node_match->count);

    ctx = node_match->node[0]->schema->module->ctx;
    idx = -1;
    parsed = 0;

    do {
        if ((i = parse_predicate(pred, &model, &mod_len, &name, &nam_len, &value, &val_len, &has_predicate)) < 1) {
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
                if (!target_match.node) {
                    LOGMEM;
                    return -1;
                }
                target_match.node[0] = node_match->node[j];
            } else {
                str = strndup(model, mod_len);
                mod = ly_ctx_get_module(ctx, str, NULL);
                free(str);

                if (resolve_data(mod, name, nam_len, node_match->node[j]->child, &target_match)) {
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
 * @brief Resolve instance-identifier in JSON data format. Logs directly.
 *
 * @param[in] data Data node where the path is used
 * @param[in] path Instance-identifier node value.
 *
 * @return Matching node or NULL if no such a node exists. If error occurs, NULL is returned and ly_errno is set.
 */
static struct lyd_node *
resolve_instid(struct lyd_node *data, const char *path)
{
    int i = 0, j;
    struct lyd_node *result = NULL;
    const struct lys_module *mod, *prev_mod;
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

    prev_mod = lyd_node_module(data);

    /* search for the instance node */
    while (path[i]) {
        j = parse_instance_identifier(&path[i], &model, &mod_len, &name, &name_len, &has_predicate);
        if (j <= 0) {
            LOGVAL(LYE_INCHAR, LY_VLOG_LYD, data, path[i-j], &path[i-j]);
            goto error;
        }
        i += j;

        if (model) {
            str = strndup(model, mod_len);
            if (!str) {
                LOGMEM;
                goto error;
            }
            mod = ly_ctx_get_module(ctx, str, NULL);
            free(str);
        } else {
            mod = prev_mod;
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

            j = resolve_predicate(&path[i], &node_match);
            if (j < 1) {
                LOGVAL(LYE_INPRED, LY_VLOG_LYD, data, &path[i-j]);
                goto error;
            }
            i += j;

            if (!node_match.count) {
                /* no instance exists */
                return NULL;
            }
        }

        prev_mod = mod;
    }

    if (!node_match.count) {
        /* no instance exists */
        return NULL;
    } else if (node_match.count > 1) {
        /* instance identifier must resolve to a single node */
        LOGVAL(LYE_TOOMANY, LY_VLOG_LYD, data, path, "data tree");

        goto error;
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
 * @brief Passes config flag down to children, skips nodes without config flags.
 * Does not log.
 *
 * @param[in] node Siblings and their children to have flags changed.
 * @param[in] flags Flags to assign to all the nodes.
 */
static void
inherit_config_flag(struct lys_node *node, int flags)
{
    assert(!(flags ^ (flags & LYS_CONFIG_MASK)));
    LY_TREE_FOR(node, node) {
        if (node->flags & LYS_CONFIG_SET) {
            /* skip nodes with an explicit config value */
            continue;
        }
        if (!(node->nodetype & (LYS_USES | LYS_GROUPING))) {
            node->flags = (node->flags & ~LYS_CONFIG_MASK) | flags;
        }
        if (!(node->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA))) {
            inherit_config_flag(node->child, flags);
        }
    }
}

/**
 * @brief Resolve augment target. Logs directly.
 *
 * @param[in] aug Augment to use.
 * @param[in] siblings Nodes where to start the search in. If set, uses augment, if not, standalone augment.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
resolve_augment(struct lys_node_augment *aug, struct lys_node *siblings)
{
    int rc;
    struct lys_node *sub;

    assert(aug);

    if (!aug->target) {
        /* resolve target node */
        rc = resolve_augment_schema_nodeid(aug->target_name, siblings, (siblings ? NULL : aug->module), (const struct lys_node **)&aug->target);
        if (rc == -1) {
            return -1;
        }
        if (rc > 0) {
            LOGVAL(LYE_INCHAR, LY_VLOG_LYS, aug, aug->target_name[rc - 1], &aug->target_name[rc - 1]);
            return -1;
        }
        if (!aug->target) {
            LOGVAL(LYE_INRESOLV, LY_VLOG_LYS, aug, "augment", aug->target_name);
            return EXIT_FAILURE;
        }

        if (!aug->child) {
            /* nothing to do */
            LOGWRN("Augment \"%s\" without children.", aug->target_name);
            return EXIT_SUCCESS;
        }
    }

    /* check for mandatory nodes - if the target node is in another module
     * the added nodes cannot be mandatory
     */
    if (!aug->parent && (lys_node_module((struct lys_node *)aug) != lys_node_module(aug->target))
            && (rc = lyp_check_mandatory_augment(aug))) {
        return rc;
    }

    /* check augment target type and then augment nodes type */
    if (aug->target->nodetype & (LYS_CONTAINER | LYS_LIST | LYS_CASE | LYS_INPUT | LYS_OUTPUT | LYS_NOTIF)) {
        LY_TREE_FOR(aug->child, sub) {
            if (!(sub->nodetype & (LYS_ANYDATA | LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST | LYS_USES | LYS_CHOICE))) {
                LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, aug, strnodetype(sub->nodetype), "augment");
                LOGVAL(LYE_SPEC, LY_VLOG_LYS, aug, "Cannot augment \"%s\" with a \"%s\".",
                       strnodetype(aug->target->nodetype), strnodetype(sub->nodetype));
                return -1;
            }
        }
    } else if (aug->target->nodetype == LYS_CHOICE) {
        LY_TREE_FOR(aug->child, sub) {
            if (!(sub->nodetype & (LYS_CASE | LYS_ANYDATA | LYS_CONTAINER | LYS_LEAF | LYS_LIST | LYS_LEAFLIST))) {
                LOGVAL(LYE_INCHILDSTMT, LY_VLOG_LYS, aug, strnodetype(sub->nodetype), "augment");
                LOGVAL(LYE_SPEC, LY_VLOG_LYS, aug, "Cannot augment \"%s\" with a \"%s\".",
                       strnodetype(aug->target->nodetype), strnodetype(sub->nodetype));
                return -1;
            }
        }
    } else {
        LOGVAL(LYE_INARG, LY_VLOG_LYS, aug, aug->target_name, "target-node");
        LOGVAL(LYE_SPEC, LY_VLOG_LYS, aug, "Invalid augment target node type \"%s\".", strnodetype(aug->target->nodetype));
        return -1;
    }

    /* inherit config information from actual parent */
    LY_TREE_FOR(aug->child, sub) {
        inherit_config_flag(sub, aug->target->flags & LYS_CONFIG_MASK);
    }

    /* check identifier uniqueness as in lys_node_addchild() */
    LY_TREE_FOR(aug->child, sub) {
        if (lys_check_id(sub, aug->target, NULL)) {
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
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
resolve_uses(struct lys_node_uses *uses, struct unres_schema *unres)
{
    struct ly_ctx *ctx;
    struct lys_node *node = NULL, *next, *iter;
    struct lys_node *node_aux, *parent, *tmp;
    struct lys_refine *rfn;
    struct lys_restr *must, **old_must;
    struct lys_iffeature *iff, **old_iff;
    int i, j, rc, parent_flags;
    uint8_t size, *old_size;
    unsigned int usize, usize1, usize2;

    assert(uses->grp);
    /* HACK just check that the grouping is resolved */
    assert(!uses->grp->nacm);

    if (!uses->grp->child) {
        /* grouping without children, warning was already displayed */
        return EXIT_SUCCESS;
    }

    /* get proper parent (config) flags */
    for (node_aux = lys_parent((struct lys_node *)uses); node_aux && (node_aux->nodetype == LYS_USES); node_aux = lys_parent(node_aux));
    if (node_aux) {
        parent_flags = node_aux->flags & LYS_CONFIG_MASK;
    } else {
        /* default */
        parent_flags = LYS_CONFIG_W;
    }

    /* copy the data nodes from grouping into the uses context */
    LY_TREE_FOR(uses->grp->child, node_aux) {
        node = lys_node_dup(uses->module, (struct lys_node *)uses, node_aux, 0, uses->nacm, unres, 0);
        if (!node) {
            LOGVAL(LYE_INARG, LY_VLOG_LYS, uses, uses->grp->name, "uses");
            LOGVAL(LYE_SPEC, LY_VLOG_LYS, uses, "Copying data from grouping failed.");
            return -1;
        }
        /* test the name of siblings */
        LY_TREE_FOR((uses->parent) ? uses->parent->child : lys_main_module(uses->module)->data, tmp) {
            if (!(tmp->nodetype & (LYS_USES | LYS_GROUPING | LYS_CASE)) && ly_strequal(tmp->name, node_aux->name, 1)) {
                return -1;
            }
        }
    }
    ctx = uses->module->ctx;

    if (parent_flags) {
        assert(uses->child);
        inherit_config_flag(uses->child, parent_flags);
    }

    /* we managed to copy the grouping, the rest must be possible to resolve */

    /* apply refines */
    for (i = 0; i < uses->refine_size; i++) {
        rfn = &uses->refine[i];
        rc = resolve_descendant_schema_nodeid(rfn->target_name, uses->child, LYS_NO_RPC_NOTIF_NODE,
                                              1, 0, (const struct lys_node **)&node);
        if (rc || !node) {
            LOGVAL(LYE_INARG, LY_VLOG_LYS, uses, rfn->target_name, "refine");
            return -1;
        }

        if (rfn->target_type && !(node->nodetype & rfn->target_type)) {
            LOGVAL(LYE_INARG, LY_VLOG_LYS, uses, rfn->target_name, "refine");
            LOGVAL(LYE_SPEC, LY_VLOG_LYS, uses, "Refine substatements not applicable to the target-node.");
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
            for (parent = lys_parent(node); parent && parent->nodetype == LYS_USES; parent = lys_parent(parent));
            if (parent && parent->nodetype != LYS_GROUPING &&
                    ((parent->flags & LYS_CONFIG_MASK) != (rfn->flags & LYS_CONFIG_MASK)) &&
                    (rfn->flags & LYS_CONFIG_W)) {
                /* setting config true under config false is prohibited */
                LOGVAL(LYE_INARG, LY_VLOG_LYS, uses, "config", "refine");
                LOGVAL(LYE_SPEC, LY_VLOG_LYS, uses,
                       "changing config from 'false' to 'true' is prohibited while "
                       "the target's parent is still config 'false'.");
                return -1;
            }

            node->flags &= ~LYS_CONFIG_MASK;
            node->flags |= (rfn->flags & LYS_CONFIG_MASK);

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
                        return -1;
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
                    /* no children */
                    if (iter == node->child) {
                        /* we are done, (START) has no children */
                        break;
                    }
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

        /* default value ... */
        if (rfn->mod.dflt) {
            if (node->nodetype == LYS_LEAF) {
                /* leaf */
                lydict_remove(ctx, ((struct lys_node_leaf *)node)->dflt);
                ((struct lys_node_leaf *)node)->dflt = lydict_insert(ctx, rfn->mod.dflt, 0);
            } else if (node->nodetype == LYS_CHOICE) {
                /* choice */
                rc = resolve_choice_default_schema_nodeid(rfn->mod.dflt, node->child,
                                                          (const struct lys_node **)&((struct lys_node_choice *)node)->dflt);
                if (rc || !((struct lys_node_choice *)node)->dflt) {
                    LOGVAL(LYE_INARG, LY_VLOG_LYS, uses, rfn->mod.dflt, "default");
                    return -1;
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

        /* if-feature in leaf, leaf-list, list, container or anyxml */
        if (rfn->iffeature_size) {
            old_size = &node->iffeature_size;
            old_iff = &node->iffeature;

            size = *old_size + rfn->iffeature_size;
            iff = realloc(*old_iff, size * sizeof *rfn->iffeature);
            if (!iff) {
                LOGMEM;
                return -1;
            }
            for (i = 0, j = *old_size; i < rfn->iffeature_size; i++, j++) {
                resolve_iffeature_getsizes(&rfn->iffeature[i], &usize1, &usize2);
                if (usize1) {
                    /* there is something to duplicate */
                    /* duplicate compiled expression */
                    usize = (usize1 / 4) + (usize1 % 4) ? 1 : 0;
                    iff[j].expr = malloc(usize * sizeof *iff[j].expr);
                    memcpy(iff[j].expr, rfn->iffeature[i].expr, usize * sizeof *iff[j].expr);

                    /* duplicate list of feature pointers */
                    iff[j].features = malloc(usize2 * sizeof *iff[i].features);
                    memcpy(iff[j].features, rfn->iffeature[i].features, usize2 * sizeof *iff[j].features);
                }
            }

            *old_iff = iff;
            *old_size = size;
        }
    }

    /* apply augments */
    for (i = 0; i < uses->augment_size; i++) {
        rc = resolve_augment(&uses->augment[i], uses->child);
        if (rc) {
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

static int
identity_backlink_update(struct lys_ident *der, struct lys_ident *base)
{
    int i;

    assert(der && base);

    base->der = ly_realloc(base->der, (base->der_size + 1) * sizeof *(base->der));
    if (!base->der) {
        LOGMEM;
        return EXIT_FAILURE;
    }
    base->der[base->der_size++] = der;

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
        ident->base[ident->base_size++] = base;
        *ret = base;

        /* maintain backlinks to the derived identities */
        return identity_backlink_update(ident, base) ? -1 : EXIT_SUCCESS;

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
resolve_base_ident(const struct lys_module *module, struct lys_ident *ident, const char *basename, const char* parent,
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
        ret = &type->info.ident.ref;
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
        }
    } else if (rc == EXIT_FAILURE) {
        LOGVAL(LYE_INRESOLV, LY_VLOG_NONE, NULL, parent, basename);
    }

    return rc;
}

/**
 * @brief Resolve JSON data format identityref. Logs directly.
 *
 * @param[in] base Base identity.
 * @param[in] ident_name Identityref name.
 * @param[in] node Node where the identityref is being resolved
 *
 * @return Pointer to the identity resolvent, NULL on error.
 */
struct lys_ident *
resolve_identref(struct lys_ident *base, const char *ident_name, struct lyd_node *node)
{
    const char *mod_name, *name;
    int mod_name_len, rc;
    int i;
    struct lys_ident *der;

    if (!base || !ident_name) {
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

    if (!strcmp(base->name, name) && (!mod_name
            || (!strncmp(base->module->name, mod_name, mod_name_len) && !base->module->name[mod_name_len]))) {
        der = base;
        goto match;
    }

    for (i = 0; i < base->der_size; i++) {
        der = base->der[i]; /* shortcut */
        if (!strcmp(der->name, name) &&
                (!mod_name || (!strncmp(der->module->name, mod_name, mod_name_len) && !der->module->name[mod_name_len]))) {
            /* we have match */
            goto match;
        }

    }

    LOGVAL(LYE_INRESOLV, LY_VLOG_LYD, node, "identityref", ident_name);
    return NULL;

match:
    for (i = 0; i < der->iffeature_size; i++) {
        if (!resolve_iffeature(&der->iffeature[i])) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, der->name, node->schema->name);
            LOGVAL(LYE_SPEC, LY_VLOG_LYD, node, "Identity \"%s\" is disabled by its if-feature condition.",
                   der->name);
            return NULL;
        }
    }
    return der;
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
            LOGVAL(LYE_INRESOLV, LY_VLOG_LYS, uses, "grouping", uses->name);
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
    } else if ((rc == EXIT_FAILURE) && par_grp && !(uses->flags & LYS_USESGRP)) {
        ((struct lys_node_grp *)par_grp)->nacm++;
        uses->flags |= LYS_USESGRP;
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
 *
 * @return EXIT_SUCCESS on pass, EXIT_FAILURE on fail, -1 on error.
 */
static int
resolve_must(struct lyd_node *node)
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
    case LYS_ANYDATA:
        must_size = ((struct lys_node_anydata *)node->schema)->must_size;
        must = ((struct lys_node_anydata *)node->schema)->must;
        break;
    default:
        must_size = 0;
        break;
    }

    for (i = 0; i < must_size; ++i) {
        if (lyxp_eval(must[i].expr, node, LYXP_NODE_ELEM, &set, LYXP_MUST)) {
            return -1;
        }

        lyxp_set_cast(&set, LYXP_SET_BOOLEAN, node, LYXP_MUST);

        if (!set.val.bool) {
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

    return EXIT_SUCCESS;
}

/**
 * @brief Resolve (find) when condition context node. Does not log.
 *
 * @param[in] node Data node, whose conditional definition is being decided.
 * @param[in] schema Schema node with a when condition.
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

    /* find a not schema-only node */
    node_type = LYXP_NODE_ELEM;
    while (schema->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE | LYS_AUGMENT | LYS_INPUT | LYS_OUTPUT)) {
        sparent = lys_parent(schema);
        if (!sparent) {
            /* context node is the document root (fake root in our case) */
            if (schema->flags & LYS_CONFIG_W) {
                node_type = LYXP_NODE_ROOT_CONFIG;
            } else {
                node_type = LYXP_NODE_ROOT_STATE;
            }
            break;
        }
        schema = sparent;
    }

    /* get node depths */
    for (parent = node, data_depth = 0; parent; parent = parent->parent, ++data_depth);
    for (sparent = lys_parent(schema), schema_depth = 1; sparent; sparent = lys_parent(sparent)) {
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
    if ((data_depth > 1) && (schema_depth > 1)) {
        if (node->schema != schema) {
            return -1;
        }
    } else {
        /* special fake root, move it to the beginning of the list */
        while (node->prev->next) {
            node = node->prev;
        }
    }

    *ctx_node = node;
    *ctx_node_type = node_type;
    return EXIT_SUCCESS;
}

int
resolve_applies_must(const struct lyd_node *node)
{
    switch (node->schema->nodetype) {
    case LYS_CONTAINER:
        return ((struct lys_node_container *)node->schema)->must_size;
    case LYS_LEAF:
        return ((struct lys_node_leaf *)node->schema)->must_size;
    case LYS_LEAFLIST:
        return ((struct lys_node_leaflist *)node->schema)->must_size;
    case LYS_LIST:
        return ((struct lys_node_list *)node->schema)->must_size;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        return ((struct lys_node_anydata *)node->schema)->must_size;
    default:
        return 0;
    }
}

int
resolve_applies_when(const struct lyd_node *node)
{
    struct lys_node *parent;

    assert(node);

    if (!(node->schema->nodetype & (LYS_NOTIF | LYS_RPC)) && (((struct lys_node_container *)node->schema)->when)) {
        return 1;
    }

    parent = node->schema;
    goto check_augment;

    while (parent && (parent->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE))) {
        if (((struct lys_node_uses *)parent)->when) {
            return 1;
        }
check_augment:

        if ((parent->parent && (parent->parent->nodetype == LYS_AUGMENT) &&
                (((struct lys_node_augment *)parent->parent)->when))) {

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
 *   0, ly_vecode = LYVE_NOCOND - false "when" statement
 *   1, ly_vecode = LYVE_INWHEN - nodes needed to resolve are conditional and not yet resolved (under another "when")
 */
static int
resolve_when(struct lyd_node *node)
{
    struct lyd_node *ctx_node = NULL;
    struct lys_node *parent;
    struct lyxp_set set;
    enum lyxp_node_type ctx_node_type;
    int rc = 0;

    assert(node);
    memset(&set, 0, sizeof set);

    if (!(node->schema->nodetype & (LYS_NOTIF | LYS_RPC)) && (((struct lys_node_container *)node->schema)->when)) {
        rc = lyxp_eval(((struct lys_node_container *)node->schema)->when->cond, node, LYXP_NODE_ELEM, &set, LYXP_WHEN);
        if (rc) {
            if (rc == 1) {
                LOGVAL(LYE_INWHEN, LY_VLOG_LYD, node, ((struct lys_node_container *)node->schema)->when->cond);
            }
            goto cleanup;
        }

        /* set boolean result of the condition */
        lyxp_set_cast(&set, LYXP_SET_BOOLEAN, node, LYXP_WHEN);
        if (!set.val.bool) {
            ly_vlog_hide(1);
            LOGVAL(LYE_NOWHEN, LY_VLOG_LYD, node, ((struct lys_node_container *)node->schema)->when->cond);
            ly_vlog_hide(0);
            node->when_status |= LYD_WHEN_FALSE;
            goto cleanup;
        }

        /* free xpath set content */
        lyxp_set_cast(&set, LYXP_SET_EMPTY, node, 0);
    }

    parent = node->schema;
    goto check_augment;

    /* check when in every schema node that affects node */
    while (parent && (parent->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE))) {
        if (((struct lys_node_uses *)parent)->when) {
            if (!ctx_node) {
                rc = resolve_when_ctx_node(node, parent, &ctx_node, &ctx_node_type);
                if (rc) {
                    LOGINT;
                    goto cleanup;
                }
            }
            rc = lyxp_eval(((struct lys_node_uses *)parent)->when->cond, ctx_node, ctx_node_type, &set, LYXP_WHEN);
            if (rc) {
                if (rc == 1) {
                    LOGVAL(LYE_INWHEN, LY_VLOG_LYD, node, ((struct lys_node_uses *)parent)->when->cond);
                }
                goto cleanup;
            }

            lyxp_set_cast(&set, LYXP_SET_BOOLEAN, ctx_node, LYXP_WHEN);
            if (!set.val.bool) {
                ly_vlog_hide(1);
                LOGVAL(LYE_NOWHEN, LY_VLOG_LYD, node, ((struct lys_node_uses *)parent)->when->cond);
                ly_vlog_hide(0);
                node->when_status |= LYD_WHEN_FALSE;
                goto cleanup;
            }

            /* free xpath set content */
            lyxp_set_cast(&set, LYXP_SET_EMPTY, ctx_node, 0);
        }

check_augment:
        if ((parent->parent && (parent->parent->nodetype == LYS_AUGMENT) && (((struct lys_node_augment *)parent->parent)->when))) {
            if (!ctx_node) {
                rc = resolve_when_ctx_node(node, parent->parent, &ctx_node, &ctx_node_type);
                if (rc) {
                    LOGINT;
                    goto cleanup;
                }
            }
            rc = lyxp_eval(((struct lys_node_augment *)parent->parent)->when->cond, ctx_node, ctx_node_type, &set, LYXP_WHEN);
            if (rc) {
                if (rc == 1) {
                    LOGVAL(LYE_INWHEN, LY_VLOG_LYD, node, ((struct lys_node_augment *)parent->parent)->when->cond);
                }
                goto cleanup;
            }

            lyxp_set_cast(&set, LYXP_SET_BOOLEAN, ctx_node, LYXP_WHEN);

            if (!set.val.bool) {
                ly_vlog_hide(1);
                LOGVAL(LYE_NOWHEN, LY_VLOG_LYD, node, ((struct lys_node_augment *)parent->parent)->when->cond);
                ly_vlog_hide(0);
                node->when_status |= LYD_WHEN_FALSE;
               goto cleanup;
            }

            /* free xpath set content */
            lyxp_set_cast(&set, LYXP_SET_EMPTY, ctx_node, 0);
        }

        parent = lys_parent(parent);
    }

    node->when_status |= LYD_WHEN_TRUE;

cleanup:

    /* free xpath set content */
    lyxp_set_cast(&set, LYXP_SET_EMPTY, ctx_node ? ctx_node : node, 0);

    return rc;
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

        rc = resolve_path_arg_schema(stype->info.lref.path, node, tpdf_flag,
                                     (const struct lys_node **)&stype->info.lref.target);
        if (!tpdf_flag && !rc) {
            assert(stype->info.lref.target);
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
                if (rc == -1) {
                    yang->type->base = yang->base;
                    lydict_remove(mod->ctx, yang->name);
                    free(yang);
                    stype->der = NULL;
                } else {
                    /* may try again later */
                    stype->der = (struct lys_tpdf *)yang;
                }
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
        } else if (rc == EXIT_FAILURE && stype->base != LY_TYPE_INGRP) {
            /* forward reference - in case the type is in grouping, we have to make the grouping unusable
             * by uses statement until the type is resolved. We do that the same way as uses statements inside
             * grouping - the grouping's nacm member (not used un grouping) is used to increase the number of
             * so far unresolved items (uses and types). The grouping cannot be used unless the nacm value is 0.
             * To remember that the grouping already increased grouping's nacm, the LY_TYPE_INGRP is used as value
             * of the type's base member. */
            for (par_grp = node; par_grp && (par_grp->nodetype != LYS_GROUPING); par_grp = lys_parent(par_grp));
            if (par_grp) {
                ((struct lys_node_grp *)par_grp)->nacm++;
                stype->base = LY_TYPE_INGRP;
            }
        }
        break;
    case UNRES_IFFEAT:
        node = str_snode;
        expr = *((const char **)item);

        rc = resolve_feature(expr, strlen(expr), node, item);
        if (!rc) {
            /* success */
            free((char *)expr);
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
                        if (unres_schema_find(unres, &ref->iffeature[i].features[j - 1], UNRES_IFFEAT) == -1) {
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
        expr = str_snode;
        has_str = 1;
        stype = item;

        rc = check_default(stype, expr, mod);
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
        has_str = 1;
        rc = resolve_list_keys(item, str_snode);
        break;
    case UNRES_LIST_UNIQ:
        unique_info = (struct unres_list_uniq *)item;
        rc = resolve_unique(unique_info->list, unique_info->expr, unique_info->trg_type);
        break;
    case UNRES_AUGMENT:
        rc = resolve_augment(item, NULL);
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
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "if-feature", (char *)item);
        break;
    case UNRES_FEATURE:
        LOGVRB("There are unresolved if-features for \"%s\" feature circular dependency check, it will be attempted later",
               ((struct lys_feature *)item)->name);
        break;
    case UNRES_USES:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "uses", ((struct lys_node_uses *)item)->name);
        break;
    case UNRES_TYPE_DFLT:
        LOGVRB("Resolving %s \"%s\" failed, it will be attempted later.", "type default", (char *)str_node);
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

    LOGVRB("Resolving unresolved schema nodes and their constraints...");
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
             * UNRES_CHOICE_DFLT and UNRES_IDENT */

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
                ly_errno = LY_SUCCESS;
                ly_vecode = LYVE_SUCCESS;
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

            /* free the allocated resources */
            if (unres->type[i] == UNRES_IFFEAT) {
                free(*((char **)unres->item[i]));
            }
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
            resolve_unres_schema_item(mod, unres->item[i], unres->type[i], unres->str_snode[i], unres);
        }
        return -1;
    }

    LOGVRB("All schema nodes and constraints resolved.");
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
    int rc;
    struct lyxml_elem *yin;
    char *path, *msg;

    assert(unres && item && ((type != UNRES_LEAFREF) && (type != UNRES_INSTID) && (type != UNRES_WHEN)
           && (type != UNRES_MUST)));

    ly_vlog_hide(1);
    rc = resolve_unres_schema_item(mod, item, type, snode, unres);
    ly_vlog_hide(0);
    if (rc != EXIT_FAILURE) {
        if (rc == -1 && ly_errno == LY_EVALID) {
            if (ly_log_level >= LY_LLERR) {
                path = strdup(ly_errpath());
                msg = strdup(ly_errmsg());
                LOGERR(LY_EVALID, "%s%s%s%s", msg, path[0] ? " (path: " : "", path[0] ? path : "", path[0] ? ")" : "");
                free(path);
                free(msg);
            }
        }
        if (type == UNRES_LIST_UNIQ) {
            /* free the allocated structure */
            free(item);
        }
        return rc;
    } else {
        /* erase info about validation errors */
        ly_errno = LY_SUCCESS;
        ly_vecode = LYVE_SUCCESS;
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

    assert(item && new_item && ((type != UNRES_LEAFREF) && (type != UNRES_INSTID) && (type != UNRES_WHEN)));

    /* hack for UNRES_LIST_UNIQ, which stores multiple items behind its item */
    if (type == UNRES_LIST_UNIQ) {
        aux_uniq.list = item;
        aux_uniq.expr = ((struct unres_list_uniq *)new_item)->expr;
        item = &aux_uniq;
    }
    i = unres_schema_find(unres, item, type);

    if (i == -1) {
        if (type == UNRES_LIST_UNIQ) {
            free(new_item);
        }
        return EXIT_FAILURE;
    }

    if ((type == UNRES_TYPE_LEAFREF) || (type == UNRES_USES) || (type == UNRES_TYPE_DFLT) ||
            (type == UNRES_IFFEAT) || (type == UNRES_FEATURE) || (type == UNRES_LIST_UNIQ)) {
        if (unres_schema_add_node(mod, unres, new_item, type, unres->str_snode[i]) == -1) {
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
unres_schema_find(struct unres_schema *unres, void *item, enum UNRES_ITEM type)
{
    uint32_t ret = -1, i;
    struct unres_list_uniq *aux_uniq1, *aux_uniq2;

    for (i = unres->count; i > 0; i--) {
        if (unres->type[i - 1] != type) {
            continue;
        }
        if (type != UNRES_LIST_UNIQ) {
            if (unres->item[i - 1] == item) {
                ret = i - 1;
                break;
            }
        } else {
            aux_uniq1 = (struct unres_list_uniq *)unres->item[i - 1];
            aux_uniq2 = (struct unres_list_uniq *)item;
            if ((aux_uniq1->list == aux_uniq2->list) && ly_strequal(aux_uniq1->expr, aux_uniq2->expr, 0)) {
                ret = i - 1;
                break;
            }
        }
    }

    return ret;
}

static void
unres_schema_free_item(struct ly_ctx *ctx, struct unres_schema *unres, uint32_t i)
{
    struct lyxml_elem *yin;
    struct yang_type *yang;

    switch (unres->type[i]) {
    case UNRES_TYPE_DER_TPDF:
    case UNRES_TYPE_DER:
        yin = (struct lyxml_elem *)((struct lys_type *)unres->item[i])->der;
        if (yin->flags & LY_YANG_STRUCTURE_FLAG) {
            yang =(struct yang_type *)yin;
            yang->type->base = yang->base;
            lydict_remove(ctx, yang->name);
            free(yang);
        } else {
            lyxml_free(ctx, yin);
        }
        break;
    case UNRES_IDENT:
    case UNRES_TYPE_IDENTREF:
    case UNRES_TYPE_DFLT:
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
 * @brief Resolve a single unres data item. Logs directly.
 *
 * @param[in] node Data node to resolve.
 * @param[in] type Type of the unresolved item.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
int
resolve_unres_data_item(struct lyd_node *node, enum UNRES_ITEM type)
{
    uint32_t i;
    int rc;
    struct lyd_node_leaf_list *leaf;
    struct lys_node_leaf *sleaf;
    struct lyd_node *parent;
    struct unres_data matches;

    memset(&matches, 0, sizeof matches);
    leaf = (struct lyd_node_leaf_list *)node;
    sleaf = (struct lys_node_leaf *)leaf->schema;

    switch (type) {
    case UNRES_LEAFREF:
        assert(sleaf->type.base == LY_TYPE_LEAFREF);
        /* EXIT_FAILURE return keeps leaf->value.lefref NULL, handled later */
        if (resolve_path_arg_data(node, sleaf->type.info.lref.path, &matches) == -1) {
            return -1;
        }

        /* check that value matches */
        for (i = 0; i < matches.count; ++i) {
            if (ly_strequal(leaf->value_str, ((struct lyd_node_leaf_list *)matches.node[i])->value_str, 1)) {
                leaf->value.leafref = matches.node[i];
                break;
            }
        }

        free(matches.node);

        if (!leaf->value.leafref) {
            /* reference not found */
            LOGVAL(LYE_NOLEAFREF, LY_VLOG_LYD, leaf, sleaf->type.info.lref.path, leaf->value_str);
            return EXIT_FAILURE;
        }
        break;

    case UNRES_INSTID:
        assert((sleaf->type.base == LY_TYPE_INST) || (sleaf->type.base == LY_TYPE_UNION));
        ly_errno = 0;
        leaf->value.instance = resolve_instid(node, leaf->value_str);
        if (!leaf->value.instance) {
            if (ly_errno) {
                return -1;
            } else if (sleaf->type.info.inst.req > -1) {
                LOGVAL(LYE_NOREQINS, LY_VLOG_LYD, leaf, leaf->value_str);
                return EXIT_FAILURE;
            } else {
                LOGVRB("There is no instance of \"%s\", but it is not required.", leaf->value_str);
            }
        }
        break;

    case UNRES_WHEN:
        if ((rc = resolve_when(node))) {
            return rc;
        }
        break;

    case UNRES_MUST:
        if ((rc = resolve_must(node))) {
            return rc;
        }
        break;

    case UNRES_EMPTYCONT:
        do {
            parent = node->parent;
            lyd_free(node);
            node = parent;
        } while (node && (node->schema->nodetype == LYS_CONTAINER) && !node->child
                 && !((struct lys_node_container *)node->schema)->presence);
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
           || (type == UNRES_EMPTYCONT));

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
 * @param[in] unres Unres data structure to use.
 * @param[in,out] root Root node of the data tree. If not NULL, auto-delete is performed on false when condition. If
 * NULL and when condition is false the error is raised.
 * @param[in] options Parer options
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
int
resolve_unres_data(struct unres_data *unres, struct lyd_node **root, int options)
{
    uint32_t i, j, first = 1, resolved = 0, del_items = 0, when_stmt = 0;
    int rc, progress;
    char *msg, *path;
    struct lyd_node *parent;

    assert(unres);
    assert((root && (*root)) || (options & LYD_OPT_NOAUTODEL));

    if (!unres->count) {
        return EXIT_SUCCESS;
    }

    LOGVRB("Resolving unresolved data nodes and their constraints...");
    ly_vlog_hide(1);

    /* when-stmt first */
    ly_errno = LY_SUCCESS;
    ly_vecode = LYVE_SUCCESS;
    do {
        progress = 0;
        for(i = 0; i < unres->count; i++) {
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

            rc = resolve_unres_data_item(unres->node[i], unres->type[i]);
            if (!rc) {
                if (unres->node[i]->when_status & LYD_WHEN_FALSE) {
                    if (!root) {
                        /* false when condition */
                        ly_vlog_hide(0);
                        if (ly_log_level >= LY_LLERR) {
                            path = strdup(ly_errpath());
                            msg = strdup(ly_errmsg());
                            LOGERR(LY_EVALID, "%s%s%s%s", msg,
                                   path[0] ? " (path: " : "", path[0] ? path : "", path[0] ? ")" : "");
                            free(path);
                            free(msg);
                        }
                        return -1;
                    } /* follows else */

                    /* only unlink now, the subtree can contain another nodes stored in the unres list */
                    /* if it has parent non-presence containers that would be empty, we should actually
                     * remove the container
                     */
                    if (!(options & LYD_OPT_KEEPEMPTYCONT)) {
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
                    }

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
                ly_errno = LY_SUCCESS;
                ly_vecode = LYVE_SUCCESS;
                resolved++;
                progress = 1;
            } else if (rc == -1) {
                ly_vlog_hide(0);
                return -1;
            } else {
                /* forward reference, erase ly_errno */
                ly_errno = LY_SUCCESS;
                ly_vecode = LYVE_SUCCESS;
            }
        }
        first = 0;
    } while (progress && resolved < when_stmt);

    /* do we have some unresolved when-stmt? */
    if (when_stmt > resolved) {
        ly_vlog_hide(0);
        if (ly_log_level >= LY_LLERR) {
            path = strdup(ly_errpath());
            msg = strdup(ly_errmsg());
            LOGERR(LY_EVALID, "%s%s%s%s", msg, path[0] ? " (path: " : "", path[0] ? path : "", path[0] ? ")" : "");
            free(path);
            free(msg);
        }
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

    /* rest */
    for (i = 0; i < unres->count; ++i) {
        if (unres->type[i] == UNRES_RESOLVED) {
            continue;
        }

        rc = resolve_unres_data_item(unres->node[i], unres->type[i]);
        if (rc == 0) {
            unres->type[i] = UNRES_RESOLVED;
            resolved++;
        } else if (rc == -1) {
            ly_vlog_hide(0);
            /* print only this last error */
            resolve_unres_data_item(unres->node[i], unres->type[i]);
            return -1;
        }
    }

    ly_vlog_hide(0);
    if (resolved < unres->count) {
        /* try to resolve the unresolved data again, it will not resolve anything, but it will print
         * all the validation errors
         */
        for (i = 0; i < unres->count; ++i) {
            if (unres->type[i] == UNRES_RESOLVED) {
                continue;
            }
            resolve_unres_data_item(unres->node[i], unres->type[i]);
        }
        return -1;
    }

    LOGVRB("All data nodes and constraints resolved.");
    unres->count = 0;
    return EXIT_SUCCESS;
}
