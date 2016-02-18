/**
 * @file xpath.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YANG XPath evaluation functions
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

/* needed by libmath functions isfinite(), isinf(), isnan(), signbit(), ... */
#define _ISOC99_SOURCE

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#include "xpath.h"
#include "libyang.h"
#include "xml_internal.h"
#include "tree_schema.h"
#include "tree_data.h"
#include "context.h"
#include "tree_internal.h"
#include "common.h"
#include "resolve.h"
#include "printer.h"
#include "dict_private.h"

static struct lyd_node *moveto_get_root(struct lyd_node *cur_node, int when_must_eval, enum lyxp_node_type *root_type);
static int reparse_expr(struct lyxp_expr *exp, uint16_t *exp_idx, uint32_t line);
static int eval_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
                     int when_must_eval, uint32_t line);

/**
 * @brief Frees a parsed XPath expression. \p exp should not be used afterwards.
 *
 * @param[in] exp Expression to free.
 */
static void
exp_free(struct lyxp_expr *exp)
{
    uint16_t i;

    free(exp->tokens);
    free(exp->expr_pos);
    free(exp->tok_len);
    for (i = 0; i < exp->used; ++i) {
        free(exp->repeat[i]);
    }
    free(exp->repeat);
    free(exp);
}

/**
 * @brief Print the type of an XPath \p set.
 *
 * @param[in] set Set to use.
 *
 * @return Set type string.
 */
static const char *
print_set_type(struct lyxp_set *set)
{
    switch (set->type) {
    case LYXP_SET_EMPTY:
        return "empty";
    case LYXP_SET_NODE_SET:
        return "node set";
    case LYXP_SET_BOOLEAN:
        return "boolean";
    case LYXP_SET_NUMBER:
        return "number";
    case LYXP_SET_STRING:
        return "string";
    }

    return NULL;
}

/**
 * @brief Print an XPath token \p tok type.
 *
 * @param[in] tok Token to use.
 *
 * @return Token type string.
 */
static const char *
print_token(enum lyxp_token tok)
{
    switch (tok) {
    case LYXP_TOKEN_PAR1:
        return "(";
    case LYXP_TOKEN_PAR2:
        return ")";
    case LYXP_TOKEN_BRACK1:
        return "[";
    case LYXP_TOKEN_BRACK2:
        return "]";
    case LYXP_TOKEN_DOT:
        return ".";
    case LYXP_TOKEN_DDOT:
        return "..";
    case LYXP_TOKEN_AT:
        return "@";
    case LYXP_TOKEN_COMMA:
        return ",";
    case LYXP_TOKEN_NAMETEST:
        return "NameTest";
    case LYXP_TOKEN_NODETYPE:
        return "NodeType";
    case LYXP_TOKEN_FUNCNAME:
        return "FunctionName";
    case LYXP_TOKEN_OPERATOR_LOG:
        return "Operator(Logic)";
    case LYXP_TOKEN_OPERATOR_COMP:
        return "Operator(Comparison)";
    case LYXP_TOKEN_OPERATOR_MATH:
        return "Operator(Math)";
    case LYXP_TOKEN_OPERATOR_UNI:
        return "Operator(Union)";
    case LYXP_TOKEN_OPERATOR_PATH:
        return "Operator(Path)";
    case LYXP_TOKEN_LITERAL:
        return "Literal";
    case LYXP_TOKEN_NUMBER:
        return "Number";
    default:
        LOGINT;
        return "";
    }
}

/**
 * @brief Print the whole expression \p exp to debug output.
 *
 * @param[in] exp Expression to use.
 */
static void
print_expr_struct_debug(struct lyxp_expr *exp)
{
    uint16_t i, j;
    char tmp[128];

    if (!exp) {
        return;
    }

    LOGDBG("XPATH: expression \"%s\":", exp->expr);
    for (i = 0; i < exp->used; ++i) {
        sprintf(tmp, "XPATH:\tToken %s, in expression \"%.*s\"", print_token(exp->tokens[i]), exp->tok_len[i],
               &exp->expr[exp->expr_pos[i]]);
        if (exp->repeat[i]) {
            sprintf(tmp + strlen(tmp), " (repeat %d", exp->repeat[i][0]);
            for (j = 1; exp->repeat[i][j]; ++j) {
                sprintf(tmp + strlen(tmp), ", %d", exp->repeat[i][j]);
            }
            strcat(tmp, ")");
        }
        LOGDBG(tmp);
    }
}

/**
 * @brief Realloc the string \p str.
 *
 * @param[in] needed How much free space is required.
 * @param[in,out] str Pointer to the string to use.
 * @param[in,out] used Used bytes in \p str.
 * @param[in,out] size Allocated bytes in \p str.
 */
static void
cast_string_realloc(uint16_t needed, char **str, uint16_t *used, uint16_t *size)
{
    if (*size - *used < needed) {
        do {
            *size += LYXP_STRING_CAST_SIZE_STEP;
        } while (*size - *used < needed);
        *str = ly_realloc(*str, *size * sizeof(char));
        if (!(*str)) {
            LOGMEM;
        }
    }
}

/**
 * @brief Cast nodes recursively to one string \p str.
 *
 * @param[in] node Node to cast.
 * @param[in] fake_cont Whether to put the data into a "fake" container.
 * @param[in] root_type Type of the XPath root.
 * @param[in] indent Current indent.
 * @param[in,out] str Resulting string.
 * @param[in,out] used Used bytes in \p str.
 * @param[in,out] size Allocated bytes in \p str.
 */
static void
cast_string_recursive(struct lyd_node *node, int fake_cont, enum lyxp_node_type root_type, uint16_t indent, char **str,
                      uint16_t *used, uint16_t *size)
{
    char *buf, *line, *ptr;
    const char *value_str;
    struct lyd_node *child;

    if ((root_type == LYXP_NODE_ROOT_CONFIG) && (node->schema->flags & LYS_CONFIG_R)) {
        return;
    }

    if ((root_type == LYXP_NODE_ROOT_OUTPUT) && (node->schema->parent->nodetype == LYS_INPUT)) {
        return;
    }

    if (fake_cont) {
        cast_string_realloc(1, str, used, size);
        strcpy(*str + (*used - 1), "\n");
        ++(*used);

        ++indent;
    }

    switch (node->schema->nodetype) {
    case LYS_CONTAINER:
    case LYS_LIST:
    case LYS_RPC:
    case LYS_NOTIF:
        cast_string_realloc(1, str, used, size);
        strcpy(*str + (*used - 1), "\n");
        ++(*used);

        LY_TREE_FOR(node->child, child) {
            cast_string_recursive(child, 0, root_type, indent + 1, str, used, size);
        }

        break;

    case LYS_LEAF:
    case LYS_LEAFLIST:
        value_str = ((struct lyd_node_leaf_list *)node)->value_str;
        if (!value_str) {
            value_str = "";
        }

        /* print indent */
        cast_string_realloc(indent * 2 + strlen(value_str) + 1, str, used, size);
        memset(*str + (*used - 1), ' ', indent * 2);
        *used += indent * 2;

        /* print value */
        if (*used == 1) {
            sprintf(*str + (*used - 1), "%s", value_str);
            *used += strlen(value_str);
        } else {
            sprintf(*str + (*used - 1), "%s\n", value_str);
            *used += strlen(value_str) + 1;
        }

        break;

    case LYS_ANYXML:
        lyxml_print_mem(&buf, ((struct lyd_node_anyxml *)node)->value->child, 0);
        line = strtok_r(buf, "\n", &ptr);
        do {
            cast_string_realloc(indent * 2 + strlen(line) + 1, str, used, size);
            memset(*str + (*used - 1), ' ', indent * 2);
            *used += indent * 2;

            strcpy(*str + (*used - 1), line);
            *used += strlen(line);

            strcpy(*str + (*used - 1), "\n");
            *used += 1;
        } while ((line = strtok_r(NULL, "\n", &ptr)));

        free(buf);
        break;

    default:
        LOGINT;
        break;
    }

    if (fake_cont) {
        cast_string_realloc(1, str, used, size);
        strcpy(*str + (*used - 1), "\n");
        ++(*used);

        --indent;
    }
}

/**
 * @brief Cast an element into a string.
 *
 * @param[in] node Node to cast.
 * @param[in] fake_cont Whether to put the data into a "fake" container.
 * @param[in] root_type Type of the XPath root.
 * @param[in] ctx libyang context to use.
 *
 * @return Element cast to string in the dictionary.
 */
static const char *
cast_string_elem(struct lyd_node *node, int fake_cont, enum lyxp_node_type root_type, struct ly_ctx *ctx)
{
    char *str;
    uint16_t used, size;

    str = malloc(LYXP_STRING_CAST_SIZE_START * sizeof(char));
    if (!str) {
        LOGMEM;
        return NULL;
    }
    str[0] = '\0';
    used = 1;
    size = LYXP_STRING_CAST_SIZE_START;

    cast_string_recursive(node, fake_cont, root_type, 0, &str, &used, &size);

    if (size > used) {
        str = ly_realloc(str, used * sizeof(char));
        if (!str) {
            LOGMEM;
            return NULL;
        }
    }
    return lydict_insert_zc(ctx, str);
}

/**
 * @brief Cast a LYXP_SET_NODE_SET set into a string.
 *        Context position aware.
 *
 * @param[in] set Set to cast.
 * @param[in] cur_node Original context node.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return Cast string in the dictionary.
 */
static const char *
cast_node_set_to_string(struct lyxp_set *set, struct lyd_node *cur_node, int when_must_eval)
{
    uint16_t pos;
    struct ly_ctx *ctx;
    enum lyxp_node_type root_type;

    ctx = cur_node->schema->module->ctx;
    moveto_get_root(cur_node, when_must_eval, &root_type);

    if (set->pos) {
        pos = set->pos - 1;
    } else {
        pos = 0;
    }
    switch (set->node_type[pos]) {
    case LYXP_NODE_ROOT_CONFIG:
    case LYXP_NODE_ROOT_STATE:
    case LYXP_NODE_ROOT_NOTIF:
    case LYXP_NODE_ROOT_RPC:
        return cast_string_elem(set->value.nodes[pos], 1, root_type, ctx);
    case LYXP_NODE_ROOT_OUTPUT:
        return cast_string_elem(set->value.nodes[pos]->child, 1, root_type, ctx);
    case LYXP_NODE_ELEM:
    case LYXP_NODE_TEXT:
        return cast_string_elem(set->value.nodes[pos], 0, root_type, ctx);
    case LYXP_NODE_ATTR:
        return lydict_insert(ctx, set->value.attrs[pos]->value, 0);
    }

    LOGINT;
    return NULL;
}

/**
 * @brief Cast a string into an XPath number.
 *
 * @param[in] str String to use.
 *
 * @return Cast number.
 */
static long double
cast_string_to_number(const char *str)
{
    long double num;
    char *ptr;

    errno = 0;
    num = strtold(str, &ptr);
    if (errno || *ptr) {
        num = NAN;
    }
    return num;
}

/*
 * lyxp_set manipulation functions
 */

/**
 * @brief Create a deep copy of a \p set.
 *
 * @param[in] set Set to copy.
 * @param[in] ctx libyang context to use.
 *
 * @return Copy of \p set.
 */
static struct lyxp_set *
set_copy(struct lyxp_set *set, struct ly_ctx *ctx)
{
    struct lyxp_set *ret;

    if (!set) {
        return NULL;
    }

    ret = malloc(sizeof *ret);
    if (!ret) {
        LOGMEM;
        return NULL;
    }
    if (set->type == LYXP_SET_NODE_SET) {
        ret->type = set->type;
        ret->value.nodes = malloc(set->used * sizeof *ret->value.nodes);
        if (!ret->value.nodes) {
            LOGMEM;
            free(ret);
            return NULL;
        }
        memcpy(ret->value.nodes, set->value.nodes, set->used * sizeof *ret->value.nodes);
        ret->node_type = malloc(set->used * sizeof *ret->node_type);
        if (!ret->node_type) {
            LOGMEM;
            free(ret->value.nodes);
            free(ret);
            return NULL;
        }
        memcpy(ret->node_type, set->node_type, set->used * sizeof *ret->node_type);
        ret->used = ret->size = set->used;
    } else {
       memcpy(ret, set, sizeof *ret);
       if (set->type == LYXP_SET_STRING) {
           ret->value.str = lydict_insert(ctx, set->value.str, 0);
       }
    }

    return ret;
}

/**
 * @brief Fill XPath set with a string. Any current data are disposed of.
 *
 * @param[in] set Set to fill.
 * @param[in] string String to fill into \p set.
 * @param[in] str_len Length of \p string. 0 is a valid value!
 * @param[in] ctx libyang context to use.
 */
static void
set_fill_string(struct lyxp_set *set, const char *string, uint16_t str_len, struct ly_ctx *ctx)
{
    if (set->type == LYXP_SET_NODE_SET) {
        free(set->value.nodes);
        free(set->node_type);
    } else if (set->type == LYXP_SET_STRING) {
        lydict_remove(ctx, set->value.str);
    }

    set->type = LYXP_SET_STRING;
    if ((str_len == 0) && (string[0] != '\0')) {
        string = "";
    }
    set->value.str = lydict_insert(ctx, string, str_len);
}

/**
 * @brief Fill XPath set with a number. Any current data are disposed of.
 *
 * @param[in] set Set to fill.
 * @param[in] number Number to fill into \p set.
 * @param[in] ctx libyang context to use.
 */
static void
set_fill_number(struct lyxp_set *set, long double number, struct ly_ctx *ctx)
{
    if (set->type == LYXP_SET_NODE_SET) {
        free(set->value.nodes);
        free(set->node_type);
    } else if (set->type == LYXP_SET_STRING) {
        lydict_remove(ctx, set->value.str);
    }

    set->type = LYXP_SET_NUMBER;
    set->value.num = number;
}

/**
 * @brief Fill XPath set with a boolean. Any current data are disposed of.
 *
 * @param[in] set Set to fill.
 * @param[in] boolean Boolean to fill into \p set.
 * @param[in] ctx libyang context to use.
 */
static void
set_fill_boolean(struct lyxp_set *set, int boolean, struct ly_ctx *ctx)
{
    if (set->type == LYXP_SET_NODE_SET) {
        free(set->value.nodes);
        free(set->node_type);
    } else if (set->type == LYXP_SET_STRING) {
        lydict_remove(ctx, set->value.str);
    }

    set->type = LYXP_SET_BOOLEAN;
    set->value.bool = boolean;
}

/**
 * @brief Fill XPath set with the value from another set (deep assign).
 *        Any current data are disposed of.
 *
 * @param[in] set Set to fill.
 * @param[in] src Source set to copy into \p set.
 * @param[in] ctx libyang context to use.
 */
static void
set_fill_set(struct lyxp_set *set, struct lyxp_set *src, struct ly_ctx *ctx)
{
    if (!set || !src) {
        return;
    }

    if (src->type == LYXP_SET_BOOLEAN) {
        set_fill_boolean(set, src->value.bool, ctx);
    } else if (src->type ==  LYXP_SET_NUMBER) {
        set_fill_number(set, src->value.num, ctx);
    } else if (src->type == LYXP_SET_STRING) {
        set_fill_string(set, src->value.str, strlen(src->value.str), ctx);
    } else {
        if (set->type == LYXP_SET_NODE_SET) {
            free(set->value.nodes);
            free(set->node_type);
        } else if (set->type == LYXP_SET_STRING) {
            lydict_remove(ctx, set->value.str);
        }

        if (src->type == LYXP_SET_EMPTY) {
            set->type = LYXP_SET_EMPTY;
        } else {
            assert(src->type == LYXP_SET_NODE_SET);

            set->type = LYXP_SET_NODE_SET;
            set->used = src->used;
            set->size = src->size;
            set->pos = src->pos;

            set->value.nodes = malloc(set->used * sizeof *set->value.nodes);
            if (!set->value.nodes) {
                LOGMEM;
                memset(set, 0, sizeof *set);
                return;
            }
            set->node_type = malloc(set->used * sizeof *set->node_type);
            if (!set->node_type) {
                LOGMEM;
                free(set->value.nodes);
                memset(set, 0, sizeof *set);
                return;
            }

            memcpy(set->value.nodes, src->value.nodes, src->used * sizeof *src->value.nodes);
            memcpy(set->node_type, src->node_type, src->used * sizeof *src->node_type);
        }
    }


}

/**
 * @brief Remove a node from a set. Removing last node changes
 *        \p set into LYXP_SET_EMPTY. Context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] idx Index from \p set of the node to be removed.
 */
static void
set_remove_node(struct lyxp_set *set, uint16_t idx)
{
    assert(set && (set->type == LYXP_SET_NODE_SET));
    assert(idx < set->used);

    --set->used;
    if (set->used && (set->pos != idx + 1)) {
        memmove(&set->value.nodes[idx], &set->value.nodes[idx + 1],
                (set->used - idx) * sizeof *set->value.nodes);
        memmove(&set->node_type[idx], &set->node_type[idx + 1],
                (set->used - idx) * sizeof *set->node_type);
        if (idx + 1 < set->pos) {
            --set->pos;
        }
    } else {
        free(set->value.nodes);
        free(set->node_type);
        /* this changes it to LYXP_SET_EMPTY */
        memset(set, 0, sizeof *set);
    }
}

/**
 * @brief Check for duplicates in a node set.
 *
 * @param[in] set Set to check.
 * @param[in] node Node to look for in \p set.
 * @param[in] node_type Type of \p node.
 * @param[in] skip_idx Index from \p set to skip.
 *
 * @return Position of the duplicate or -1 if there is none.
 */
static int
set_dup_node_check(struct lyxp_set *set, void *node, enum lyxp_node_type node_type, int skip_idx)
{
    uint16_t i;

    for (i = 0; i < set->used; ++i) {
        if ((skip_idx > -1) && (i == (unsigned)skip_idx)) {
            continue;
        }

        if ((set->value.nodes[i] == node) && (set->node_type[i] == node_type)) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Remove duplicate entries in a sorted node set.
 *
 * @param[in] set Sorted set to check.
 *
 * @return EXIT_SUCCESS if no duplicates were found,
 *         EXIT_FAILURE otherwise.
 */
static int
set_sorted_dup_node_clean(struct lyxp_set *set)
{
    uint16_t i = 0;
    int ret = EXIT_SUCCESS;

    while (i < set->used - 1) {
        if ((set->value.nodes[i] == set->value.nodes[i + 1])
                && (set->node_type[i] == set->node_type[i + 1])) {
            set_remove_node(set, i + 1);
        ret = EXIT_FAILURE;
        } else {
            ++i;
        }
    }

    return ret;
}

/**
 * @brief Insert a node into a set. Context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] node Node to insert to \p set.
 * @param[in] node_type Node type of \p node.
 * @param[in] idx Index in \p set to insert into.
 */
static void
set_insert_node(struct lyxp_set *set, void *node, enum lyxp_node_type node_type, uint16_t idx)
{
    assert(set && ((set->type == LYXP_SET_NODE_SET) || (set->type == LYXP_SET_EMPTY)));

    if (set->type == LYXP_SET_EMPTY) {
        /* first item */
        if (idx) {
            /* no real harm done, but it is a bug */
            LOGINT;
        }
        set->value.nodes = calloc(LYXP_SET_SIZE_START, sizeof *set->value.nodes);
        if (!set->value.nodes) {
            LOGMEM;
            return;
        }
        set->value.nodes[0] = node;
        set->node_type = malloc(LYXP_SET_SIZE_START * sizeof *set->node_type);
        if (!set->node_type) {
            LOGMEM;
            free(set->value.nodes);
            return;
        }
        set->node_type[0] = node_type;
        set->type = LYXP_SET_NODE_SET;
        set->used = 1;
        set->size = LYXP_SET_SIZE_START;
        set->pos = 0;
    } else {
        /* not an empty set */
        if (set->used == set->size) {

            /* set is full */
            set->value.nodes = ly_realloc(set->value.nodes, (set->size + LYXP_SET_SIZE_STEP) * sizeof *set->value.nodes);
            if (!set->value.nodes) {
                LOGMEM;
                memset(set, 0, sizeof *set);
                return;
            }
            set->node_type = ly_realloc(set->node_type, (set->size + LYXP_SET_SIZE_STEP) * sizeof *set->node_type);
            if (!set->node_type) {
                LOGMEM;
                free(set->value.nodes);
                memset(set, 0, sizeof *set);
                return;
            }
            set->size += LYXP_SET_SIZE_STEP;
        }

        if (idx > set->used) {
            LOGINT;
            idx = set->used;
        }

        /* make space for the new node */
        if (idx < set->used) {
            memmove(&set->value.nodes[idx + 1], &set->value.nodes[idx], (set->used - idx) * sizeof *set->value.nodes);
            memmove(&set->node_type[idx + 1], &set->node_type[idx], (set->used - idx) * sizeof *set->node_type);
            if (set->pos >= idx + 1) {
                ++set->pos;
            }
        }

        /* finally assign the value */
        set->value.nodes[idx] = node;
        set->node_type[idx] = node_type;
        ++set->used;
    }
}

/**
 * @brief Print XPath set content to debug output.
 *
 * @param[in] set Set to print.
 */
static void
print_set_debug(struct lyxp_set *set)
{
    uint16_t i;
    char *str_num;

    switch (set->type) {
    case LYXP_SET_NODE_SET:
        LOGDBG("XPATH: set NODE SET:");
        for (i = 0; i < set->used; ++i) {
            switch (set->node_type[i]) {
            case LYXP_NODE_ROOT_CONFIG:
                LOGDBG("XPATH:\t%d: ROOT CONFIG", i + 1);
                break;
            case LYXP_NODE_ROOT_STATE:
                LOGDBG("XPATH:\t%d: ROOT STATE", i + 1);
                break;
            case LYXP_NODE_ROOT_NOTIF:
                LOGDBG("XPATH:\t%d: ROOT NOTIF %s", i + 1, set->value.nodes[i]->schema->name);
                break;
            case LYXP_NODE_ROOT_RPC:
                LOGDBG("XPATH:\t%d: ROOT RPC %s", i + 1, set->value.nodes[i]->schema->name);
                break;
            case LYXP_NODE_ROOT_OUTPUT:
                LOGDBG("XPATH:\t%d: ROOT OUTPUT %s", i + 1, set->value.nodes[i]->schema->name);
                break;
            case LYXP_NODE_ELEM:
                if ((set->value.nodes[i]->schema->nodetype == LYS_LIST)
                        && (set->value.nodes[i]->child->schema->nodetype == LYS_LEAF)) {
                    LOGDBG("XPATH:\t%d: ELEM %s (1st child val: %s)", i + 1, set->value.nodes[i]->schema->name,
                           ((struct lyd_node_leaf_list *)set->value.nodes[i]->child)->value_str);
                } else if (set->value.nodes[i]->schema->nodetype == LYS_LEAFLIST) {
                    LOGDBG("XPATH:\t%d: ELEM %s (val: %s)", i + 1, set->value.nodes[i]->schema->name,
                           ((struct lyd_node_leaf_list *)set->value.nodes[i])->value_str);
                } else {
                    LOGDBG("XPATH:\t%d: ELEM %s", i + 1, set->value.nodes[i]->schema->name);
                }
                break;
            case LYXP_NODE_TEXT:
                if (set->value.nodes[i]->schema->nodetype == LYS_ANYXML) {
                    LOGDBG("XPATH:\t%d: TEXT <anyxml>", i + 1);
                } else {
                    LOGDBG("XPATH:\t%d: TEXT %s", i + 1, ((struct lyd_node_leaf_list *)set->value.nodes[i])->value_str);
                }
                break;
            case LYXP_NODE_ATTR:
                LOGDBG("XPATH:\t%d: ATTR %s = %s", i + 1, set->value.attrs[i]->name, set->value.attrs[i]->value);
                break;
            }
        }
        break;

    case LYXP_SET_EMPTY:
        LOGDBG("XPATH: set EMPTY");
        break;

    case LYXP_SET_BOOLEAN:
        LOGDBG("XPATH: set BOOLEAN");
        LOGDBG("XPATH:\t%s", (set->value.bool ? "true" : "false"));
        break;

    case LYXP_SET_STRING:
        LOGDBG("XPATH: set STRING");
        LOGDBG("XPATH:\t%s", set->value.str);
        break;

    case LYXP_SET_NUMBER:
        LOGDBG("XPATH: set NUMBER");

        if (isnan(set->value.num)) {
            str_num = strdup("NaN");
        } else if ((set->value.num == 0) || (set->value.num == -0)) {
            str_num = strdup("0");
        } else if (isinf(set->value.num) && !signbit(set->value.num)) {
            str_num = strdup("Infinity");
        } else if (isinf(set->value.num) && signbit(set->value.num)) {
            str_num = strdup("-Infinity");
        } else if ((long long)set->value.num == set->value.num) {
            asprintf(&str_num, "%lld", (long long)set->value.num);
        } else {
            asprintf(&str_num, "%03.1Lf", set->value.num);
        }

        if (!str_num) {
            LOGMEM;
            return;
        }

        LOGDBG("XPATH:\t%s", str_num);
        free(str_num);
    }
}

/**
 * @brief Get unique \p node position in the data.
 *
 * @param[in] node Node to find.
 * @param[in] node_type Node type of \p node.
 * @param[in] root Root node.
 * @param[in] root_type Type of the XPath \p root node.
 *
 * @return Node position.
 */
static uint16_t
get_node_pos(struct lyd_node *node, enum lyxp_node_type node_type, struct lyd_node *root, enum lyxp_node_type root_type)
{
    struct lyd_node *next, *elem;
    uint16_t pos = 1;

    if ((node_type == LYXP_NODE_ROOT_CONFIG) || (node_type == LYXP_NODE_ROOT_STATE) || (node_type == LYXP_NODE_ROOT_NOTIF)
            || (node_type == LYXP_NODE_ROOT_RPC) || (node_type == LYXP_NODE_ROOT_OUTPUT)) {
        return 0;
    }

    if (root_type == LYXP_NODE_ROOT_OUTPUT) {
        root = root->child;
    }

    /* TREE DFS */
    for (elem = next = root; elem; elem = next) {
        if ((root_type == LYXP_NODE_ROOT_CONFIG) && (elem->schema->flags & LYS_CONFIG_R)) {
            goto skip_children;
        }
        if ((root_type == LYXP_NODE_ROOT_OUTPUT) && (elem->schema->parent->nodetype == LYS_INPUT)) {
            goto skip_children;
        }

        if (elem == node) {
            break;
        }
        ++pos;

        /* TREE DFS END */
        /* select element for the next run - children first */
        next = elem->child;
        /* child exception for lyd_node_leaf and lyd_node_leaflist, but not the root */
        if (elem->schema && (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML))) {
            next = NULL;
        }
        if (!next) {
skip_children:
            /* no children, so try siblings */
            next = elem->next;
        }
        while (!next) {
            /* no siblings, go back through parents */
            if (elem->parent == root->parent) {
                /* we are done, no next element to process */
                break;
            }
            /* parent is already processed, go to its sibling */
            elem = elem->parent;
            next = elem->next;
        }
    }

    if (!elem) {
        LOGINT;
    }

    return pos;
}

/**
 * @brief Get unique \p attr position in the parent attributes.
 *
 * @param[in] attr Attr to use.
 * @param[in] parent Parent of \p attr.
 *
 * @return Attribute position.
 */
static uint16_t
get_attr_pos(struct lyd_attr *attr, struct lyd_node *parent)
{
    uint16_t pos = 0;
    struct lyd_attr *attr2;

    for (attr2 = parent->attr; attr2 && (attr2 != attr); attr2 = attr2->next) {
        ++pos;
    }

    assert(attr2);
    return pos;
}

/**
 * @brief Compare 2 nodes in respect to XPath document order.
 *
 * @param[in] first_node_pos 1st node position.
 * @param[in] first_attr_pos 1st attribute node position.
 * @param[in] second_node_pos 2nd node position.
 * @param[in] second_attr_pos 2nd attribute node position.
 * @param[in] first_idx Index of the 1st node in \p set.
 * @param[in] second_idx Index of the 2nd node in \p set.
 * @param[in] set Set with the nodes.
 *
 * @return If 1st > 2nd returns 1, 1st == 2nd returns 0, and 1st < 2nd returns -1.
 */
static int
set_sort_compare(uint16_t first_node_pos, uint16_t first_attr_pos, uint16_t second_node_pos, uint16_t second_attr_pos,
                 uint16_t first_idx, uint16_t second_idx, struct lyxp_set *set)
{
    if (first_node_pos < second_node_pos) {
        return -1;
    }

    if (first_node_pos > second_node_pos) {
        return 1;
    }

    /* node positions are equal, the fun case */

    /* 1st ELEM - == - 2nd TEXT, 1st TEXT - == - 2nd ELEM */
    /* special case since text nodes are actually saved as their parents */
    if ((set->value.nodes[first_idx] == set->value.nodes[second_idx])
            && (set->node_type[first_idx] != set->node_type[second_idx])) {
        if (set->node_type[first_idx] == LYXP_NODE_ELEM) {
            assert(set->node_type[second_idx] == LYXP_NODE_TEXT);
            return -1;
        } else {
            assert((set->node_type[first_idx] == LYXP_NODE_TEXT) && (set->node_type[second_idx] == LYXP_NODE_ELEM));
            return 1;
        }
    }

    /* 1st ROOT - 2nd ROOT, 1st ELEM - 2nd ELEM, 1st TEXT - 2nd TEXT, 1st ATTR - =pos= - 2nd ATTR */
    /* check for duplicates */
    if (set->value.nodes[first_idx] == set->value.nodes[second_idx]) {
        assert((set->node_type[first_idx] == set->node_type[second_idx])
                && ((set->node_type[first_idx] != LYXP_NODE_ATTR) || (first_attr_pos == second_attr_pos)));
        return 0;
    }

    /* 1st ELEM - 2nd TEXT, 1st ELEM - any pos - 2nd ATTR */
    /* elem is always first, 2nd node is after it */
    if (set->node_type[first_idx] == LYXP_NODE_ELEM) {
        assert(set->node_type[second_idx] != LYXP_NODE_ELEM);
        return -1;
    }

    /* 1st TEXT - 2nd ELEM, 1st TEXT - any pos - 2nd ATTR, 1st ATTR - any pos - 2nd ELEM, 1st ATTR - >pos> - 2nd ATTR */
    /* 2nd is before 1st */
    if (((set->node_type[first_idx] == LYXP_NODE_TEXT)
            && ((set->node_type[second_idx] == LYXP_NODE_ELEM) || (set->node_type[second_idx] == LYXP_NODE_ATTR)))
            || ((set->node_type[first_idx] == LYXP_NODE_ATTR) && (set->node_type[second_idx] == LYXP_NODE_ELEM))
            || (((set->node_type[first_idx] == LYXP_NODE_ATTR) && (set->node_type[second_idx] == LYXP_NODE_ATTR))
            && (first_attr_pos > second_attr_pos))) {
        return 1;
    }

    /* 1st ATTR - any pos - 2nd TEXT, 1st ATTR <pos< - 2nd ATTR */
    /* 2nd is after 1st */
    return -1;
}

/**
 * @brief Bubble sort \p set into XPath document order.
 *        Context position aware.
 *
 * @param[in] set Set to sort.
 * @param[in] cur_node Original context node.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return How many times the whole set was traversed.
 */
static int
set_sort(struct lyxp_set *set, struct lyd_node *cur_node, int when_must_eval)
{
    uint16_t i, j, node_pos1 = 0, node_pos2 = 0, attr_pos1 = 0, attr_pos2 = 0;
    int ret = 0, cmp, inverted, change;
    struct lyd_node *tmp_node, *root = NULL;
    enum lyxp_node_type tmp_type, root_type;

    if ((set->type != LYXP_SET_NODE_SET) || (set->used == 1)) {
        return ret;
    }

    /* get root */
    root = moveto_get_root(cur_node, when_must_eval, &root_type);

    LOGDBG("XPATH: SORT BEGIN");
    print_set_debug(set);

    for (i = 0; i < set->used; ++i) {
        inverted = 0;
        change = 0;

        /* first node position */
        if (set->node_type[0] == LYXP_NODE_ATTR) {
            tmp_node = lyd_attr_parent(root, set->value.attrs[0]);
            if (!tmp_node) {
                LOGINT;
                return -1;
            }
            node_pos1 = get_node_pos(tmp_node, set->node_type[0], root, root_type);
            attr_pos1 = get_attr_pos(set->value.attrs[0], tmp_node);
        } else {
            node_pos1 = get_node_pos(set->value.nodes[0], set->node_type[0], root, root_type);
        }

        for (j = 1; j < set->used - i; ++j) {
            /* another node position */
            if (set->node_type[j] == LYXP_NODE_ATTR) {
                tmp_node = lyd_attr_parent(root, set->value.attrs[j]);
                if (!tmp_node) {
                    LOGINT;
                    return -1;
                }

                if (inverted) {
                    node_pos1 = get_node_pos(tmp_node, set->node_type[j], root, root_type);
                    attr_pos1 = get_attr_pos(set->value.attrs[j], tmp_node);
                } else {
                    node_pos2 = get_node_pos(tmp_node, set->node_type[j], root, root_type);
                    attr_pos2 = get_attr_pos(set->value.attrs[j], tmp_node);
                }
            } else {
                if (inverted) {
                    node_pos1 = get_node_pos(set->value.nodes[j], set->node_type[j], root, root_type);
                } else {
                    node_pos2 = get_node_pos(set->value.nodes[j], set->node_type[j], root, root_type);
                }
            }

            /* compare node positions */
            if (inverted) {
                cmp = set_sort_compare(node_pos1, attr_pos1, node_pos2, attr_pos2, j, j - 1, set);
            } else {
                cmp = set_sort_compare(node_pos1, attr_pos1, node_pos2, attr_pos2, j - 1, j, set);
            }

            /* swap if needed */
            if ((inverted && (cmp < 0)) || (!inverted && (cmp > 0))) {
                change = 1;

                tmp_node = set->value.nodes[j - 1];
                tmp_type = set->node_type[j - 1];

                set->value.nodes[j - 1] = set->value.nodes[j];
                set->node_type[j - 1] = set->node_type[j];

                set->value.nodes[j] = tmp_node;
                set->node_type[j] = tmp_type;

                /* pos == index + 1 */
                if (set->pos == j + 1) {
                    set->pos = j;
                } else if (set->pos == j) {
                    set->pos = j + 1;
                }
            } else {
                /* whether node_pos1 should be smaller than node_pos2 or the other way around */
                inverted = !inverted;
            }
        }

        ++ret;

        if (!change) {
            break;
        }
    }

    LOGDBG("XPATH: SORT END %d", ret);
    print_set_debug(set);

    return ret;
}

/*
 * (re)parse functions
 *
 * Parse functions parse the expression into
 * tokens (syntactic analysis).
 *
 * Reparse functions perform semantic analysis
 * (do not save the result, just a check) of
 * the expression and fill repeat indices.
 */

/**
 * @brief Add \p token into the expression \p exp.
 *
 * @param[in] exp Expression to use.
 * @param[in] token Token to add.
 * @param[in] expr_pos Token position in the XPath expression.
 * @param[in] tok_len Token length in the XPath expression.
 */
static void
exp_add_token(struct lyxp_expr *exp, enum lyxp_token token, uint16_t expr_pos, uint16_t tok_len)
{
    if (exp->used == exp->size) {
        exp->size += LYXP_EXPR_SIZE_STEP;
        exp->tokens = ly_realloc(exp->tokens, exp->size * sizeof *exp->tokens);
        if (!exp->tokens) {
            LOGMEM;
            return;
        }
        exp->expr_pos = ly_realloc(exp->expr_pos, exp->size * sizeof *exp->expr_pos);
        if (!exp->expr_pos) {
            LOGMEM;
            return;
        }
        exp->tok_len = ly_realloc(exp->tok_len, exp->size * sizeof *exp->tok_len);
        if (!exp->tok_len) {
            LOGMEM;
            return;
        }
    }

    exp->tokens[exp->used] = token;
    exp->expr_pos[exp->used] = expr_pos;
    exp->tok_len[exp->used] = tok_len;
    ++exp->used;
}

/**
 * @brief Look at the next token and check its kind.
 *
 * @param[in] exp Expression to use.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] want_tok Expected token.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS if the current token matches the expected one,
 *         -1 otherwise.
 */
static int
exp_check_token(struct lyxp_expr *exp, uint16_t exp_idx, enum lyxp_token want_tok, uint32_t line)
{
    if (exp->used == exp_idx) {
        LOGVAL(LYE_XPATH_EOF, line, 0, NULL);
        return -1;
    }

    if (want_tok && (exp->tokens[exp_idx] != want_tok)) {
        LOGVAL(LYE_XPATH_INTOK, line, 0, NULL,
               print_token(exp->tokens[exp_idx]), &exp->expr[exp->expr_pos[exp_idx]]);
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Stack operation peek on the repeat array.
 *
 * @param[in] exp Expression to use.
 * @param[in] exp_idx Position in the expression \p exp.
 *
 * @return Last repeat or 0.
 */
static uint16_t
exp_repeat_peek(struct lyxp_expr *exp, uint16_t exp_idx)
{
    uint16_t i;

    if (!exp->repeat[exp_idx]) {
        return 0;
    }

    for (i = 0; exp->repeat[exp_idx][i + 1]; ++i);

    return exp->repeat[exp_idx][i];
}

/**
 * @brief Stack operation pop on the repeat array.
 *
 * @param[in] exp Expression to use.
 * @param[in] exp_idx Position in the expression \p exp.
 */
static void
exp_repeat_pop(struct lyxp_expr *exp, uint16_t exp_idx)
{
    uint16_t i;

    if (!exp->repeat[exp_idx]) {
        LOGINT;
        return;
    }

    for (i = 0; exp->repeat[exp_idx][i + 1]; ++i);

    exp->repeat[exp_idx][i] = 0;
}

/**
 * @brief Stack operation push on the repeat array.
 *
 * @param[in] exp Expression to use.
 * @param[in] exp_idx Position in the expresion \p exp.
 * @param[in] repeat_op_idx Index from \p exp of the operator token. This value is pushed.
 */
static void
exp_repeat_push(struct lyxp_expr *exp, uint16_t exp_idx, uint16_t repeat_op_idx)
{
    uint16_t i;

    if (exp->repeat[exp_idx]) {
        for (i = 0; exp->repeat[exp_idx][i]; ++i);
        exp->repeat[exp_idx] = realloc(exp->repeat[exp_idx], (i + 2) * sizeof *exp->repeat[exp_idx]);
        if (!exp->repeat[exp_idx]) {
            LOGMEM;
            return;
        }
        exp->repeat[exp_idx][i] = repeat_op_idx;
        exp->repeat[exp_idx][i + 1] = 0;
    } else {
        exp->repeat[exp_idx] = calloc(2, sizeof *exp->repeat[exp_idx]);
        if (!exp->repeat[exp_idx]) {
            LOGMEM;
            return;
        }
        exp->repeat[exp_idx][0] = repeat_op_idx;
    }
}

/**
 * @brief Reparse Predicate. Logs directly on error.
 *
 * [6] Predicate ::= '[' Expr ']'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_predicate(struct lyxp_expr *exp, uint16_t *exp_idx, uint32_t line)
{
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_BRACK1, line)) {
        return -1;
    }
    ++(*exp_idx);

    if (reparse_expr(exp, exp_idx, line)) {
        return -1;
    }

    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_BRACK2, line)) {
        return -1;
    }
    ++(*exp_idx);

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse RelativeLocationPath. Logs directly on error.
 *
 * [3] RelativeLocationPath ::= Step | RelativeLocationPath '/' Step | RelativeLocationPath '//' Step
 * [4] Step ::= '@'? NodeTest Predicate* | '.' | '..'
 * [5] NodeTest ::= NameTest | NodeType '(' ')'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
reparse_relative_location_path(struct lyxp_expr *exp, uint16_t *exp_idx, uint32_t line)
{
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_NONE, line)) {
        return -1;
    }

    goto step;
    do {
        /* '/' or '//' */
        ++(*exp_idx);

        if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_NONE, line)) {
            return -1;
        }
step:
        /* Step */
        switch (exp->tokens[*exp_idx]) {
        case LYXP_TOKEN_DOT:
            ++(*exp_idx);
            break;

        case LYXP_TOKEN_DDOT:
            ++(*exp_idx);
            break;

        case LYXP_TOKEN_AT:
            ++(*exp_idx);

            if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_NONE, line)) {
                return -1;
            }
            if ((exp->tokens[*exp_idx] != LYXP_TOKEN_NAMETEST) && (exp->tokens[*exp_idx] != LYXP_TOKEN_NODETYPE)) {
                LOGVAL(LYE_XPATH_INTOK, line, 0, NULL,
                       print_token(exp->tokens[*exp_idx]), &exp->expr[exp->expr_pos[*exp_idx]]);
                return -1;
            }
            /* fall through */
        case LYXP_TOKEN_NAMETEST:
            ++(*exp_idx);
            goto reparse_predicate;
            break;

        case LYXP_TOKEN_NODETYPE:
            ++(*exp_idx);

            /* '(' */
            if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_PAR1, line)) {
                return -1;
            }
            ++(*exp_idx);

            /* ')' */
            if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_PAR2, line)) {
                return -1;
            }
            ++(*exp_idx);

reparse_predicate:
            /* Predicate* */
            while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_BRACK1)) {
                if (reparse_predicate(exp, exp_idx, line)) {
                    return -1;
                }
            }
            break;
        default:
            LOGVAL(LYE_XPATH_INTOK, line, 0, NULL,
                   print_token(exp->tokens[*exp_idx]), &exp->expr[exp->expr_pos[*exp_idx]]);
            return -1;
        }
    } while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_PATH));

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse AbsoluteLocationPath. Logs directly on error.
 *
 * [2] AbsoluteLocationPath ::= '/' RelativeLocationPath? | '//' RelativeLocationPath
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_absolute_location_path(struct lyxp_expr *exp, uint16_t *exp_idx, uint32_t line)
{
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_PATH, line)) {
        return -1;
    }

    /* '/' RelativeLocationPath? */
    if (exp->tok_len[*exp_idx] == 1) {
        /* '/' */
        ++(*exp_idx);

        if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_NONE, UINT_MAX)) {
            return EXIT_SUCCESS;
        }
        switch (exp->tokens[*exp_idx]) {
        case LYXP_TOKEN_DOT:
        case LYXP_TOKEN_DDOT:
        case LYXP_TOKEN_AT:
        case LYXP_TOKEN_NAMETEST:
        case LYXP_TOKEN_NODETYPE:
            if (reparse_relative_location_path(exp, exp_idx, line)) {
                return -1;
            }
            /* fall through */
        default:
            break;
        }

    /* '//' RelativeLocationPath */
    } else {
        /* '//' */
        ++(*exp_idx);

        if (reparse_relative_location_path(exp, exp_idx, line)) {
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse FunctionCall. Logs directly on error.
 *
 * [8] FunctionCall ::= FunctionName '(' ( Expr ( ',' Expr )* )? ')'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_function_call(struct lyxp_expr *exp, uint16_t *exp_idx, uint32_t line)
{
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_FUNCNAME, line)) {
        return -1;
    }
    ++(*exp_idx);

    /* '(' */
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_PAR1, line)) {
        return -1;
    }
    ++(*exp_idx);

    /* ( Expr ( ',' Expr )* )? */
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_NONE, line)) {
        return -1;
    }
    if (exp->tokens[*exp_idx] != LYXP_TOKEN_PAR2) {
        if (reparse_expr(exp, exp_idx, line)) {
            return -1;
        }
    }
    while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_COMMA)) {
        ++(*exp_idx);

        if (reparse_expr(exp, exp_idx, line)) {
            return -1;
        }
    }

    /* ')' */
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_PAR2, line)) {
        return -1;
    }
    ++(*exp_idx);

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse PathExpr. Logs directly on error.
 *
 * [9] PathExpr ::= LocationPath | PrimaryExpr Predicate*
 *                 | PrimaryExpr Predicate* '/' RelativeLocationPath
 *                 | PrimaryExpr Predicate* '//' RelativeLocationPath
 * [1] LocationPath ::= RelativeLocationPath | AbsoluteLocationPath
 * [7] PrimaryExpr ::= '(' Expr ')' | Literal | Number | FunctionCall
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_path_expr(struct lyxp_expr *exp, uint16_t *exp_idx, uint32_t line)
{
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_NONE, line)) {
        return -1;
    }

    switch (exp->tokens[*exp_idx]) {
    case LYXP_TOKEN_PAR1:
        /* '(' Expr ')' Predicate* */
        ++(*exp_idx);

        if (reparse_expr(exp, exp_idx, line)) {
            return -1;
        }

        if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_PAR2, line)) {
            return -1;
        }
        ++(*exp_idx);
        goto predicate;
        break;
    case LYXP_TOKEN_DOT:
    case LYXP_TOKEN_DDOT:
    case LYXP_TOKEN_AT:
    case LYXP_TOKEN_NAMETEST:
    case LYXP_TOKEN_NODETYPE:
        /* RelativeLocationPath */
        if (reparse_relative_location_path(exp, exp_idx, line)) {
            return -1;
        }
        break;
    case LYXP_TOKEN_FUNCNAME:
        /* FunctionCall */
        if (reparse_function_call(exp, exp_idx, line)) {
            return -1;
        }
        goto predicate;
        break;
    case LYXP_TOKEN_OPERATOR_PATH:
        /* AbsoluteLocationPath */
        if (reparse_absolute_location_path(exp, exp_idx, line)) {
            return -1;
        }
        break;
    case LYXP_TOKEN_LITERAL:
        /* Literal */
        ++(*exp_idx);
        goto predicate;
        break;
    case LYXP_TOKEN_NUMBER:
        /* Number */
        ++(*exp_idx);
        goto predicate;
        break;
    default:
        LOGVAL(LYE_XPATH_INTOK, line, 0, NULL,
               print_token(exp->tokens[*exp_idx]), &exp->expr[exp->expr_pos[*exp_idx]]);
        return -1;
    }

    return EXIT_SUCCESS;

predicate:
    /* Predicate* */
    while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_BRACK1)) {
        if (reparse_predicate(exp, exp_idx, line)) {
            return -1;
        }
    }

    /* ('/' or '//') RelativeLocationPath */
    if ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_PATH)) {

        /* '/' or '//' */
        ++(*exp_idx);

        if (reparse_relative_location_path(exp, exp_idx, line)) {
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse UnaryExpr. Logs directly on error.
 *
 * [16] UnaryExpr ::= UnionExpr | '-' UnaryExpr
 * [17] UnionExpr ::= PathExpr | UnionExpr '|' PathExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_unary_expr(struct lyxp_expr *exp, uint16_t *exp_idx, uint32_t line)
{
    uint16_t prev_exp;

    /* ('-')* */
    while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_MATH, UINT_MAX)
            && (exp->expr[exp->expr_pos[*exp_idx]] == '-')) {
        ++(*exp_idx);
    }

    /* PathExpr */
    prev_exp = *exp_idx;
    if (reparse_path_expr(exp, exp_idx, line)) {
        return -1;
    }

    /* ('|' PathExpr)* */
    while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_UNI, UINT_MAX)) {
        exp_repeat_push(exp, prev_exp, *exp_idx);
        ++(*exp_idx);

        prev_exp = *exp_idx;
        if (reparse_path_expr(exp, exp_idx, line)) {
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse AdditiveExpr. Logs directly on error.
 *
 * [14] AdditiveExpr ::= MultiplicativeExpr
 *                     | AdditiveExpr '+' MultiplicativeExpr
 *                     | AdditiveExpr '-' MultiplicativeExpr
 * [15] MultiplicativeExpr ::= UnaryExpr
 *                     | MultiplicativeExpr '*' UnaryExpr
 *                     | MultiplicativeExpr 'div' UnaryExpr
 *                     | MultiplicativeExpr 'mod' UnaryExpr
 *
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_additive_expr(struct lyxp_expr *exp, uint16_t *exp_idx, uint32_t line)
{
    uint16_t prev_add_exp, prev_mul_exp;

    goto reparse_multiplicative_expr;

    /* ('+' / '-' MultiplicativeExpr)* */
    while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_MATH, UINT_MAX)
            && ((exp->expr[exp->expr_pos[*exp_idx]] == '+') || (exp->expr[exp->expr_pos[*exp_idx]] == '-'))) {
        exp_repeat_push(exp, prev_add_exp, *exp_idx);
        ++(*exp_idx);

reparse_multiplicative_expr:
        prev_add_exp = *exp_idx;
        prev_mul_exp = *exp_idx;

        /* UnaryExpr */
        if (reparse_unary_expr(exp, exp_idx, line)) {
            return -1;
        }

        /* ('*' / 'div' / 'mod' UnaryExpr)* */
        while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_MATH, UINT_MAX)
                && ((exp->expr[exp->expr_pos[*exp_idx]] == '*') || (exp->tok_len[*exp_idx] == 3))) {
            exp_repeat_push(exp, prev_mul_exp, *exp_idx);
            ++(*exp_idx);

            prev_mul_exp = *exp_idx;
            if (reparse_unary_expr(exp, exp_idx, line)) {
                return -1;
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse EqualityExpr. Logs directly on error.
 *
 * [12] EqualityExpr ::= RelationalExpr | EqualityExpr '=' RelationalExpr
 *                     | EqualityExpr '!=' RelationalExpr
 * [13] RelationalExpr ::= AdditiveExpr
 *                       | RelationalExpr '<' AdditiveExpr
 *                       | RelationalExpr '>' AdditiveExpr
 *                       | RelationalExpr '<=' AdditiveExpr
 *                       | RelationalExpr '>=' AdditiveExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_equality_expr(struct lyxp_expr *exp, uint16_t *exp_idx, uint32_t line)
{
    uint16_t prev_eq_exp, prev_rel_exp;

    goto reparse_additive_expr;

    /* ('=' / '!=' RelationalExpr)* */
    while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_COMP, UINT_MAX)
            && ((exp->expr[exp->expr_pos[*exp_idx]] == '=') || (exp->expr[exp->expr_pos[*exp_idx]] == '!'))) {
        exp_repeat_push(exp, prev_eq_exp, *exp_idx);
        ++(*exp_idx);

reparse_additive_expr:
        prev_eq_exp = *exp_idx;
        prev_rel_exp = *exp_idx;

        /* AdditiveExpr */
        if (reparse_additive_expr(exp, exp_idx, line)) {
            return -1;
        }

        /* ('<' / '>' / '<=' / '>=' AdditiveExpr)* */
        while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_COMP, UINT_MAX)
                && ((exp->expr[exp->expr_pos[*exp_idx]] == '<') || (exp->expr[exp->expr_pos[*exp_idx]] == '>'))) {
            exp_repeat_push(exp, prev_rel_exp, *exp_idx);
            ++(*exp_idx);

            prev_rel_exp = *exp_idx;
            if (reparse_additive_expr(exp, exp_idx, line)) {
                return -1;
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Reparse Expr. Logs directly on error.
 *
 * [10] Expr ::= AndExpr | Expr 'or' AndExpr
 * [11] AndExpr ::= EqualityExpr | AndExpr 'and' EqualityExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_expr(struct lyxp_expr *exp, uint16_t *exp_idx, uint32_t line)
{
    uint16_t prev_or_exp, prev_and_exp;

    goto reparse_equality_expr;

    /* ('or' AndExpr)* */
    while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_LOG, UINT_MAX) && (exp->tok_len[*exp_idx] == 2)) {
        exp_repeat_push(exp, prev_or_exp, *exp_idx);
        ++(*exp_idx);

reparse_equality_expr:
        prev_or_exp = *exp_idx;
        prev_and_exp = *exp_idx;

        /* EqualityExpr */
        if (reparse_equality_expr(exp, exp_idx, line)) {
            return -1;
        }

        /* ('and' EqualityExpr)* */
        while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_LOG, UINT_MAX) && (exp->tok_len[*exp_idx] == 3)) {
            exp_repeat_push(exp, prev_and_exp, *exp_idx);
            ++(*exp_idx);

            prev_and_exp = *exp_idx;
            if (reparse_equality_expr(exp, exp_idx, line)) {
                return -1;
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Parse NCName.
 *
 * @param[in] ncname Name to parse.
 *
 * @return Length of \p ncname valid characters.
 */
static uint16_t
parse_ncname(const char *ncname)
{
    uint16_t parsed = 0;
    int uc;
    unsigned int size;

    uc = lyxml_getutf8(&ncname[parsed], &size, UINT_MAX);
    if (!is_xmlnamestartchar(uc) || (uc == ':')) {
       return parsed;
    }

    do {
        parsed += size;
        if (!ncname[parsed]) {
            break;
        }
        uc = lyxml_getutf8(&ncname[parsed], &size, UINT_MAX);
    } while (is_xmlnamechar(uc) && (uc != ':'));

    return parsed;
}

/**
 * @brief Parse an XPath expression into a structure of tokens.
 *        Logs directly.
 *
 * http://www.w3.org/TR/1999/REC-xpath-19991116/ section 3.7
 *
 * @param[in] expr XPath expression to parse.
 * @param[in] line Line of the expression in the input file.
 *
 * @return Filled expression structure or NULL on error.
 */
static struct lyxp_expr *
parse_expr(const char *expr, uint32_t line)
{
    struct lyxp_expr *ret;
    uint16_t parsed = 0, tok_len, ncname_len;
    enum lyxp_token tok_type;
    int prev_function_check = 0;

    /* init lyxp_expr structure */
    ret = malloc(sizeof *ret);
    if (!ret) {
        LOGMEM;
        return NULL;
    }
    ret->expr = expr;
    ret->used = 0;
    ret->size = LYXP_EXPR_SIZE_START;
    ret->tokens = malloc(ret->size * sizeof *ret->tokens);
    if (!ret->tokens) {
        LOGMEM;
        free(ret);
        return NULL;
    }
    ret->expr_pos = malloc(ret->size * sizeof *ret->expr_pos);
    if (!ret->expr_pos) {
        LOGMEM;
        free(ret->tokens);
        free(ret);
        return NULL;
    }
    ret->tok_len = malloc(ret->size * sizeof *ret->tok_len);
    if (!ret->tok_len) {
        LOGMEM;
        free(ret->tokens);
        free(ret->expr_pos);
        free(ret);
        return NULL;
    }

    while (is_xmlws(expr[parsed])) {
        ++parsed;
    }

    do {
        if (expr[parsed] == '(') {

            /* '(' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_PAR1;

            if (prev_function_check && ret->used && (ret->tokens[ret->used - 1] == LYXP_TOKEN_NAMETEST)) {
                /* it is a NodeType/FunctionName after all */
                if (((ret->tok_len[ret->used - 1] == 4)
                        && (!strncmp(&expr[ret->expr_pos[ret->used - 1]], "node", 4)
                        || !strncmp(&expr[ret->expr_pos[ret->used - 1]], "text", 4))) ||
                        ((ret->tok_len[ret->used - 1] == 7)
                        && !strncmp(&expr[ret->expr_pos[ret->used - 1]], "comment", 7))) {
                    ret->tokens[ret->used - 1] = LYXP_TOKEN_NODETYPE;
                } else {
                    ret->tokens[ret->used - 1] = LYXP_TOKEN_FUNCNAME;
                }
                prev_function_check = 0;
            }

        } else if (expr[parsed] == ')') {

            /* ')' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_PAR2;

        } else if (expr[parsed] == '[') {

            /* '[' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_BRACK1;

        } else if (expr[parsed] == ']') {

            /* ']' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_BRACK2;

        } else if (!strncmp(&expr[parsed], "..", 2)) {

            /* '..' */
            tok_len = 2;
            tok_type = LYXP_TOKEN_DDOT;

        } else if ((expr[parsed] == '.') && (!isdigit(expr[parsed+1]))) {

            /* '.' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_DOT;

        } else if (expr[parsed] == '@') {

            /* '@' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_AT;

        } else if (expr[parsed] == ',') {

            /* ',' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_COMMA;

        } else if (expr[parsed] == '\'') {

            /* Literal with ' */
            for (tok_len = 1; expr[parsed + tok_len] != '\''; ++tok_len);
            ++tok_len;
            tok_type = LYXP_TOKEN_LITERAL;

        } else if (expr[parsed] == '\"') {

            /* Literal with " */
            for (tok_len = 1; expr[parsed + tok_len] != '\"'; ++tok_len);
            ++tok_len;
            tok_type = LYXP_TOKEN_LITERAL;

        } else if ((expr[parsed] == '.') || (isdigit(expr[parsed]))) {

            /* Number */
            for (tok_len = 0; isdigit(expr[parsed + tok_len]); ++tok_len);
            if (expr[parsed + tok_len] == '.') {
                ++tok_len;
                for (; isdigit(expr[parsed + tok_len]); ++tok_len);
            }
            tok_type = LYXP_TOKEN_NUMBER;

        } else if (expr[parsed] == '/') {

            /* Operator '/', '//' */
            if (!strncmp(&expr[parsed], "//", 2)) {
                tok_len = 2;
            } else {
                tok_len = 1;
            }
            tok_type = LYXP_TOKEN_OPERATOR_PATH;

        } else if  (!strncmp(&expr[parsed], "!=", 2) || !strncmp(&expr[parsed], "<=", 2)
                || !strncmp(&expr[parsed], ">=", 2)) {

            /* Operator '!=', '<=', '>=' */
            tok_len = 2;
            tok_type = LYXP_TOKEN_OPERATOR_COMP;

        } else if (expr[parsed] == '|') {

            /* Operator '|' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_OPERATOR_UNI;

        } else if ((expr[parsed] == '+') || (expr[parsed] == '-')) {

            /* Operator '+', '-' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_OPERATOR_MATH;

        } else if ((expr[parsed] == '=') || (expr[parsed] == '<') || (expr[parsed] == '>')) {

            /* Operator '=', '<', '>' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_OPERATOR_COMP;

        } else if (ret->used && (ret->tokens[ret->used - 1] != LYXP_TOKEN_AT)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_PAR1)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_BRACK1)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_COMMA)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_OPERATOR_LOG)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_OPERATOR_COMP)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_OPERATOR_MATH)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_OPERATOR_UNI)
                && (ret->tokens[ret->used - 1] != LYXP_TOKEN_OPERATOR_PATH)) {

            /* Operator '*', 'or', 'and', 'mod', or 'div' */
            if (expr[parsed] == '*') {
                tok_len = 1;
                tok_type = LYXP_TOKEN_OPERATOR_MATH;

            } else if (!strncmp(&expr[parsed], "or", 2)) {
                tok_len = 2;
                tok_type = LYXP_TOKEN_OPERATOR_LOG;

            } else if (!strncmp(&expr[parsed], "and", 3)) {
                tok_len = 3;
                tok_type = LYXP_TOKEN_OPERATOR_LOG;

            } else if (!strncmp(&expr[parsed], "mod", 3) || !strncmp(&expr[parsed], "div", 3)) {
                tok_len = 3;
                tok_type = LYXP_TOKEN_OPERATOR_MATH;

            } else {
                LOGVAL(LYE_INCHAR, line, 0, NULL, expr[parsed], &expr[parsed]);
                goto error;
            }
        } else if (expr[parsed] == '*') {

            /* NameTest '*' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_NAMETEST;

        } else {

            /* NameTest (NCName ':' '*' | QName) or NodeType/FunctionName */
            ncname_len = parse_ncname(&expr[parsed]);
            if (!ncname_len) {
                LOGVAL(LYE_INCHAR, line, 0, NULL, expr[parsed], &expr[parsed]);
                goto error;
            }
            tok_len = ncname_len;

            if (expr[parsed + tok_len] == ':') {
                ++tok_len;
                if (expr[parsed + tok_len] == '*') {
                    ++tok_len;
                } else {
                    ncname_len = parse_ncname(&expr[parsed + tok_len]);
                    if (!ncname_len) {
                        LOGVAL(LYE_INCHAR, line, 0, NULL, expr[parsed], &expr[parsed]);
                        goto error;
                    }
                    tok_len += ncname_len;
                }
                /* remove old flag to prevent ambiguities */
                prev_function_check = 0;
                tok_type = LYXP_TOKEN_NAMETEST;
            } else {
                /* there is no prefix so it can still be NodeType/FunctioName, we can't finally decide now */
                prev_function_check = 1;
                tok_type = LYXP_TOKEN_NAMETEST;
            }
        }

        /* store the token, move on to the next one */
        exp_add_token(ret, tok_type, parsed, tok_len);
        parsed += tok_len;
        while (is_xmlws(expr[parsed])) {
            ++parsed;
        }

    } while (expr[parsed]);

    /* prealloc repeat */
    ret->repeat = calloc(ret->size, sizeof *ret->repeat);
    if (!ret->repeat) {
        LOGMEM;
        goto error;
    }

    return ret;

error:
    free(ret->tokens);
    free(ret->expr_pos);
    free(ret->tok_len);
    free(ret);

    return NULL;
}

/*
 * XPath functions
 */

/**
 * @brief Execute the XPath boolean(object) function. Returns LYXP_SET_BOOLEAN
 *        with the argument converted to boolean.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_boolean(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
              int when_must_eval, uint32_t line)
{
    struct ly_ctx *ctx;

    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "boolean(object)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    lyxp_set_cast(args, LYXP_SET_BOOLEAN, cur_node, when_must_eval);
    set_fill_set(set, args, ctx);

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath ceiling(number) function. Returns LYXP_SET_NUMBER
 *        with the first argument rounded up to the nearest integer.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_ceiling(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
              int when_must_eval, uint32_t line)
{
    struct ly_ctx *ctx;

    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "ceiling(number)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    lyxp_set_cast(args, LYXP_SET_NUMBER, cur_node, when_must_eval);
    if ((long long)args->value.num != args->value.num) {
        set_fill_number(set, ((long long)args->value.num) + 1, ctx);
    } else {
        set_fill_number(set, args->value.num, ctx);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath concat(string, string, string*) function.
 *        Returns LYXP_SET_STRING with the concatenation of all the arguments.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_concat(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
             int when_must_eval, uint32_t line)
{
    uint16_t i;
    char *str = NULL;
    size_t used = 1;
    struct ly_ctx *ctx;

    if (arg_count < 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "concat(string, string, string*)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    for (i = 0; i < arg_count; ++i) {
        lyxp_set_cast(&args[i], LYXP_SET_STRING, cur_node, when_must_eval);

        str = ly_realloc(str, (used + strlen(args[i].value.str)) * sizeof(char));
        if (!str) {
            LOGMEM;
            return -1;
        }
        strcpy(str + used - 1, args[i].value.str);
        used += strlen(args[i].value.str);
    }

    /* free, kind of */
    lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, when_must_eval);
    set->type = LYXP_SET_STRING;
    set->value.str = lydict_insert_zc(ctx, str);

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath contains(string, string) function.
 *        Returns LYXP_SET_BOOLEAN whether the second argument can
 *        be found in the first or not.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_contains(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
               int when_must_eval, uint32_t line)
{
    struct ly_ctx *ctx;

    if (arg_count != 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "contains(string, string)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    lyxp_set_cast(&args[0], LYXP_SET_STRING, cur_node, when_must_eval);
    lyxp_set_cast(&args[1], LYXP_SET_STRING, cur_node, when_must_eval);

    if (strstr(args[0].value.str, args[1].value.str)) {
        set_fill_boolean(set, 1, ctx);
    } else {
        set_fill_boolean(set, 0, ctx);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath count(node-set) function. Returns LYXP_SET_NUMBER
 *        with the size of the node-set from the argument.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_count(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
            int UNUSED(when_must_eval), uint32_t line)
{
    struct ly_ctx *ctx;

    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "count(node-set)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    if (args->type == LYXP_SET_EMPTY) {
        set_fill_number(set, 0, ctx);
        return EXIT_SUCCESS;
    }

    if (args->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INARGTYPE, line, 0, NULL, 1, print_set_type(args), "count(node-set)");
        return -1;
    }

    set_fill_number(set, args->used, ctx);
    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath current() function. Returns LYXP_SET_NODE_SET
 *        with the context with the intial node.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_current(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
              int when_must_eval, uint32_t line)
{
    if (arg_count || args) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "current()");
        return -1;
    }

    lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, when_must_eval);
    set_insert_node(set, cur_node, LYXP_NODE_ELEM, 0);
    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath false() function. Returns LYXP_SET_BOOLEAN
 *        with false value.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_false(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
            int UNUSED(when_must_eval), uint32_t line)
{
    struct ly_ctx *ctx;

    if (arg_count || args) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "false()");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    set_fill_boolean(set, 0, ctx);
    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath floor(number) function. Returns LYXP_SET_NUMBER
 *        with the first argument floored (truncated).
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_floor(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
            int when_must_eval, uint32_t line)
{
    struct ly_ctx *ctx;

    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "floor(number)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    lyxp_set_cast(args, LYXP_SET_NUMBER, cur_node, when_must_eval);
    if (isfinite(args->value.num)) {
        set_fill_number(set, (long long)args->value.num, ctx);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath lang(string) function. Returns LYXP_SET_BOOLEAN
 *        whether the language of the text matches the one from the argument.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_lang(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
           int when_must_eval, uint32_t line)
{
    struct lyd_node *node;
    struct lyd_attr *attr = NULL;
    int i;
    struct ly_ctx *ctx;

    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "lang(string)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    lyxp_set_cast(&args[0], LYXP_SET_STRING, cur_node, when_must_eval);

    if (set->type == LYXP_SET_EMPTY) {
        set_fill_boolean(set, 0, ctx);
        return EXIT_SUCCESS;
    }
    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INCTX, line, 0, NULL, print_set_type(set), "lang(string)");
        return -1;
    }

    /* assign the context node */
    if (set->pos) {
        node = set->value.nodes[set->pos - 1];
    } else {
        node = set->value.nodes[0];
    }

    /* find lang sttribute */
    for (; node; node = node->parent) {
        for (attr = node->attr; attr; attr = attr->next) {
            if (attr->name && !strcmp(attr->name, "lang") && !strcmp(attr->module->name, "xml")) {
                break;
            }
        }

        if (attr) {
            break;
        }
    }

    /* compare languages */
    if (!attr) {
        set_fill_boolean(set, 0, ctx);
    } else {
        for (i = 0; args->value.str[i]; ++i) {
            if (tolower(args->value.str[i]) != tolower(attr->value[i])) {
                set_fill_boolean(set, 0, ctx);
                break;
            }
        }
        if (!args->value.str[i]) {
            if (!attr->value[i] || (attr->value[i] == '-')) {
                set_fill_boolean(set, 1, ctx);
            } else {
                set_fill_boolean(set, 0, ctx);
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath last() function. Returns LYXP_SET_NUMBER
 *        with the context size.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_last(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
           int UNUSED(when_must_eval), uint32_t line)
{
    struct ly_ctx *ctx;

    if (arg_count || args) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "last()");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    if (set->type == LYXP_SET_EMPTY) {
        set_fill_number(set, 0, ctx);
        return EXIT_SUCCESS;
    }
    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INCTX, line, 0, NULL, print_set_type(set), "last()");
        return -1;
    }

    set_fill_number(set, set->used, ctx);
    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath local-name(node-set?) function. Returns LYXP_SET_STRING
 *        with the node name without namespace from the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_local_name(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                 int UNUSED(when_must_eval), uint32_t line)
{
    struct lyd_node *node;
    enum lyxp_node_type type;
    struct ly_ctx *ctx;

    if (arg_count > 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "local-name(node-set?)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    if (arg_count) {
        if (args->type == LYXP_SET_EMPTY) {
            set_fill_string(set, "", 0, ctx);
            return EXIT_SUCCESS;
        }
        if (args->type != LYXP_SET_NODE_SET) {
            LOGVAL(LYE_XPATH_INARGTYPE, line, 0, NULL, 1, print_set_type(args), "local-name(node-set?)");
            return -1;
        }

        node = args->value.nodes[0];
        type = args->node_type[0];
    } else {
        if (set->type == LYXP_SET_EMPTY) {
            set_fill_string(set, "", 0, ctx);
            return EXIT_SUCCESS;
        }
        if (set->type != LYXP_SET_NODE_SET) {
            LOGVAL(LYE_XPATH_INCTX, line, 0, NULL, print_set_type(set), "local-name(node-set?)");
            return -1;
        }

        if (set->pos) {
            node = set->value.nodes[set->pos - 1];
            type = set->node_type[set->pos - 1];
        } else {
            node = set->value.nodes[0];
            type = set->node_type[0];
        }
    }

    switch (type) {
    case LYXP_NODE_ROOT_CONFIG:
    case LYXP_NODE_ROOT_STATE:
    case LYXP_NODE_ROOT_NOTIF:
    case LYXP_NODE_ROOT_RPC:
    case LYXP_NODE_ROOT_OUTPUT:
    case LYXP_NODE_TEXT:
        set_fill_string(set, "", 0, ctx);
        break;
    case LYXP_NODE_ELEM:
        set_fill_string(set, node->schema->name, strlen(node->schema->name), ctx);
        break;
    case LYXP_NODE_ATTR:
        set_fill_string(set, ((struct lyd_attr *)node)->name, strlen(((struct lyd_attr *)node)->name), ctx);
        break;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath namespace-uri(node-set?) function. Returns LYXP_SET_STRING
 *        with the namespace of the node from the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_namespace_uri(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                    int UNUSED(when_must_eval), uint32_t line)
{
    struct lyd_node *node;
    struct lys_module *module;
    enum lyxp_node_type type;
    struct ly_ctx *ctx;

    if (arg_count > 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "namespace-uri(node-set?)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    if (arg_count) {
        if (args->type == LYXP_SET_EMPTY) {
            set_fill_string(set, "", 0, ctx);
            return EXIT_SUCCESS;
        }
        if (args->type != LYXP_SET_NODE_SET) {
            LOGVAL(LYE_XPATH_INARGTYPE, line, 0, NULL, 1, print_set_type(args), "namespace-uri(node-set?)");
            return -1;
        }

        node = args->value.nodes[0];
        type = args->node_type[0];
    } else {
        if (set->type == LYXP_SET_EMPTY) {
            set_fill_string(set, "", 0, ctx);
            return EXIT_SUCCESS;
        }
        if (set->type != LYXP_SET_NODE_SET) {
            LOGVAL(LYE_XPATH_INCTX, line, 0, NULL, print_set_type(set), "namespace-uri(node-set?)");
            return -1;
        }

        if (set->pos) {
            node = set->value.nodes[set->pos - 1];
            type = set->node_type[set->pos - 1];
        } else {
            node = set->value.nodes[0];
            type = set->node_type[0];
        }
    }

    switch (type) {
    case LYXP_NODE_ROOT_CONFIG:
    case LYXP_NODE_ROOT_STATE:
    case LYXP_NODE_ROOT_NOTIF:
    case LYXP_NODE_ROOT_RPC:
    case LYXP_NODE_ROOT_OUTPUT:
    case LYXP_NODE_TEXT:
        set_fill_string(set, "", 0, ctx);
        break;
    case LYXP_NODE_ELEM:
    case LYXP_NODE_ATTR:
        if (type == LYXP_NODE_ELEM) {
            module =  node->schema->module;
        } else { /* LYXP_NODE_ATTR */
            module = ((struct lyd_attr *)node)->module;
        }

        module = lys_module(module);

        set_fill_string(set, module->ns, strlen(module->ns), ctx);
        break;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath node() function (node type). Returns LYXP_SET_NODE_SET
 *        with only nodes from the context. In practice it either leaves the context
 *        as it is or returns an empty node set.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_node(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
           int when_must_eval, uint32_t line)
{
    if (arg_count || args) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "node()");
        return -1;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, when_must_eval);
    }
    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath normalize-space(string?) function. Returns LYXP_SET_STRING
 *        with normalized value (no leading, trailing, double white spaces) of the node
 *        from the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_normalize_space(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                      int when_must_eval, uint32_t line)
{
    uint16_t i, new_used;
    char *new;
    int have_spaces = 0, space_before = 0;
    struct ly_ctx *ctx;

    if (arg_count > 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "normalize-space(string?)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    if (arg_count) {
        set_fill_set(set, args, ctx);
    }
    lyxp_set_cast(set, LYXP_SET_STRING, cur_node, when_must_eval);

    /* is there any normalization necessary? */
    for (i = 0; set->value.str[i]; ++i) {
        if (is_xmlws(set->value.str[i])) {
            if ((i == 0) || space_before || (!set->value.str[i + 1])) {
                have_spaces = 1;
                break;
            }
            space_before = 1;
        } else {
            space_before = 0;
        }
    }

    /* yep, there is */
    if (have_spaces) {
        /* it's enough, at least one character will go, makes space for ending '\0' */
        new = malloc(strlen(set->value.str) * sizeof(char));
        if (!new) {
            LOGMEM;
            return -1;
        }
        new_used = 0;

        space_before = 0;
        for (i = 0; set->value.str[i]; ++i) {
            if (is_xmlws(set->value.str[i])) {
                if ((i == 0) || space_before) {
                    space_before = 1;
                    continue;
                } else {
                    space_before = 1;
                }
            } else {
                space_before = 0;
            }

            new[new_used] = set->value.str[i];
            ++new_used;
        }

        /* at worst there is one trailing space now */
        if (new_used && is_xmlws(new[new_used - 1])) {
            --new_used;
        }

        new = ly_realloc(new, (new_used + 1) * sizeof(char));
        if (!new) {
            LOGMEM;
            return -1;
        }
        new[new_used] = '\0';

        lydict_remove(ctx, set->value.str);
        set->value.str = lydict_insert_zc(ctx, new);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath not(boolean) function. Returns LYXP_SET_BOOLEAN
 *        with the argument converted to boolean and logically inverted.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_not(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
          int when_must_eval, uint32_t line)
{
    struct ly_ctx *ctx;

    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "not(boolean)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    lyxp_set_cast(args, LYXP_SET_BOOLEAN, cur_node, when_must_eval);
    if (args->value.bool) {
        set_fill_boolean(set, 0, ctx);
    } else {
        set_fill_boolean(set, 1, ctx);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath bumber(object?) function. Returns LYXP_SET_NUMBER
 *        with the number representation of either the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_number(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
             int when_must_eval, uint32_t line)
{
    struct ly_ctx *ctx;

    if (arg_count > 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "number(object?)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    if (arg_count) {
        lyxp_set_cast(args, LYXP_SET_NUMBER, cur_node, when_must_eval);
        set_fill_set(set, args, ctx);
    } else {
        lyxp_set_cast(set, LYXP_SET_NUMBER, cur_node, when_must_eval);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath position() function. Returns LYXP_SET_NUMBER
 *        with the context position.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_position(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
               int UNUSED(when_must_eval), uint32_t line)
{
    struct ly_ctx *ctx;

    if (arg_count || args) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "position()");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    if (set->type == LYXP_SET_EMPTY) {
        set_fill_number(set, 0, ctx);
        return EXIT_SUCCESS;
    }
    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INCTX, line, 0, NULL, print_set_type(set), "position()");
        return -1;
    }

    set_fill_number(set, set->pos, ctx);
    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath round(number) function. Returns LYXP_SET_NUMBER
 *        with the rounded first argument. For details refer to
 *        http://www.w3.org/TR/1999/REC-xpath-19991116/#function-round.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_round(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
            int when_must_eval, uint32_t line)
{
    struct ly_ctx *ctx;

    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "round(number)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    lyxp_set_cast(args, LYXP_SET_NUMBER, cur_node, when_must_eval);

    /* cover only the cases where floor can't be used */
    if ((args->value.num == -0) || ((args->value.num < 0) && (args->value.num >= -0.5))) {
        set_fill_number(set, -0, ctx);
    } else {
        args->value.num += 0.5;
        if (xpath_floor(args, 1, cur_node, args, when_must_eval, line)) {
            return -1;
        }
        set_fill_number(set, args->value.num, ctx);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath starts-with(string, string) function.
 *        Returns LYXP_SET_BOOLEAN whether the second argument is
 *        the prefix of the first or not.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_starts_with(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                  int when_must_eval, uint32_t line)
{
    struct ly_ctx *ctx;

    if (arg_count != 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "starts-with(string, string)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    lyxp_set_cast(&args[0], LYXP_SET_STRING, cur_node, when_must_eval);
    lyxp_set_cast(&args[1], LYXP_SET_STRING, cur_node, when_must_eval);

    if (strncmp(args[0].value.str, args[1].value.str, strlen(args[1].value.str))) {
        set_fill_boolean(set, 0, ctx);
    } else {
        set_fill_boolean(set, 1, ctx);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath string(object?) function. Returns LYXP_SET_STRING
 *        with the string representation of either the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_string(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
             int when_must_eval, uint32_t line)
{
    struct ly_ctx *ctx;

    if (arg_count > 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "string(object?)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    if (arg_count) {
        lyxp_set_cast(args, LYXP_SET_STRING, cur_node, when_must_eval);
        set_fill_set(set, args, ctx);
    } else {
        lyxp_set_cast(set, LYXP_SET_STRING, cur_node, when_must_eval);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath string-length(string?) function. Returns LYXP_SET_NUMBER
 *        with the length of the string in either the argument or the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_string_length(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                    int when_must_eval, uint32_t line)
{
    struct ly_ctx *ctx;

    if (arg_count > 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "string-length(string?)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    if (arg_count) {
        lyxp_set_cast(args, LYXP_SET_STRING, cur_node, when_must_eval);
        set_fill_number(set, strlen(args->value.str), ctx);
    } else {
        lyxp_set_cast(set, LYXP_SET_STRING, cur_node, when_must_eval);
        set_fill_number(set, strlen(set->value.str), ctx);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath substring(string, number, number?) function.
 *        Returns LYXP_SET_STRING substring of the first argument starting
 *        on the second argument index ending on the third argument index,
 *        indexed from 1. For exact definition refer to
 *        http://www.w3.org/TR/1999/REC-xpath-19991116/#function-substring.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_substring(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                int when_must_eval, uint32_t line)
{
    int start, len;
    uint16_t str_start, str_len, pos;
    struct ly_ctx *ctx;

    if ((arg_count < 2) || (arg_count > 3)) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "substring(string, number, number?)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    lyxp_set_cast(&args[0], LYXP_SET_STRING, cur_node, when_must_eval);

    /* start */
    if (xpath_round(&args[1], 1, cur_node, &args[1], when_must_eval, line)) {
        return -1;
    }
    if (isfinite(args[1].value.num)) {
        start = args[1].value.num - 1;
    } else if (isinf(args[1].value.num) && signbit(args[1].value.num)) {
        start = INT_MIN;
    } else {
        start = INT_MAX;
    }

    /* len */
    if (arg_count == 3) {
        if (xpath_round(&args[2], 1, cur_node, &args[2], when_must_eval, line)) {
            return -1;
        }
        if (isfinite(args[2].value.num)) {
            len = args[2].value.num;
        } else if (isnan(args[2].value.num) || signbit(args[2].value.num)) {
            len = 0;
        } else {
            len = INT_MAX;
        }
    } else {
        len = INT_MAX;
    }

    /* find matching character positions */
    str_start = 0;
    str_len = 0;
    for (pos = 0; args[0].value.str[pos]; ++pos) {
        if (pos < start) {
            ++str_start;
        } else if (pos < start + len) {
            ++str_len;
        } else {
            break;
        }
    }

    set_fill_string(set, args[0].value.str + str_start, str_len, ctx);
    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath substring-after(string, string) function.
 *        Returns LYXP_SET_STRING with the string succeeding the occurance
 *        of the second argument in the first or an empty string.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_substring_after(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                      int when_must_eval, uint32_t line)
{
    char *ptr;
    struct ly_ctx *ctx;

    if (arg_count != 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "substring-after(string, string)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    lyxp_set_cast(&args[0], LYXP_SET_STRING, cur_node, when_must_eval);
    lyxp_set_cast(&args[1], LYXP_SET_STRING, cur_node, when_must_eval);

    ptr = strstr(args[0].value.str, args[1].value.str);
    if (ptr) {
        set_fill_string(set, ptr + strlen(args[1].value.str), strlen(ptr + strlen(args[1].value.str)), ctx);
    } else {
        set_fill_string(set, "", 0, ctx);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath substring-before(string, string) function.
 *        Returns LYXP_SET_STRING with the string preceding the occurance
 *        of the second argument in the first or an empty string.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_substring_before(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                       int when_must_eval, uint32_t line)
{
    char *ptr;
    struct ly_ctx *ctx;

    if (arg_count != 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "substring-before(string, string)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    lyxp_set_cast(&args[0], LYXP_SET_STRING, cur_node, when_must_eval);
    lyxp_set_cast(&args[1], LYXP_SET_STRING, cur_node, when_must_eval);

    ptr = strstr(args[0].value.str, args[1].value.str);
    if (ptr) {
        set_fill_string(set, args[0].value.str, ptr - args[0].value.str, ctx);
    } else {
        set_fill_string(set, "", 0, ctx);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath sum(node-set) function. Returns LYXP_SET_NUMBER
 *        with the sum of all the nodes in the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_sum(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
          int when_must_eval, uint32_t line)
{
    long double num;
    const char *str;
    uint16_t i;
    struct lyxp_set set_item;
    struct ly_ctx *ctx;

    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "sum(node-set)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    set_fill_number(set, 0, ctx);
    if (args->type == LYXP_SET_EMPTY) {
        return EXIT_SUCCESS;
    }

    if (args->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INARGTYPE, line, 0, NULL, 1, print_set_type(args), "sum(node-set)");
        return -1;
    }

    set_item.type = LYXP_SET_NODE_SET;
    set_item.value.nodes = malloc(sizeof *set_item.value.nodes);
    if (!set_item.value.nodes) {
        LOGMEM;
        return -1;
    }
    set_item.node_type = malloc(sizeof *set_item.node_type);
    if (!set_item.node_type) {
        LOGMEM;
        free(set_item.value.nodes);
        return -1;
    }
    set_item.used = 1;
    set_item.size = 1;
    set_item.pos = 0;

    for (i = 0; i < args->used; ++i) {
        set_item.value.nodes[0] = args->value.nodes[i];
        set_item.node_type[0] = args->node_type[i];

        str = cast_node_set_to_string(&set_item, cur_node, when_must_eval);
        num = cast_string_to_number(str);
        lydict_remove(ctx, str);
        set->value.num += num;
    }

    free(set_item.value.nodes);
    free(set_item.node_type);

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath text() function (node type). Returns LYXP_SET_NODE_SET
 *        with the text content of the nodes in the context.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_text(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *UNUSED(cur_node), struct lyxp_set *set,
           int UNUSED(when_must_eval), uint32_t line)
{
    uint16_t i;

    if (arg_count || args) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "text()");
        return -1;
    }
    if (set->type == LYXP_SET_EMPTY) {
        return EXIT_SUCCESS;
    }
    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INCTX, line, 0, NULL, print_set_type(set), "text()");
        return -1;
    }

    for (i = 0; i < set->used;) {
        switch (set->node_type[i]) {
        case LYXP_NODE_ELEM:
            if ((set->value.nodes[i]->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))
                    && ((struct lyd_node_leaf_list *)set->value.nodes[i])->value_str) {
                set->node_type[i] = LYXP_NODE_TEXT;
                ++i;
                break;
            }
            /* fall through */
        case LYXP_NODE_ROOT_CONFIG:
        case LYXP_NODE_ROOT_STATE:
        case LYXP_NODE_ROOT_NOTIF:
        case LYXP_NODE_ROOT_RPC:
        case LYXP_NODE_ROOT_OUTPUT:
        case LYXP_NODE_TEXT:
        case LYXP_NODE_ATTR:
            set_remove_node(set, i);
            break;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath translate(string, string, string) function.
 *        Returns LYXP_SET_STRING with the first argument with the characters
 *        from the second argument replaced by those on the corresponding
 *        positions in the third argument.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_translate(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                int when_must_eval, uint32_t line)
{
    uint16_t i, j, new_used;
    char *new;
    int found, have_removed;
    struct ly_ctx *ctx;

    if (arg_count != 3) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "translate(string, string, string)");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    lyxp_set_cast(&args[0], LYXP_SET_STRING, cur_node, when_must_eval);
    lyxp_set_cast(&args[1], LYXP_SET_STRING, cur_node, when_must_eval);
    lyxp_set_cast(&args[2], LYXP_SET_STRING, cur_node, when_must_eval);

    new = malloc((strlen(args[0].value.str) + 1) * sizeof(char));
    if (!new) {
        LOGMEM;
        return -1;
    }
    new_used = 0;

    have_removed = 0;
    for (i = 0; args[0].value.str[i]; ++i) {
        found = 0;

        for (j = 0; args[1].value.str[j]; ++j) {
            if (args[0].value.str[i] == args[1].value.str[j]) {
                /* removing this char */
                if (j >= strlen(args[2].value.str)) {
                    have_removed = 1;
                    found = 1;
                    break;
                }
                /* replacing this char */
                new[new_used] = args[2].value.str[j];
                ++new_used;
                found = 1;
                break;
            }
        }

        /* copying this char */
        if (!found) {
            new[new_used] = args[0].value.str[i];
            ++new_used;
        }
    }

    if (have_removed) {
        new = ly_realloc(new, (new_used + 1) * sizeof(char));
        if (!new) {
            LOGMEM;
            return -1;
        }
    }
    new[new_used] = '\0';

    lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, when_must_eval);
    set->type = LYXP_SET_STRING;
    set->value.str = lydict_insert_zc(ctx, new);

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath true() function. Returns LYXP_SET_BOOLEAN
 *        with true value.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_true(struct lyxp_set *args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
           int UNUSED(when_must_eval), uint32_t line)
{
    struct ly_ctx *ctx;

    if (arg_count || args) {
        LOGVAL(LYE_XPATH_INARGCOUNT, line, 0, NULL, arg_count, "true()");
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    set_fill_boolean(set, 1, ctx);
    return EXIT_SUCCESS;
}

/*
 * moveto functions
 *
 * They and only they actually change the context (set).
 */

/**
 * @brief Resolve and find a specific model.
 *
 * @param[in] mod_name_ns Either module name or namespace.
 * @param[in] mon_nam_ns_len Length of \p mod_name_ns.
 * @param[in] ctx libyang context.
 * @param[in] is_name Whether \p mod_name_ns is module name (1) or namespace (0).
 *
 * @return Corresponding module or NULL on error.
 */
static struct lys_module *
moveto_resolve_model(const char *mod_name_ns, uint16_t mod_nam_ns_len, struct ly_ctx *ctx, int is_name)
{
    uint16_t i;
    const char *str;

    for (i = 0; i < ctx->models.used; ++i) {
        str = (is_name ? ctx->models.list[i]->name : ctx->models.list[i]->ns);
        if (!strncmp(str, mod_name_ns, mod_nam_ns_len) && !str[mod_nam_ns_len]) {
            return ctx->models.list[i];
        }
    }

    LOGINT;
    return NULL;
}

/**
 * @brief Get the context root.
 *
 * @param[in] cur_node Original context node.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[out] root_type Root type, differs only in when, must evaluation.
 *
 * @return Context root.
 */
static struct lyd_node *
moveto_get_root(struct lyd_node *cur_node, int when_must_eval, enum lyxp_node_type *root_type)
{
    struct lyd_node *root, *prev;
    int is_output = 0;

    assert(cur_node && root_type);

    if (!when_must_eval) {
        /* only one kind of root that can access everything */
        for (root = cur_node; root->parent; root = root->parent);
        *root_type = LYXP_NODE_ROOT_STATE;
        return root;
    }

    /* handle all kinds of special XPath roots */
    root = cur_node;
    if (root->parent) {
        do {
            prev = root;
            root = root->parent;

            if ((root->schema->nodetype == LYS_RPC) || (root->schema->nodetype == LYS_NOTIF)) {
                if (prev->schema->parent->nodetype == LYS_OUTPUT) {
                    is_output = 1;
                }
                break;
            }
        } while (root->parent);
    }

    if (root->schema->nodetype == LYS_NOTIF) {
        *root_type = LYXP_NODE_ROOT_NOTIF;
    } else if (root->schema->nodetype == LYS_RPC) {
        if (is_output) {
            *root_type = LYXP_NODE_ROOT_OUTPUT;
        } else {
            *root_type = LYXP_NODE_ROOT_RPC;
        }
    } else {
        if (cur_node->schema->flags & LYS_CONFIG_W) {
            *root_type = LYXP_NODE_ROOT_CONFIG;
        } else {
            assert(cur_node->schema->flags & LYS_CONFIG_R);
            *root_type = LYXP_NODE_ROOT_STATE;
        }

        for (; root->prev->next; root = root->prev);
    }

    return root;
}

/**
 * @brief Move context \p set to the root. Handles absolute path.
 *        Result is LYXP_SET_NODE_SET.
 *
 * @param[in,out] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 */
static void
moveto_root(struct lyxp_set *set, struct lyd_node *cur_node, int when_must_eval)
{
    struct lyd_node *root;
    enum lyxp_node_type root_type;

    if (!set) {
        return;
    }

    if (!cur_node) {
        LOGINT;
        return;
    }

    root = moveto_get_root(cur_node, when_must_eval, &root_type);

    lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, when_must_eval);
    set_insert_node(set, root, root_type, 0);
}

/**
 * @brief Check (process) \p node as a part of NameTest processing.
 *
 * @param[in] node Node to use.
 * @param[in,out] set Set to use.
 * @param[in] i Current index in \p set.
 * @param[in] cur_node Original context node.
 * @param[in] qname Qualified node name to move to.
 * @param[in] qname_len Length of \p qname.
 * @param[in] moveto_mod Expected module of the node.
 * @param[in,out] replaced Whether the node in \p set has already been replaced.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static void
moveto_node_check(struct lyd_node *node, struct lyxp_set *set, uint16_t i, enum lyxp_node_type root_type,
                  const char *qname, uint16_t qname_len, struct lys_module *moveto_mod, int *replaced)
{
    /* module check */
    if (moveto_mod) {
        if (lys_node_module(node->schema) != moveto_mod) {
            return;
        }
    }

    /* context check */
    if (((root_type == LYXP_NODE_ROOT_CONFIG) && (node->schema->flags & LYS_CONFIG_R))
            || ((root_type == LYXP_NODE_ROOT_OUTPUT) && (node->schema->parent->nodetype == LYS_INPUT))
            || ((root_type != LYXP_NODE_ROOT_NOTIF) && (node->schema->nodetype == LYS_NOTIF))
            || ((root_type != LYXP_NODE_ROOT_RPC) && (node->schema->nodetype == LYS_RPC))) {
        return;
    }

    /* name check */
    if (((qname_len == 1) && (qname[0] == '*'))
            || (!strncmp(node->schema->name, qname, qname_len) && !node->schema->name[qname_len])) {
        if (!(*replaced)) {
            set->value.nodes[i] = node;
            set->node_type[i] = LYXP_NODE_ELEM;
            *replaced = 1;
        } else {
            set_insert_node(set, node, LYXP_NODE_ELEM, set->used);
        }
    }
}

/**
 * @brief Move context \p set to a node. Handles '/' and '*', 'NAME', 'PREFIX:*', or 'PREFIX:NAME'.
 *        Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY). Indirectly context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] qname Qualified node name to move to.
 * @param[in] qname_len Length of \p qname.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
moveto_node(struct lyxp_set *set, struct lyd_node *cur_node, const char *qname, uint16_t qname_len, int when_must_eval,
            uint32_t line)
{
    uint16_t i, orig_used;
    int replaced, pref_len;
    struct lys_module *moveto_mod;
    struct lyd_node *sub;
    struct ly_ctx *ctx;
    enum lyxp_node_type root_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, 0, NULL, "path operator", print_set_type(set));
        return -1;
    }

    ctx = cur_node->schema->module->ctx;
    moveto_get_root(cur_node, when_must_eval, &root_type);

    /* prefix */
    if (strnchr(qname, ':', qname_len)) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, ctx, 1);
        if (!moveto_mod) {
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        moveto_mod = NULL;
    }

    orig_used = set->used;
    for (i = 0; (i < orig_used) && (set->type == LYXP_SET_NODE_SET); ) {
        replaced = 0;

        if ((set->node_type[i] == LYXP_NODE_ROOT_NOTIF) || (set->node_type[i] == LYXP_NODE_ROOT_RPC)) {
            moveto_node_check(set->value.nodes[i], set, i, root_type, qname, qname_len, moveto_mod, &replaced);

        } else if ((set->node_type[i] == LYXP_NODE_ROOT_CONFIG) || (set->node_type[i] == LYXP_NODE_ROOT_STATE)) {
            LY_TREE_FOR(set->value.nodes[i], sub) {
                moveto_node_check(sub, set, i, root_type, qname, qname_len, moveto_mod, &replaced);
            }

        /* skip nodes without children - leaves, leaflists, and anyxmls (ouput root will eval to true) */
        } else if (!(set->value.nodes[i]->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML))) {
            LY_TREE_FOR(set->value.nodes[i]->child, sub) {
                moveto_node_check(sub, set, i, root_type, qname, qname_len, moveto_mod, &replaced);
            }
        }

        if (!replaced) {
            /* no match */
            set_remove_node(set, i);
            --orig_used;
        } else {
            ++i;
        }
    }

    set_sort(set, cur_node, when_must_eval);
    assert(!set_sorted_dup_node_clean(set));

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to a node and all its descendants. Handles '//' and '*', 'NAME',
 *        'PREFIX:*', or 'PREFIX:NAME'. Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY).
 *        Indirectly context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] qname Qualified node name to move to.
 * @param[in] qname_len Length of \p qname.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
moveto_node_alldesc(struct lyxp_set *set, struct lyd_node *cur_node, const char *qname, uint16_t qname_len,
                    int when_must_eval, uint32_t line)
{
    uint16_t i;
    int pref_len, all = 0, replace, match;
    struct lyd_node *next, *elem, *start;
    struct lys_module *moveto_mod;
    struct ly_ctx *ctx;
    enum lyxp_node_type root_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, 0, NULL, "path operator", print_set_type(set));
        return -1;
    }

    ctx = cur_node->schema->module->ctx;
    moveto_get_root(cur_node, when_must_eval, &root_type);

    /* prefix */
    if (strnchr(qname, ':', qname_len)) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, ctx, 1);
        if (!moveto_mod) {
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        moveto_mod = 0;
    }

    /* replace the original nodes (and throws away all text and attr nodes, root is replaced by a child) */
    if (moveto_node(set, cur_node, "*", 1, when_must_eval, line)) {
        return -1;
    }

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    /* this loop traverses all the nodes in the set and addds/keeps only
     * those that match qname */
    for (i = 0; i < set->used; ) {
        /* TREE DFS */
        start = set->value.nodes[i];
        replace = 0;
        for (elem = next = start; elem; elem = next) {

            /* context check */
            if (((root_type == LYXP_NODE_ROOT_CONFIG) && (elem->schema->flags & LYS_CONFIG_R))
                    || ((root_type == LYXP_NODE_ROOT_OUTPUT) && (elem->schema->parent->nodetype == LYS_INPUT))
                    || ((root_type != LYXP_NODE_ROOT_NOTIF) && (elem->schema->nodetype == LYS_NOTIF))
                    || ((root_type != LYXP_NODE_ROOT_RPC) && (elem->schema->nodetype == LYS_RPC))) {
                goto skip_children;
            }

            match = 1;

            /* module check */
            if (moveto_mod) {
                if (lys_node_module(elem->schema) != moveto_mod) {
                    /* no match */
                    match = 0;
                }
            }

            /* name check */
            if (!all && (strncmp(elem->schema->name, qname, qname_len) || elem->schema->name[qname_len])) {
                /* no match */
                match = 0;
            }

            if (match && (elem != start)) {
                if (set_dup_node_check(set, elem, LYXP_NODE_ELEM, i) > -1) {
                    /* we'll process it later */
                    goto skip_children;
                } else if (replace) {
                    set->value.nodes[i] = elem;
                    assert(set->node_type[i] == LYXP_NODE_ELEM);
                    replace = 0;
                } else {
                    set_insert_node(set, elem, LYXP_NODE_ELEM, i + 1);
                    ++i;
                }
            } else if (!match && (elem == start)) {
                /* we need to replace a node that is already in the set */
                replace = 1;
            }

            /* TREE DFS NEXT ELEM */
            /* select element for the next run - children first */
            next = elem->child;
            if (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYXML)) {
                next = NULL;
            }
            if (!next) {
skip_children:
                /* no children, so try siblings, but only if it's not the start,
                 * that is considered to be the root and it's siblings are not traversed */
                if (elem != start) {
                    next = elem->next;
                } else {
                    break;
                }
            }
            while (!next) {
                /* no siblings, go back through the parents */
                if (elem->parent == start) {
                    /* we are done, no next element to process */
                    break;
                }
                /* parent is already processed, go to its sibling */
                elem = elem->parent;
                next = elem->next;
            }
        }

        if (replace) {
            set_remove_node(set, i);
        } else {
            ++i;
        }
    }

    set_sort(set, cur_node, when_must_eval);
    assert(!set_sorted_dup_node_clean(set));

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to an attribute. Handles '/' and '@*', '@NAME', '@PREFIX:*',
 *        or '@PREFIX:NAME'. Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY).
 *        Indirectly context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] qname Qualified node name to move to.
 * @param[in] qname_len Length of \p qname.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
moveto_attr(struct lyxp_set *set, struct lyd_node *cur_node, const char *qname, uint16_t qname_len, int when_must_eval,
            uint32_t line)
{
    uint16_t i;
    int replaced, all = 0, pref_len;
    struct lys_module *moveto_mod;
    struct lyd_attr *sub;
    struct ly_ctx *ctx;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, 0, NULL, "path operator", print_set_type(set));
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    /* prefix */
    if (strnchr(qname, ':', qname_len)) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, ctx, 1);
        if (!moveto_mod) {
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        pref_len = 0;
    }

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    for (i = 0; i < set->used; ) {
        replaced = 0;

        /* only attributes of an elem can be in the result, skip all the rest;
         * our attributes are always qualified */
        if (pref_len && set->node_type[i] == LYXP_NODE_ELEM) {
            LY_TREE_FOR(set->value.nodes[i]->attr, sub) {

                /* check "namespace" */
                if (sub->module != moveto_mod) {
                    /* no match */
                    continue;
                }

                if (all || (!strncmp(sub->name, qname, qname_len) && !sub->name[qname_len])) {
                    /* match */
                    if (!replaced) {
                        set->value.attrs[i] = sub;
                        set->node_type[i] = LYXP_NODE_ATTR;
                        replaced = 1;
                    } else {
                        set_insert_node(set, (struct lyd_node *)sub, LYXP_NODE_ATTR, i + 1);
                        ++i;
                    }
                }
            }
        }

        if (!replaced) {
            /* no match */
            set_remove_node(set, i);
        } else {
            ++i;
        }
    }

    /* no need to sort */
    (void)when_must_eval; /* suppress unused variable warning */
    assert(!set_sort(set, cur_node, when_must_eval));
    assert(!set_sorted_dup_node_clean(set));

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set1 to union with \p set2. \p set2 is emptied afterwards.
 *        Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY). Context position aware.
 *
 * @param[in,out] set1 Set to use for the result.
 * @param[in] set2 Set that is copied to \p set1.
 * @param[in] cur_node Original context node.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
moveto_union(struct lyxp_set *set1, struct lyxp_set *set2, struct lyd_node *cur_node, int when_must_eval,
             uint32_t line)
{
    if (((set1->type != LYXP_SET_NODE_SET) && (set1->type != LYXP_SET_EMPTY))
            || ((set2->type != LYXP_SET_NODE_SET) && (set2->type != LYXP_SET_EMPTY))) {
        LOGVAL(LYE_XPATH_INOP_2, line, 0, NULL, "union", print_set_type(set1), print_set_type(set2));
        return -1;
    }

    /* set2 is empty or both set1 and set2 */
    if (set2->type == LYXP_SET_EMPTY) {
        return EXIT_SUCCESS;
    }

    if (set1->type == LYXP_SET_EMPTY) {
        memcpy(set1, set2, sizeof *set1);
        /* dynamic memory belongs to set1 now, do not free */
        set2->type = LYXP_SET_EMPTY;
        return EXIT_SUCCESS;
    }

    /* remove all other nodes */
    if (set1->pos || set2->pos) {
        assert(set1->pos && set2->pos);

        if (set1->pos > 1) {
            set1->value.nodes[0] = set1->value.nodes[set1->pos - 1];
            set1->node_type[0] = set1->node_type[set1->pos - 1];
        }
        set1->used = 1;

        if (set2->pos > 1) {
            set2->value.nodes[0] = set2->value.nodes[set2->pos - 1];
            set2->node_type[0] = set2->node_type[set2->pos - 1];
        }
        set2->used = 1;
    }

    /* make sure there is enough memory */
    if (set1->size - set1->used < set2->used) {
        set1->size = set1->used + set2->used;
        set1->value.nodes = realloc(set1->value.nodes, set1->size * sizeof *set1->value.nodes);
        set1->node_type = realloc(set1->node_type, set1->size * sizeof *set1->node_type);
    }

    /* copy nodes */
    memcpy(&set1->value.nodes[set1->used], set2->value.nodes, set2->used * sizeof *set2->value.nodes);
    memcpy(&set1->node_type[set1->used], set2->node_type, set2->used * sizeof *set2->node_type);
    set1->used += set2->used;

    lyxp_set_cast(set2, LYXP_SET_EMPTY, cur_node, when_must_eval);

    /* sort, remove duplicates */
    set_sort(set1, cur_node, when_must_eval);
    set_sorted_dup_node_clean(set1);

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to an attribute in any of the descendants. Handles '//' and '@*',
 *        '@NAME', '@PREFIX:*', or '@PREFIX:NAME'. Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY).
 *        Indirectly context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] qname Qualified node name to move to.
 * @param[in] qname_len Length of \p qname.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
moveto_attr_alldesc(struct lyxp_set *set, struct lyd_node *cur_node, const char *qname, uint16_t qname_len,
                    int when_must_eval, uint32_t line)
{
    uint16_t i;
    int pref_len, replaced, all = 0;
    struct lyd_attr *sub;
    struct lys_module *moveto_mod;
    struct lyxp_set *set_all_desc = NULL;
    struct ly_ctx *ctx;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, 0, NULL, "path operator", print_set_type(set));
        return -1;
    }

    ctx = cur_node->schema->module->ctx;

    /* prefix */
    if (strnchr(qname, ':', qname_len)) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, ctx, 1);
        if (!moveto_mod) {
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        moveto_mod = 0;
    }

    /* can be optimized similarly to moveto_node_alldesc() and save considerable amount of memory,
     * but it likely won't be used much, so it's a waste of time */
    /* copy the context */
    set_all_desc = set_copy(set, ctx);
    /* get all descendant nodes (the original context nodes are removed) */
    if (moveto_node_alldesc(set_all_desc, cur_node, "*", 1, when_must_eval, line)) {
        lyxp_set_free(set_all_desc, ctx);
        return -1;
    }
    /* prepend the original context nodes */
    if (moveto_union(set, set_all_desc, cur_node, when_must_eval, line)) {
        lyxp_set_free(set_all_desc, ctx);
        return -1;
    }
    lyxp_set_free(set_all_desc, ctx);

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    for (i = 0; i < set->used; ) {
        replaced = 0;

        /* only attributes of an elem can be in the result, skip all the rest,
         * we have all attributes qualified in lyd tree */
        if (moveto_mod && set->node_type[i] == LYXP_NODE_ELEM) {
            LY_TREE_FOR(set->value.nodes[i]->attr, sub) {
                /* check "namespace" */
                if (sub->module != moveto_mod) {
                    /* no match */
                    continue;
                }

                if (all || (!strncmp(sub->name, qname, qname_len) && !sub->name[qname_len])) {
                    /* match */
                    if (!replaced) {
                        set->value.attrs[i] = sub;
                        set->node_type[i] = LYXP_NODE_ATTR;
                        replaced = 1;
                    } else {
                        set_insert_node(set, (struct lyd_node *)sub, LYXP_NODE_ATTR, i + 1);
                        ++i;
                    }
                }
            }
        }

        if (!replaced) {
            /* no match */
            set_remove_node(set, i);
        } else {
            ++i;
        }
    }

    /* no need to sort */
    assert(!set_sort(set, cur_node, when_must_eval));
    assert(!set_sorted_dup_node_clean(set));

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to self. Handles '/' or '//' and '.'. Result is LYXP_SET_NODE_SET
 *        (or LYXP_SET_EMPTY). Indirectly context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] all_desc Whether to go to all descendants ('//') or not ('/').
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
moveto_self(struct lyxp_set *set, struct lyd_node *cur_node, int all_desc, int when_must_eval, uint32_t line)
{
    struct lyd_node *sub;
    uint16_t i, cont_i;
    enum lyxp_node_type root_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, 0, NULL, "path operator", print_set_type(set));
        return -1;
    }

    /* nothing to do */
    if (!all_desc) {
        return EXIT_SUCCESS;
    }

    moveto_get_root(cur_node, when_must_eval, &root_type);

    /* add all the children, they get added recursively */
    for (i = 0; i < set->used; ++i) {
        cont_i = 0;

        /* do not touch attributes and text nodes */
        if ((set->node_type[i] == LYXP_NODE_TEXT) || (set->node_type[i] == LYXP_NODE_ATTR)) {
            continue;
        }

        /* skip anyxmls */
        if (set->value.nodes[i]->schema->nodetype == LYS_ANYXML) {
            continue;
        }

        /* add all the children ... */
        if (!(set->value.nodes[i]->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            LY_TREE_FOR(set->value.nodes[i]->child, sub) {
                /* context check */
                if (((root_type == LYXP_NODE_ROOT_CONFIG) && (sub->schema->flags & LYS_CONFIG_R))
                        || ((root_type == LYXP_NODE_ROOT_OUTPUT) && (sub->schema->parent->nodetype == LYS_INPUT))
                        || ((root_type != LYXP_NODE_ROOT_NOTIF) && (sub->schema->nodetype == LYS_NOTIF))
                        || ((root_type != LYXP_NODE_ROOT_RPC) && (sub->schema->nodetype == LYS_RPC))) {
                    continue;
                }

                if (set_dup_node_check(set, sub, LYXP_NODE_ELEM, -1) == -1) {
                    set_insert_node(set, sub, LYXP_NODE_ELEM, i + cont_i + 1);
                    ++cont_i;
                }
            }

        /* ... or add their text node, ... */
        } else {
            /* ... but only non-empty */
            sub = set->value.nodes[i];
            if (((struct lyd_node_leaf_list *)sub)->value_str) {
                if (set_dup_node_check(set, sub, LYXP_NODE_TEXT, -1) == -1) {
                    set_insert_node(set, sub, LYXP_NODE_TEXT, i + 1);
                }
            }
        }
    }

    set_sort(set, cur_node, when_must_eval);
    assert(!set_sorted_dup_node_clean(set));
    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to parent. Handles '/' or '//' and '..'. Result is LYXP_SET_NODE_SET
 *        (or LYXP_SET_EMPTY). Indirectly context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] all_desc Whether to go to all descendants ('//') or not ('/').
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
moveto_parent(struct lyxp_set *set, struct lyd_node *cur_node, int all_desc, int when_must_eval, uint32_t line)
{
    uint16_t i;
    struct lyd_node *node, *new_node, *root;
    enum lyxp_node_type root_type, new_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, 0, NULL, "path operator", print_set_type(set));
        return -1;
    }

    if (all_desc) {
        /* <path>//.. == <path>//./.. */
        if (moveto_self(set, cur_node, 1, when_must_eval, line)) {
            return -1;
        }
    }

    root = moveto_get_root(cur_node, when_must_eval, &root_type);

    for (i = 0; i < set->used; ) {
        node = set->value.nodes[i];

        if (set->node_type[i] == LYXP_NODE_ELEM) {
            new_node = node->parent;
        } else if (set->node_type[i] == LYXP_NODE_TEXT) {
            new_node = node;
        } else if (set->node_type[i] == LYXP_NODE_ATTR) {
            new_node = lyd_attr_parent(root, set->value.attrs[i]);
            if (!new_node) {
                LOGINT;
                return -1;
            }
        } else {
            /* root does not have a parent */
            set_remove_node(set, i);
            continue;
        }

        /* node already there can also be the root */
        if (root == node) {
            if (node->schema->nodetype == LYS_RPC) {
                new_type = LYXP_NODE_ROOT_RPC;
            } else if (node->schema->nodetype == LYS_NOTIF) {
                new_type = LYXP_NODE_ROOT_NOTIF;
            } else {
                if (cur_node->schema->flags & LYS_CONFIG_W) {
                    new_type = LYXP_NODE_ROOT_CONFIG;
                } else {
                    assert(cur_node->schema->flags & LYS_CONFIG_R);
                    new_type = LYXP_NODE_ROOT_STATE;
                }
            }
            new_node = node;

        /* node has no parent */
        } else if (!new_node) {
            if (cur_node->schema->flags & LYS_CONFIG_W) {
                new_type = LYXP_NODE_ROOT_CONFIG;
            } else {
                assert(cur_node->schema->flags & LYS_CONFIG_R);
                new_type = LYXP_NODE_ROOT_STATE;
            }
#ifndef NDEBUG
            for (; node->prev->next; node = node->prev);
            if (node != root) {
                LOGINT;
            }
#endif
            new_node = root;

        /* new node is the root (not interesting except this one case) */
        } else if ((new_node == root) && (root_type == LYXP_NODE_ROOT_OUTPUT)) {
            assert(new_node->schema->nodetype == LYS_RPC);
            new_type = LYXP_NODE_ROOT_OUTPUT;

        /* node has a standard parent (it can equal the root, it's not the root yet since they are fake) */
        } else {
            new_type = LYXP_NODE_ELEM;
        }

        assert((new_type == LYXP_NODE_ELEM) || ((new_type == root_type) && (new_node == root)));

        if (set_dup_node_check(set, new_node, new_type, -1) > -1) {
            set_remove_node(set, i);
        } else {
            set->node_type[i] = new_type;
            set->value.nodes[i] = new_node;

            ++i;
        }
    }

    set_sort(set, cur_node, when_must_eval);
    assert(!set_sorted_dup_node_clean(set));
    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to the result of a comparison. Handles '=', '!=', '<=', '<', '>=', or '>'.
 *        Result is LYXP_SET_BOOLEAN. Indirectly context position aware.
 *
 * @param[in,out] set1 Set to use for the result.
 * @param[in] set2 Set acting as the second operand for \p op.
 * @param[in] op Comparison operator to process.
 * @param[in] cur_node Original context node.
 */
static void
moveto_op_comp(struct lyxp_set *set1, struct lyxp_set *set2, const char *op, struct lyd_node *cur_node,
               int when_must_eval)
{
    /*
     * NODE SET + NODE SET = STRING + STRING  /1 STRING, 2 STRING
     * NODE SET + STRING = STRING + STRING    /1 STRING (2 STRING)
     * NODE SET + NUMBER = NUMBER + NUMBER    /1 NUMBER (2 NUMBER)
     * NODE SET + BOOLEAN = BOOLEAN + BOOLEAN /1 BOOLEAN (2 BOOLEAN)
     * STRING + NODE SET = STRING + STRING    /(1 STRING) 2 STRING
     * NUMBER + NODE SET = NUMBER + NUMBER    /(1 NUMBER) 2 NUMBER
     * BOOLEAN + NODE SET = BOOLEAN + BOOLEAN /(1 BOOLEAN) 2 BOOLEAN
     *
     * '=' or '!='
     * BOOLEAN + BOOLEAN
     * BOOLEAN + STRING = BOOLEAN + BOOLEAN   /(1 BOOLEAN) 2 BOOLEAN
     * BOOLEAN + NUMBER = BOOLEAN + BOOLEAN   /(1 BOOLEAN) 2 BOOLEAN
     * STRING + BOOLEAN = BOOLEAN + BOOLEAN   /1 BOOLEAN (2 BOOLEAN)
     * NUMBER + BOOLEAN = BOOLEAN + BOOLEAN   /1 BOOLEAN (2 BOOLEAN)
     * NUMBER + NUMBER
     * NUMBER + STRING = NUMBER + NUMBER      /(1 NUMBER) 2 NUMBER
     * STRING + NUMBER = NUMBER + NUMBER      /1 NUMBER (2 NUMBER)
     * STRING + STRING
     *
     * '<=', '<', '>=', '>'
     * NUMBER + NUMBER
     * BOOLEAN + BOOLEAN = NUMBER + NUMBER    /1 NUMBER, 2 NUMBER
     * BOOLEAN + NUMBER = NUMBER + NUMBER     /1 NUMBER (2 NUMBER)
     * BOOLEAN + STRING = NUMBER + NUMBER     /1 NUMBER, 2 NUMBER
     * NUMBER + STRING = NUMBER + NUMBER      /(1 NUMBER) 2 NUMBER
     * STRING + STRING = NUMBER + NUMBER      /1 NUMBER, 2 NUMBER
     * STRING + NUMBER = NUMBER + NUMBER      /1 NUMBER (2 NUMBER)
     * NUMBER + BOOLEAN = NUMBER + NUMBER     /(1 NUMBER) 2 NUMBER
     * STRING + BOOLEAN = NUMBER + NUMBER     /(1 NUMBER) 2 NUMBER
     */
    int result;
    struct ly_ctx *ctx;

    ctx = cur_node->schema->module->ctx;

    /* we can evaluate it immediately */
    if ((set1->type == set2->type) && (set1->type != LYXP_SET_EMPTY) && (set1->type != LYXP_SET_NODE_SET)
            && (((op[0] == '=') || (op[0] == '!')) || ((set1->type != LYXP_SET_BOOLEAN) && (set1->type != LYXP_SET_STRING)))) {

        /* compute result */
        if (op[0] == '=') {
            if (set1->type == LYXP_SET_BOOLEAN) {
                result = (set1->value.bool == set2->value.bool);
            } else if (set1->type == LYXP_SET_NUMBER) {
                result = (set1->value.num == set2->value.num);
            } else {
                result = (ly_strequal(set1->value.str, set2->value.str, 1));
            }
        } else if (op[0] == '!') {
            if (set1->type == LYXP_SET_BOOLEAN) {
                result = (set1->value.bool != set2->value.bool);
            } else if (set1->type == LYXP_SET_NUMBER) {
                result = (set1->value.num != set2->value.num);
            } else {
                result = (!ly_strequal(set1->value.str, set2->value.str, 1));
            }
        } else {
            if (set1->type != LYXP_SET_NUMBER) {
                LOGINT;
                return;
            }

            if (op[0] == '<') {
                if (op[1] == '=') {
                    result = (set1->value.num <= set2->value.num);
                } else {
                    result = (set1->value.num < set2->value.num);
                }
            } else {
                if (op[1] == '=') {
                    result = (set1->value.num >= set2->value.num);
                } else {
                    result = (set1->value.num > set2->value.num);
                }
            }
        }

        /* assign result */
        if (result) {
            set_fill_boolean(set1, 1, ctx);
        } else {
            set_fill_boolean(set1, 0, ctx);
        }

        return;
    }

    /* convert first */
    if (((set1->type == LYXP_SET_NODE_SET) || (set1->type == LYXP_SET_EMPTY) || (set1->type == LYXP_SET_STRING))
            && ((set2->type == LYXP_SET_NODE_SET) || (set2->type == LYXP_SET_EMPTY) || (set2->type == LYXP_SET_STRING))
            && ((set1->type != LYXP_SET_STRING) || (set2->type != LYXP_SET_STRING))) {
        lyxp_set_cast(set1, LYXP_SET_STRING, cur_node, when_must_eval);
        lyxp_set_cast(set2, LYXP_SET_STRING, cur_node, when_must_eval);

    } else if ((((set1->type == LYXP_SET_NODE_SET) || (set1->type == LYXP_SET_EMPTY) || (set1->type == LYXP_SET_BOOLEAN))
            && ((set2->type == LYXP_SET_NODE_SET) || (set2->type == LYXP_SET_EMPTY) || (set2->type == LYXP_SET_BOOLEAN)))
            || (((op[0] == '=') || (op[0] == '!')) && ((set1->type == LYXP_SET_BOOLEAN) || (set2->type == LYXP_SET_BOOLEAN)))) {
        lyxp_set_cast(set1, LYXP_SET_BOOLEAN, cur_node, when_must_eval);
        lyxp_set_cast(set2, LYXP_SET_BOOLEAN, cur_node, when_must_eval);

    } else {
        lyxp_set_cast(set1, LYXP_SET_NUMBER, cur_node, when_must_eval);
        lyxp_set_cast(set2, LYXP_SET_NUMBER, cur_node, when_must_eval);
    }

    /* now we can evaluate */
    moveto_op_comp(set1, set2, op, cur_node, when_must_eval);
}

/**
 * @brief Move context \p set to the result of a basic operation. Handles '+', '-', unary '-', '*', 'div',
 *        or 'mod'. Result is LYXP_SET_NUMBER. Indirectly context position aware.
 *
 * @param[in,out] set1 Set to use for the result.
 * @param[in] set2 Set acting as the second operand for \p op.
 * @param[in] op Operator to process.
 * @param[in] cur_node Original context node.
 */
static void
moveto_op_math(struct lyxp_set *set1, struct lyxp_set *set2, const char *op, struct lyd_node *cur_node,
               int when_must_eval)
{
    struct ly_ctx *ctx;

    ctx = cur_node->schema->module->ctx;

    /* unary '-' */
    if (!set2 && (op[0] == '-')) {
        lyxp_set_cast(set1, LYXP_SET_NUMBER, cur_node, when_must_eval);
        set1->value.num *= -1;
        lyxp_set_free(set2, ctx);
        return;
    }

    assert(set1 && set2);

    lyxp_set_cast(set1, LYXP_SET_NUMBER, cur_node, when_must_eval);
    lyxp_set_cast(set2, LYXP_SET_NUMBER, cur_node, when_must_eval);

    switch (op[0]) {
    /* '+' */
    case '+':
        set1->value.num += set2->value.num;
        break;

    /* '-' */
    case '-':
        set1->value.num -= set2->value.num;
        break;

    /* '*' */
    case '*':
        set1->value.num *= set2->value.num;
        break;

    /* 'div' */
    case 'd':
        set1->value.num /= set2->value.num;
        break;

    /* 'mod' */
    case 'm':
        set1->value.num = ((long long)set1->value.num) % ((long long)set2->value.num);
        break;

    default:
        LOGINT;
        break;
    }
}

/* MAY BE REIMPLEMENTED ONCE
 * moveto_schema functions
 *

static struct lys_node *
moveto_schema_get_root(struct lys_node *cur_node)
{
    for (; cur_node; cur_node = cur_node->parent) {
        if (cur_node->nodetype & (LYS_NOTIF | LYS_RPC | LYS_OUTPUT)) {
            break;
        }
    }

    return cur_node;
}

static void
moveto_schema_root(struct lyxp_set *set, struct lys_node *cur_node)
{
    struct lys_node *root;
    int is_output;

    if (!set) {
        return;
    }

    if (!cur_node) {
        LOGINT;
        return;
    }

    lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node->module->ctx);

    root = moveto_schema_get_root(cur_node, &is_output);

    if (!root) {
        set_insert_node(set, root, LYXP_NODE_ROOT_TOP, 0);
    } else if (root->nodetype == LYS_NOTIF) {
        set_insert_node(set, root, LYXP_NODE_ROOT_NOTIF, 0);
    } else if (root->nodetype == LYS_RPC) {
        set_insert_node(set, root, LYXP_NODE_ROOT_RPC, 0);
    } else {
        assert(root->nodetype == LYS_OUTPUT);
        set_insert_node(set, root, LYXP_NODE_ROOT_OUTPUT, 0);
    }
}

static void
moveto_schema_node_check(struct lys_node *node, struct lyxp_set *set, uint16_t i, struct lys_node *cur_node,
                         const char *qname, uint16_t qname_len, struct lys_module *moveto_mod, int *replaced)
{
    struct lys_module *cur_mod;
    struct lys_node *child;

    if (node->nodetype & (LYS_GROUPING | LYS_AUGMENT)) {
        return;
    }

    if (node->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE | LYS_INPUT | LYS_OUTPUT)) {
        LY_TREE_FOR(node->child, child) {
            moveto_schema_node_check(child, set, i, cur_node, qname, qname_len, moveto_mod, replaced);
        }
        return;
    }

    * module check *
    if (moveto_mod) {
        cur_mod = node->module;
        if (cur_mod->type) {
            cur_mod = ((struct lys_submodule *)cur_mod)->belongsto;
        }
        if (cur_mod != moveto_mod) {
            return;
        }
    }

    * context check *
    if ((cur_node->flags & LYS_CONFIG_W) && (node->flags & LYS_CONFIG_R)) {
        return;
    }

    * name check *
    if (((qname[0] == '*') && (qname_len == 1))
            || (!strncmp(node->name, qname, qname_len) && !node->name[qname_len])) {
        if (!(*replaced)) {
            set->value.nodes[i] = (struct lyd_node *)node;
            set->node_type[i] = LYXP_NODE_ELEM;
            *replaced = 1;
        } else {
            set_insert_node(set, node, LYXP_NODE_ELEM, set->used);
        }
    }
}

static int
moveto_schema_node(struct lyxp_set *set, struct lys_node *cur_node, const char *qname, uint16_t qname_len,
                   uint32_t line)
{
    uint16_t i, orig_used, j;
    int replaced, pref_len;
    struct lys_module *moveto_mod;
    struct lys_node *sub;
    struct ly_ctx *ctx;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, "path operator", print_set_type(set));
        return -1;
    }

    ctx = cur_node->module->ctx;

    * prefix *
    if (strnchr(qname, ':', qname_len)) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, ctx, 1);
        if (!moveto_mod) {
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        moveto_mod = NULL;
    }

    orig_used = set->used;
    for (i = 0; i < orig_used; ) {
        replaced = 0;

        if (set->node_type[i] == LYXP_NODE_ROOT_TOP) {
            for (j = 0; j < ctx->models.used; ++j) {
                LY_TREE_FOR(ctx->models.list[j]->data, sub) {
                    * LYS_GROUPING and LYS_USES handled inside *
                    moveto_schema_node_check(sub, set, i, cur_node, qname, qname_len, moveto_mod, &replaced);
                }
            }
        } else if ((set->node_type[i] == LYXP_NODE_ROOT_NOTIF) || (set->node_type[i] == LYXP_NODE_ROOT_RPC)) {
            LY_TREE_FOR((struct lys_node *)set->value.nodes[i], sub) {
                moveto_schema_node_check(sub, set, i, cur_node, qname, qname_len, moveto_mod, &replaced);
            }
        } else if ((set->node_type[i] == LYXP_NODE_ROOT_OUTPUT) || (set->node_type[i] == LYXP_NODE_ELEM)) {
            LY_TREE_FOR(((struct lys_node *)set->value.nodes[i])->child, sub) {
                moveto_schema_node_check(sub, set, i, cur_node, qname, qname_len, moveto_mod, &replaced);
            }
        }
        * LYXP_NODE_TEXT gets removed *

        if (!replaced) {
            * no match *
            set_remove_node(set, i);
            --orig_used;
        } else {
            ++i;
        }
    }

    set_sort(set, 1);
    assert(!set_sorted_dup_node_clean(set));

    return EXIT_SUCCESS;
}

static int
moveto_schema_node_alldesc(struct lyxp_set *set, struct lys_node *cur_node, const char *qname, uint16_t qname_len,
                           uint32_t line)
{
    uint16_t i;
    int pref_len, all = 0, replace, match;
    struct lys_node *next, *elem, *start;
    struct lys_module *moveto_mod, *cur_mod;
    struct ly_ctx *ctx;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, "path operator", print_set_type(set));
        return -1;
    }

    ctx = cur_node->module->ctx;

    * prefix *
    if (strnchr(qname, ':', qname_len)) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, ctx, 1);
        if (!moveto_mod) {
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        pref_len = 0;
    }

    * replace the original nodes (and throw away all text nodes, root nodes are replaced by their children) *
    if (moveto_schema_node(set, cur_node, "*", 1, line)) {
        return -1;
    }

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    * this loop traverses all the nodes in the set and addds/keeps only
     * those that match qname *
    for (i = 0; i < set->used; ) {
        * TREE DFS *
        start = (struct lys_node *)set->value.nodes[i];
        replace = 0;
        for (elem = next = start; elem; elem = next) {
            * it should not be possible to find them here *
            assert(!(elem->nodetype & (LYS_NOTIF | LYS_RPC)));

            * duplicated nodes skipped, context check *
            if ((elem->nodetype & (LYS_GROUPING | LYS_AUGMENT))
                    || ((cur_node->flags & LYS_CONFIG_W) && (elem->flags & LYS_CONFIG_R))){
                goto skip_children;
            }

            * schema-only (not data) elements are skipped this way *
            if (elem->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE | LYS_INPUT | LYS_OUTPUT)) {
                match = 0;
            } else {
                match = 1;
            }

            * module check *
            if (pref_len) {
                cur_mod = elem->module;
                if (cur_mod->type) {
                    cur_mod = ((struct lys_submodule *)cur_mod)->belongsto;
                }
                if (cur_mod != moveto_mod) {
                    * no match *
                    match = 0;
                }
            }

            * name check *
            if (!all && (strncmp(elem->name, qname, qname_len) || elem->name[qname_len])) {
                * no match *
                match = 0;
            }

            if (match && (elem != start)) {
                if (set_dup_node_check(set, elem, LYXP_NODE_ELEM, i) > -1) {
                    * we'll process it later *
                    goto skip_children;
                } else if (replace) {
                    set->value.nodes[i] = (struct lyd_node *)elem;
                    assert(set->node_type[i] == LYXP_NODE_ELEM);
                    replace = 0;
                } else {
                    set_insert_node(set, elem, LYXP_NODE_ELEM, i + 1);
                    ++i;
                }
            } else if (!match && (elem == start)) {
                * we need to replace a node that is already in the set *
                replace = 1;
            }

            * TREE DFS NEXT ELEM *
            * select element for the next run - children first *
            next = elem->child;
            if (!next) {
skip_children:
                * no children, so try siblings, but only if it's not the start,
                 * that is considered to be the root and it's siblings are not traversed *
                if (elem != start) {
                    next = elem->next;
                } else {
                    break;
                }
            }
            while (!next) {
                * no siblings, go back through the parents *
                if (elem->parent == start) {
                    * we are done, no next element to process *
                    break;
                }
                * parent is already processed, go to its sibling *
                elem = elem->parent;
                * this was an augment, behave accordingly *
                if (elem->nodetype == LYS_AUGMENT) {
                    elem = ((struct lys_node_augment *)elem)->target;
                }
                next = elem->next;
            }
        }

        if (replace) {
            set_remove_node(set, i);
        } else {
            ++i;
        }
    }

    set_sort(set, 1);
    assert(!set_sorted_dup_node_clean(set));

    return EXIT_SUCCESS;
}

static int
moveto_schema_union(struct lyxp_set *set1, struct lyxp_set *set2, uint32_t line)
{
    if (((set1->type != LYXP_SET_NODE_SET) && (set1->type != LYXP_SET_EMPTY))
            || ((set2->type != LYXP_SET_NODE_SET) && (set2->type != LYXP_SET_EMPTY))) {
        LOGVAL(LYE_XPATH_INOP_2, line, "union", print_set_type(set1), print_set_type(set2));
        return -1;
    }

    * set2 is empty or both set1 and set2 *
    if (set2->type == LYXP_SET_EMPTY) {
        return EXIT_SUCCESS;
    }

    if (set1->type == LYXP_SET_EMPTY) {
        memcpy(set1, set2, sizeof *set1);
        * dynamic memory belongs to set1 now, do not free *
        set2->type = LYXP_SET_EMPTY;
        return EXIT_SUCCESS;
    }

    * remove all other nodes *
    if (set1->pos || set2->pos) {
        assert(set1->pos && set2->pos);

        if (set1->pos > 1) {
            set1->value.nodes[0] = set1->value.nodes[set1->pos - 1];
            set1->node_type[0] = set1->node_type[set1->pos - 1];
        }
        set1->used = 1;

        if (set2->pos > 1) {
            set2->value.nodes[0] = set2->value.nodes[set2->pos - 1];
            set2->node_type[0] = set2->node_type[set2->pos - 1];
        }
        set2->used = 1;
    }

    * make sure there is enough memory *
    if (set1->size - set1->used < set2->used) {
        set1->size = set1->used + set2->used;
        set1->value.nodes = realloc(set1->value.nodes, set1->size * sizeof *set1->value.nodes);
        set1->node_type = realloc(set1->node_type, set1->size * sizeof *set1->node_type);
    }

    * copy nodes *
    memcpy(&set1->value.nodes[set1->used], set2->value.nodes, set2->used * sizeof *set2->value.nodes);
    memcpy(&set1->node_type[set1->used], set2->node_type, set2->used * sizeof *set2->node_type);
    set1->used += set2->used;

    * empty set2, NULL ctx is fine *
    lyxp_set_cast(set2, LYXP_SET_EMPTY, NULL);

    * sort, remove duplicates *
    set_sort(set1, 1);
    set_sorted_dup_node_clean(set1);

    return EXIT_SUCCESS;
}

static void
moveto_schema_self_check(struct lys_node *node, struct lyxp_set *set, uint16_t i, struct lys_node *cur_node,
                         uint16_t *cur_i)
{
    struct lys_node *child;

    if (node->nodetype & (LYS_GROUPING | LYS_AUGMENT)) {
        return;
    }

    if (node->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE | LYS_INPUT | LYS_OUTPUT)) {
        LY_TREE_FOR(node->child, child) {
            moveto_schema_self_check(child, set, i, cur_node, cur_i);
        }
        return;
    }

    if ((cur_node->flags & LYS_CONFIG_W) && (node->flags & LYS_CONFIG_R)) {
        return;
    }

    if (set_dup_node_check(set, node, LYXP_NODE_ELEM, -1) == -1) {
        set_insert_node(set, node, LYXP_NODE_ELEM, i + (*cur_i) + 1);
        ++cur_i;
    }
}

static int
moveto_schema_self(struct lyxp_set *set, struct lys_node *cur_node, int all_desc, uint32_t line)
{
    struct lys_node *sub;
    struct ly_ctx *ctx;
    uint16_t i, cur_i, j;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, "path operator", print_set_type(set));
        return -1;
    }

    * nothing to do *
    if (!all_desc) {
        return EXIT_SUCCESS;
    }

    * add all the children, they get added recursively *
    for (i = 0; i < set->used; ++i) {
        cur_i = 0;

        if (set->node_type[i] == LYXP_NODE_ROOT_TOP) {
            for (j = 0; j < ctx->models.used; ++j) {
                LY_TREE_FOR(ctx->models.list[j]->data, sub) {
                    * LYS_GROUPING and LYS_USES handled inside *
                    moveto_schema_self_check(sub, set, i, cur_node, &cur_i);
                }
            }
        } else if ((set->node_type[i] == LYXP_NODE_ROOT_NOTIF) || (set->node_type[i] == LYXP_NODE_ROOT_RPC)) {
            LY_TREE_FOR((struct lys_node *)set->value.nodes[i], sub) {
                moveto_schema_self_check(sub, set, i, cur_node, &cur_i);
            }
        } else if ((set->node_type[i] == LYXP_NODE_ROOT_OUTPUT) || (set->node_type[i] == LYXP_NODE_ELEM)) {
            if (((struct lys_node *)set->value.nodes[i])->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
                sub = (struct lys_node *)set->value.nodes[i];
                if (set_dup_node_check(set, sub, LYXP_NODE_TEXT, -1) == -1) {
                    set_insert_node(set, sub, LYXP_NODE_TEXT, i + 1);
                }
            } else { * LYS_ANYXML can go here, it has no children anyway *
                LY_TREE_FOR(((struct lys_node *)set->value.nodes[i])->child, sub) {
                    moveto_schema_self_check(sub, set, i, cur_node, &cur_i);
                }
            }
        }
    }

    set_sort(set, 1);
    assert(!set_sorted_dup_node_clean(set));
    return EXIT_SUCCESS;
}

static int
moveto_schema_parent(struct lyxp_set *set, struct lys_node *cur_node, int all_desc, uint32_t line)
{
    uint16_t i;
    int is_output;
    struct lys_node *new_node, *root;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, line, "path operator", print_set_type(set));
        return -1;
    }

    if (all_desc) {
        * <path>//.. == <path>//./.. *
        if (moveto_schema_self(set, cur_node, 1, line)) {
            return -1;
        }
    }

    for (i = 0; i < set->used; ) {
        if ((set->node_type[i] == LYXP_NODE_ROOT_TOP) || (set->node_type[i] == LYXP_NODE_ROOT_NOTIF)
                || (set->node_type[i] == LYXP_NODE_ROOT_RPC) || (set->node_type[i] == LYXP_NODE_ROOT_OUTPUT)) {
            * root does not have a parent *
            set_remove_node(set, i);
            continue;
        } else if (set->node_type[i] == LYXP_NODE_ELEM) {
            new_node = lys_parent((struct lys_node *)set->value.nodes[i]);
        } else { * LYXP_NODE_TEXT *
            set->node_type[i] = LYXP_NODE_ELEM;
            continue;
        }

        * check for duplicate *
        if ((set_dup_node_check(set, new_node, LYXP_NODE_ELEM, -1) > -1)
                || (!new_node && (set_dup_node_check(set, new_node, LYXP_NODE_ROOT, -1) > -1))) {
            set_remove_node(set, i);
            continue;
        }

        * update it (but first decide what kind of root it is) *
        root = moveto_schema_get_root(cur_node);
        if (root == (struct lys_node *)set->value.nodes[i]) {
            if (((struct lys_node *)set->value.nodes[i])->nodetype == LYS_NOTIF) {
                set->node_type[i] = LYXP_NODE_ROOT_NOTIF;
            } else {
                assert(((struct lys_node *)set->value.nodes[i])->nodetype == LYS_RPC);
                set->node_type[i] = LYXP_NODE_ROOT_RPC;
            }
        } else if (!new_node) {
            assert(!root);
            set->node_type[i] = LYXP_NODE_ROOT_TOP;
            set->value.nodes[i] = NULL;
        } else if (new_node->nodetype == LYS_OUTPUT) {
            assert(root && root->nodetype == LYS_OUTPUT);
            set->node_type[i] = LYXP_NODE_ROOT_OUTPUT;
            set->value.nodes[i] = (struct lyd_node *)new_node;
        } else {
            set->node_type[i] = LYXP_NODE_ELEM;
            set->value.nodes[i] = (struct lyd_node *)new_node;
        }

        ++i;
    }

    set_sort(set, 1);
    assert(!set_sorted_dup_node_clean(set));
    return EXIT_SUCCESS;
}

*/

/*
 * eval functions
 *
 * They execute a parsed XPath expression on some data subtree.
 */

/**
 * @brief Evaluate Literal. Logs directly on error.
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] ctx libyang context with the dictionary.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static void
eval_literal(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyxp_set *set, struct ly_ctx *ctx)
{
    if (set) {
        if (exp->tok_len[*exp_idx] == 2) {
            set_fill_string(set, "", 0, ctx);
        } else {
            set_fill_string(set, &exp->expr[exp->expr_pos[*exp_idx] + 1], exp->tok_len[*exp_idx] - 2, ctx);
        }
    }
    LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
    ++(*exp_idx);
}

/**
 * @brief Evaluate NodeTest. Logs directly on error.
 *
 * [5] NodeTest ::= NameTest | NodeType '(' ')'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in] attr_axis Whether to search attributes or standard nodes.
 * @param[in] all_desc Whether to search all the descendants or children only.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_node_test(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, int attr_axis, int all_desc,
               struct lyxp_set *set, int when_must_eval, uint32_t line)
{
    int rc = 0;

    switch (exp->tokens[*exp_idx]) {
    case LYXP_TOKEN_NAMETEST:
        if (attr_axis) {
            if (all_desc) {
                rc = moveto_attr_alldesc(set, cur_node, &exp->expr[exp->expr_pos[*exp_idx]],
                                         exp->tok_len[*exp_idx], when_must_eval, line);
            } else {
                rc = moveto_attr(set, cur_node, &exp->expr[exp->expr_pos[*exp_idx]], exp->tok_len[*exp_idx],
                                 when_must_eval, line);
            }
        } else {
            if (all_desc) {
                rc = moveto_node_alldesc(set, cur_node, &exp->expr[exp->expr_pos[*exp_idx]],
                                         exp->tok_len[*exp_idx], when_must_eval, line);
            } else {
                rc = moveto_node(set, cur_node, &exp->expr[exp->expr_pos[*exp_idx]], exp->tok_len[*exp_idx],
                                 when_must_eval, line);
            }
        }
        if (rc) {
            return rc;
        }

        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);
        break;

    case LYXP_TOKEN_NODETYPE:
        if (set) {
            assert(exp->tok_len[*exp_idx] == 4);
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "node", 4)) {
                if (xpath_node(NULL, 0, cur_node, set, when_must_eval, line)) {
                    return -1;
                }
            } else {
                assert(!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "text", 4));
                if (xpath_text(NULL, 0, cur_node, set, when_must_eval, line)) {
                    return -1;
                }
            }
        }
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        /* '(' */
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        /* ')' */
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);
        break;

    default:
        LOGINT;
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate Predicate. Logs directly on error.
 *
 * [6] Predicate ::= '[' Expr ']'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_predicate(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
               int when_must_eval, uint32_t line)
{
    uint16_t i, j, orig_i, orig_exp, brack2_exp;
    uint8_t **pred_repeat, rep_size;
    struct lyxp_set *set2, *orig_set;
    struct ly_ctx *ctx;

    ctx = cur_node->schema->module->ctx;

    /* '[' */
    LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
           print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
    ++(*exp_idx);

    if (!set) {
        if (eval_expr(exp, exp_idx, cur_node, NULL, when_must_eval, line)) {
            return -1;
        }
    } else if (set->type == LYXP_SET_NODE_SET) {
        orig_set = set_copy(set, ctx);
        orig_exp = *exp_idx;

        /* find the predicate end */
        for (brack2_exp = orig_exp; exp->tokens[brack2_exp] != LYXP_TOKEN_BRACK2; ++brack2_exp);

        /* copy predicate repeats, since they get deleted each time (probably not an ideal solution) */
        pred_repeat = calloc(brack2_exp - orig_exp, sizeof *pred_repeat);
        if (!pred_repeat) {
            LOGMEM;
            lyxp_set_free(orig_set, ctx);
            return -1;
        }
        for (j = 0; j < brack2_exp - orig_exp; ++j) {
            if (exp->repeat[orig_exp + j]) {
                for (rep_size = 0; exp->repeat[orig_exp + j][rep_size]; ++rep_size);
                ++rep_size;
                pred_repeat[j] = malloc(rep_size * sizeof **pred_repeat);
                if (!pred_repeat[j]) {
                    LOGMEM;
                    for (i = 0; i < j; ++i) {
                        free(pred_repeat[j]);
                    }
                    free(pred_repeat);
                    lyxp_set_free(orig_set, ctx);
                    return -1;
                }
                memcpy(pred_repeat[j], exp->repeat[orig_exp + j], rep_size * sizeof **pred_repeat);
            }
        }

        i = 0;
        for (orig_i = 0; orig_i < orig_set->used; ++orig_i) {
            set2 = set_copy(orig_set, ctx);
            set2->pos = orig_i + 1;
            *exp_idx = orig_exp;

            /* replace repeats */
            for (j = 0; j < brack2_exp - orig_exp; ++j) {
                if (pred_repeat[j]) {
                    for (rep_size = 0; pred_repeat[j][rep_size]; ++rep_size);
                    ++rep_size;
                    memcpy(exp->repeat[orig_exp + j], pred_repeat[j], rep_size * sizeof **pred_repeat);
                }
            }

            if (eval_expr(exp, exp_idx, cur_node, set2, when_must_eval, line)) {
                for (j = 0; j < brack2_exp - orig_exp; ++j) {
                    free(pred_repeat[j]);
                }
                free(pred_repeat);
                lyxp_set_free(set2, ctx);
                lyxp_set_free(orig_set, ctx);
                return -1;
            }

            /* number is a position */
            if (set2->type == LYXP_SET_NUMBER) {
                if ((long long)set2->value.num == orig_i + 1) {
                    set2->value.num = 1;
                } else {
                    set2->value.num = 0;
                }
            }
            lyxp_set_cast(set2, LYXP_SET_BOOLEAN, cur_node, when_must_eval);

            /* predicate satisfied or not? */
            if (set2->value.bool) {
                ++i;
            } else {
                set_remove_node(set, i);
            }
            lyxp_set_free(set2, ctx);
        }

        /* free predicate repeats */
        for (j = 0; j < brack2_exp - orig_exp; ++j) {
            free(pred_repeat[j]);
        }
        free(pred_repeat);

        lyxp_set_free(orig_set, ctx);
    } else {
        set2 = set_copy(set, ctx);

        if (eval_expr(exp, exp_idx, cur_node, set2, when_must_eval, line)) {
            lyxp_set_free(set2, ctx);
            return -1;
        }

        lyxp_set_cast(set2, LYXP_SET_BOOLEAN, cur_node, when_must_eval);
        if (!set2->value.bool) {
            lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, when_must_eval);
        }
        lyxp_set_free(set2, ctx);
    }

    /* ']' */
    LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
           print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
    ++(*exp_idx);

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate RelativeLocationPath. Logs directly on error.
 *
 * [3] RelativeLocationPath ::= Step | RelativeLocationPath '/' Step | RelativeLocationPath '//' Step
 * [4] Step ::= '@'? NodeTest Predicate* | '.' | '..'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in] all_desc Whether to search all the descendants or children only.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
eval_relative_location_path(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, int all_desc,
                            struct lyxp_set *set, int when_must_eval, uint32_t line)
{
    int attr_axis;

    goto step;
    do {
        /* evaluate '/' or '//' */
        if (exp->tok_len[*exp_idx] == 1) {
            all_desc = 0;
        } else {
            assert(exp->tok_len[*exp_idx] == 2);
            all_desc = 1;
        }
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

step:
        /* Step */
        attr_axis = 0;
        switch (exp->tokens[*exp_idx]) {
        case LYXP_TOKEN_DOT:
            /* evaluate '.' */
            if (moveto_self(set, cur_node, all_desc, when_must_eval, line)) {
                return -1;
            }
            LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
            ++(*exp_idx);
            break;
        case LYXP_TOKEN_DDOT:
            /* evaluate '..' */
            if (moveto_parent(set, cur_node, all_desc, when_must_eval, line)) {
                return -1;
            }
            LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
            ++(*exp_idx);
            break;

        case LYXP_TOKEN_AT:
            /* evaluate '@' */
            attr_axis = 1;
            LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
            ++(*exp_idx);

            /* fall through */
        case LYXP_TOKEN_NAMETEST:
        case LYXP_TOKEN_NODETYPE:
            if (eval_node_test(exp, exp_idx, cur_node, attr_axis, all_desc, set, when_must_eval, line)) {
                return -1;
            }
            while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_BRACK1)) {
                if (eval_predicate(exp, exp_idx, cur_node, set, when_must_eval, line)) {
                    return -1;
                }
            }
            break;
        default:
            LOGINT;
            return -1;
        }
    } while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_PATH));

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate AbsoluteLocationPath. Logs directly on error.
 *
 * [2] AbsoluteLocationPath ::= '/' RelativeLocationPath? | '//' RelativeLocationPath
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_absolute_location_path(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node,
                            struct lyxp_set *set, int when_must_eval, uint32_t line)
{
    int all_desc;

    if (set) {
        /* no matter what tokens follow, we need to be at the root */
        moveto_root(set, cur_node, when_must_eval);
    }

    /* '/' RelativeLocationPath? */
    if (exp->tok_len[*exp_idx] == 1) {
        /* evaluate '/' - deferred */
        all_desc = 0;
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_NONE, UINT_MAX)) {
            return EXIT_SUCCESS;
        }
        switch (exp->tokens[*exp_idx]) {
        case LYXP_TOKEN_DOT:
        case LYXP_TOKEN_DDOT:
        case LYXP_TOKEN_AT:
        case LYXP_TOKEN_NAMETEST:
        case LYXP_TOKEN_NODETYPE:
            if (eval_relative_location_path(exp, exp_idx, cur_node, all_desc, set, when_must_eval, line)) {
                return -1;
            }
        default:
            break;
        }

    /* '//' RelativeLocationPath */
    } else {
        /* evaluate '//' - deferred so as not to waste memory by remembering all the nodes */
        all_desc = 1;
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        if (eval_relative_location_path(exp, exp_idx, cur_node, all_desc, set, when_must_eval, line)) {
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate FunctionCall. Logs directly on error.
 *
 * [8] FunctionCall ::= FunctionName '(' ( Expr ( ',' Expr )* )? ')'
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_function_call(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
                   int when_must_eval, uint32_t line)
{
    int rc = EXIT_FAILURE;
    int (*xpath_func)(struct lyxp_set *, uint16_t, struct lyd_node *, struct lyxp_set *, int, uint32_t) = NULL;
    uint16_t arg_count = 0, i;
    struct lyxp_set *args = NULL;

    if (set) {
        /* FunctionName */
        switch (exp->tok_len[*exp_idx]) {
        case 3:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "not", 3)) {
                xpath_func = &xpath_not;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "sum", 3)) {
                xpath_func = &xpath_sum;
            }
            break;
        case 4:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "lang", 4)) {
                xpath_func = &xpath_lang;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "last", 4)) {
                xpath_func = &xpath_last;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "true", 4)) {
                xpath_func = &xpath_true;
            }
            break;
        case 5:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "count", 5)) {
                xpath_func = &xpath_count;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "false", 5)) {
                xpath_func = &xpath_false;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "floor", 5)) {
                xpath_func = &xpath_floor;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "round", 5)) {
                xpath_func = &xpath_round;
            }
            break;
        case 6:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "concat", 6)) {
                xpath_func = &xpath_concat;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "number", 6)) {
                xpath_func = &xpath_number;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "string", 6)) {
                xpath_func = &xpath_string;
            }
            break;
        case 7:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "boolean", 7)) {
                xpath_func = &xpath_boolean;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "ceiling", 7)) {
                xpath_func = &xpath_ceiling;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "current", 7)) {
                xpath_func = &xpath_current;
            }
            break;
        case 8:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "contains", 8)) {
                xpath_func = &xpath_contains;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "position", 8)) {
                xpath_func = &xpath_position;
            }
            break;
        case 9:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "substring", 9)) {
                xpath_func = &xpath_substring;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "translate", 9)) {
                xpath_func = &xpath_translate;
            }
            break;
        case 10:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "local-name", 10)) {
                xpath_func = &xpath_local_name;
            }
            break;
        case 11:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "starts-with", 11)) {
                xpath_func = &xpath_starts_with;
            }
            break;
        case 13:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "namespace-uri", 13)) {
                xpath_func = &xpath_namespace_uri;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "string-length", 13)) {
                xpath_func = &xpath_string_length;
            }
            break;
        case 15:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "normalize-space", 15)) {
                xpath_func = &xpath_normalize_space;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "substring-after", 15)) {
                xpath_func = &xpath_substring_after;
            }
            break;
        case 16:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "substring-before", 16)) {
                xpath_func = &xpath_substring_before;
            }
            break;
        }

        if (!xpath_func) {
            LOGVAL(LYE_SPEC, line, 0, NULL,
                   "Unknown XPath function \"%.*s\".", exp->tok_len[*exp_idx], &exp->expr[exp->expr_pos[*exp_idx]]);
            return -1;
        }
    }

    LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
    ++(*exp_idx);

    /* '(' */
    LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
    ++(*exp_idx);

    /* ( Expr ( ',' Expr )* )? */
    if (exp->tokens[*exp_idx] != LYXP_TOKEN_PAR2) {
        if (set) {
            arg_count = 1;
            args = calloc(1, sizeof *args);
            if (!args) {
                LOGMEM;
                goto cleanup;
            }
        }
        if ((rc = eval_expr(exp, exp_idx, cur_node, args, when_must_eval, line))) {
            goto cleanup;
        }
    }
    while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_COMMA)) {
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        if (set) {
            ++arg_count;
            args = ly_realloc(args, arg_count * sizeof *args);
            if (!args) {
                LOGMEM;
                goto cleanup;
            }
            memset(&args[arg_count - 1], 0, sizeof *args);

            if ((rc = eval_expr(exp, exp_idx, cur_node, &args[arg_count - 1], when_must_eval, line))) {
                goto cleanup;
            }
        } else {
            if ((rc = eval_expr(exp, exp_idx, cur_node, NULL, when_must_eval, line))) {
                goto cleanup;
            }
        }
    }

    /* ')' */
    LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
    ++(*exp_idx);

    if (set) {
        /* evaluate function */
        rc = xpath_func(args, arg_count, cur_node, set, when_must_eval, line);
    } else {
        rc = EXIT_SUCCESS;
    }

cleanup:
    for (i = 0; i < arg_count; ++i) {
        lyxp_set_cast(&args[i], LYXP_SET_EMPTY, cur_node, when_must_eval);
    }
    free(args);

    return rc;
}

/**
 * @brief Evaluate Number. Logs directly on error.
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] any_node Any node from the data.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_number(struct lyxp_expr *exp, uint16_t *exp_idx, struct ly_ctx *ctx, struct lyxp_set *set, uint32_t line)
{
    long double num;
    char *endptr;

    if (set) {
        errno = 0;
        num = strtold(&exp->expr[exp->expr_pos[*exp_idx]], &endptr);
        if (errno) {
            LOGVAL(LYE_SPEC, line, 0, NULL, "Failed to convert \"%.*s\" into a long double (%s).",
                   exp->tok_len[*exp_idx], &exp->expr[exp->expr_pos[*exp_idx]], strerror(errno));
            return -1;
        } else if (endptr - &exp->expr[exp->expr_pos[*exp_idx]] != exp->tok_len[*exp_idx]) {
            LOGVAL(LYE_SPEC, line, 0, NULL, "Failed to convert \"%.*s\" into a long double.",
                   exp->tok_len[*exp_idx], &exp->expr[exp->expr_pos[*exp_idx]]);
            return -1;
        }

        set_fill_number(set, num, ctx);
    }

    LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
    ++(*exp_idx);
    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate PathExpr. Logs directly on error.
 *
 * [9] PathExpr ::= LocationPath | PrimaryExpr Predicate*
 *                 | PrimaryExpr Predicate* '/' RelativeLocationPath
 *                 | PrimaryExpr Predicate* '//' RelativeLocationPath
 * [1] LocationPath ::= RelativeLocationPath | AbsoluteLocationPath
 * [7] PrimaryExpr ::= '(' Expr ')' | Literal | Number | FunctionCall
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_path_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
               int when_must_eval, uint32_t line)
{
    int all_desc;
    struct ly_ctx *ctx;

    ctx = cur_node->schema->module->ctx;

    switch (exp->tokens[*exp_idx]) {
    case LYXP_TOKEN_PAR1:
        /* '(' Expr ')' */

        /* '(' */
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        /* Expr */
        if (eval_expr(exp, exp_idx, cur_node, set, when_must_eval, line)) {
            return -1;
        }

        /* ')' */
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        goto predicate;
        break;

    case LYXP_TOKEN_DOT:
    case LYXP_TOKEN_DDOT:
    case LYXP_TOKEN_AT:
    case LYXP_TOKEN_NAMETEST:
    case LYXP_TOKEN_NODETYPE:
        /* RelativeLocationPath */
        if (eval_relative_location_path(exp, exp_idx, cur_node, 0, set, when_must_eval, line)) {
            return -1;
        }
        break;

    case LYXP_TOKEN_FUNCNAME:
        /* FunctionCall */
        if (eval_function_call(exp, exp_idx, cur_node, set, when_must_eval, line)) {
            return -1;
        }

        goto predicate;
        break;

    case LYXP_TOKEN_OPERATOR_PATH:
        /* AbsoluteLocationPath */
        if (eval_absolute_location_path(exp, exp_idx, cur_node, set, when_must_eval, line)) {
            return -1;
        }
        break;

    case LYXP_TOKEN_LITERAL:
        /* Literal */
        eval_literal(exp, exp_idx, set, ctx);

        goto predicate;
        break;

    case LYXP_TOKEN_NUMBER:
        /* Number */
        if (eval_number(exp, exp_idx, ctx, set, line)) {
            return -1;
        }

        goto predicate;
        break;

    default:
        LOGVAL(LYE_XPATH_INTOK, line, 0, NULL,
               print_token(exp->tokens[*exp_idx]), &exp->expr[exp->expr_pos[*exp_idx]]);
        return -1;
    }

    return EXIT_SUCCESS;

predicate:
    /* Predicate* */
    while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_BRACK1)) {
        if (eval_predicate(exp, exp_idx, cur_node, set, when_must_eval, line)) {
            return -1;
        }
    }

    /* ('/' or '//') RelativeLocationPath */
    if ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_PATH)) {

        /* evaluate '/' or '//' */
        if (exp->tok_len[*exp_idx] == 1) {
            all_desc = 0;
        } else {
            assert(exp->tok_len[*exp_idx] == 2);
            all_desc = 1;
        }

        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        if (eval_relative_location_path(exp, exp_idx, cur_node, all_desc, set, when_must_eval, line)) {
            return -1;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate UnaryExpr. Logs directly on error.
 *
 * [16] UnaryExpr ::= UnionExpr | '-' UnaryExpr
 * [17] UnionExpr ::= PathExpr | UnionExpr '|' PathExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_unary_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
                int when_must_eval, uint32_t line)
{
    int unary_minus;
    uint16_t op_exp;
    struct lyxp_set orig_set, set2;
    struct ly_ctx *ctx;

    ctx = cur_node->schema->module->ctx;

    /* ('-')* */
    unary_minus = -1;
    while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_MATH, UINT_MAX)
            && (exp->expr[exp->expr_pos[*exp_idx]] == '-')) {
        if (unary_minus == -1) {
            unary_minus = *exp_idx;
        } else {
            /* double '-' makes '+', ignore */
            unary_minus = -1;
        }
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);
    }

    orig_set.type = LYXP_SET_EMPTY;
    set2.type = LYXP_SET_EMPTY;

    op_exp = exp_repeat_peek(exp, *exp_idx);
    if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_UNI)) {
        /* there is an operator */
        exp_repeat_pop(exp, *exp_idx);
        set_fill_set(&orig_set, set, ctx);
    } else {
        op_exp = 0;
    }

    /* PathExpr */
    if (eval_path_expr(exp, exp_idx, cur_node, set, when_must_eval, line)) {
        lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
        return -1;
    }

    /* ('|' PathExpr)* */
    while (op_exp) {
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        op_exp = exp_repeat_peek(exp, *exp_idx);
        if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_UNI)) {
            /* there is another operator */
            exp_repeat_pop(exp, *exp_idx);
        } else {
            op_exp = 0;
        }

        if (!set) {
            if (eval_path_expr(exp, exp_idx, cur_node, NULL, when_must_eval, line)) {
                return -1;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set, ctx);
        if (eval_path_expr(exp, exp_idx, cur_node, &set2, when_must_eval, line)) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, when_must_eval);
            return -1;
        }

        /* eval */
        if (moveto_union(set, &set2, cur_node, when_must_eval, line)) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, when_must_eval);
            return -1;
        }
    }

    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
    /* now we have all the unions in set and no other memory allocated */

    if (set && (unary_minus > -1)) {
        moveto_op_math(set, NULL, &exp->expr[exp->expr_pos[unary_minus]], cur_node, when_must_eval);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate MultiplicativeExpr. Logs directly on error.
 *
 * [15] MultiplicativeExpr ::= UnaryExpr
 *                     | MultiplicativeExpr '*' UnaryExpr
 *                     | MultiplicativeExpr 'div' UnaryExpr
 *                     | MultiplicativeExpr 'mod' UnaryExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_multiplicative_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
                         int when_must_eval, uint32_t line)
{
    uint16_t this_op, op_exp;
    struct lyxp_set orig_set, set2;
    struct ly_ctx *ctx;

    ctx = cur_node->schema->module->ctx;

    orig_set.type = LYXP_SET_EMPTY;
    set2.type = LYXP_SET_EMPTY;

    op_exp = exp_repeat_peek(exp, *exp_idx);
    if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_MATH)
            && ((exp->expr[exp->expr_pos[op_exp]] == '*') || (exp->tok_len[op_exp] == 3))) {
        /* there is an operator */
        exp_repeat_pop(exp, *exp_idx);
        set_fill_set(&orig_set, set, ctx);
    } else {
        op_exp = 0;
    }

    /* UnaryExpr */
    if (eval_unary_expr(exp, exp_idx, cur_node, set, when_must_eval, line)) {
        lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
        return -1;
    }

    /* ('*' / 'div' / 'mod' UnaryExpr)* */
    while (op_exp) {
        this_op = *exp_idx;

        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        op_exp = exp_repeat_peek(exp, *exp_idx);
        if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_MATH)
                && ((exp->expr[exp->expr_pos[op_exp]] == '*') || (exp->tok_len[op_exp] == 3))) {
            /* there is another operator */
            exp_repeat_pop(exp, *exp_idx);
        } else {
            op_exp = 0;
        }

        if (!set) {
            if (eval_unary_expr(exp, exp_idx, cur_node, NULL, when_must_eval, line)) {
                return -1;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set, ctx);
        if (eval_unary_expr(exp, exp_idx, cur_node, &set2, when_must_eval, line)) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, when_must_eval);
            return -1;
        }

        /* eval */
        moveto_op_math(set, &set2, &exp->expr[exp->expr_pos[this_op]], cur_node, when_must_eval);
    }

    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
    lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, when_must_eval);
    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate AdditiveExpr. Logs directly on error.
 *
 * [14] AdditiveExpr ::= MultiplicativeExpr
 *                     | AdditiveExpr '+' MultiplicativeExpr
 *                     | AdditiveExpr '-' MultiplicativeExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_additive_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
                   int when_must_eval, uint32_t line)
{
    uint16_t this_op, op_exp;
    struct lyxp_set orig_set, set2;
    struct ly_ctx *ctx;

    ctx = cur_node->schema->module->ctx;

    orig_set.type = LYXP_SET_EMPTY;
    set2.type = LYXP_SET_EMPTY;

    op_exp = exp_repeat_peek(exp, *exp_idx);
    if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_MATH)
            && ((exp->expr[exp->expr_pos[op_exp]] == '+') || (exp->expr[exp->expr_pos[op_exp]] == '-'))) {
        /* there is an operator */
        exp_repeat_pop(exp, *exp_idx);
        set_fill_set(&orig_set, set, ctx);
    } else {
        op_exp = 0;
    }

    /* MultiplicativeExpr */
    if (eval_multiplicative_expr(exp, exp_idx, cur_node, set, when_must_eval, line)) {
        lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
        return -1;
    }

    /* ('+' / '-' MultiplicativeExpr)* */
    while (op_exp) {
        this_op = *exp_idx;

        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        op_exp = exp_repeat_peek(exp, *exp_idx);
        if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_MATH)
                && ((exp->expr[exp->expr_pos[op_exp]] == '+') || (exp->expr[exp->expr_pos[op_exp]] == '-'))) {
            /* there is another operator */
            exp_repeat_pop(exp, *exp_idx);
        } else {
            op_exp = 0;
        }

        if (!set) {
            if (eval_multiplicative_expr(exp, exp_idx, cur_node, NULL, when_must_eval, line)) {
                return -1;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set, ctx);
        if (eval_multiplicative_expr(exp, exp_idx, cur_node, &set2, when_must_eval, line)) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, when_must_eval);
            return -1;
        }

        /* eval */
        moveto_op_math(set, &set2, &exp->expr[exp->expr_pos[this_op]], cur_node, when_must_eval);
    }

    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
    lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, when_must_eval);
    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate RelationalExpr. Logs directly on error.
 *
 * [13] RelationalExpr ::= AdditiveExpr
 *                       | RelationalExpr '<' AdditiveExpr
 *                       | RelationalExpr '>' AdditiveExpr
 *                       | RelationalExpr '<=' AdditiveExpr
 *                       | RelationalExpr '>=' AdditiveExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_relational_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
                     int when_must_eval, uint32_t line)
{
    uint16_t this_op, op_exp;
    struct lyxp_set orig_set, set2;
    struct ly_ctx *ctx;

    ctx = cur_node->schema->module->ctx;

    orig_set.type = LYXP_SET_EMPTY;
    set2.type = LYXP_SET_EMPTY;

    op_exp = exp_repeat_peek(exp, *exp_idx);
    if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_COMP)
            && ((exp->expr[exp->expr_pos[op_exp]] == '<') || (exp->expr[exp->expr_pos[op_exp]] == '>'))) {
        /* there is an operator */
        exp_repeat_pop(exp, *exp_idx);
        set_fill_set(&orig_set, set, ctx);
    } else {
        op_exp = 0;
    }

    /* AdditiveExpr */
    if (eval_additive_expr(exp, exp_idx, cur_node, set, when_must_eval, line)) {
        lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
        return -1;
    }

    /* ('<' / '>' / '<=' / '>=' AdditiveExpr)* */
    while (op_exp) {
        this_op = *exp_idx;

        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        op_exp = exp_repeat_peek(exp, *exp_idx);
        if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_COMP)
                && ((exp->expr[exp->expr_pos[op_exp]] == '<') || (exp->expr[exp->expr_pos[op_exp]] == '>'))) {
            /* there is another operator */
            exp_repeat_pop(exp, *exp_idx);
        } else {
            op_exp = 0;
        }

        if (!set) {
            if (eval_relational_expr(exp, exp_idx, cur_node, NULL, when_must_eval, line)) {
                return -1;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set, ctx);
        if (eval_additive_expr(exp, exp_idx, cur_node, &set2, when_must_eval, line)) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, when_must_eval);
            return -1;
        }

        /* eval */
        moveto_op_comp(set, &set2, &exp->expr[exp->expr_pos[this_op]], cur_node, when_must_eval);
    }

    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
    lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, when_must_eval);
    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate EqualityExpr. Logs directly on error.
 *
 * [12] EqualityExpr ::= RelationalExpr | EqualityExpr '=' RelationalExpr
 *                     | EqualityExpr '!=' RelationalExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_equality_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
                   int when_must_eval, uint32_t line)
{
    uint16_t this_op, op_exp;
    struct lyxp_set orig_set, set2;
    struct ly_ctx *ctx;

    ctx = cur_node->schema->module->ctx;

    orig_set.type = LYXP_SET_EMPTY;
    set2.type = LYXP_SET_EMPTY;

    op_exp = exp_repeat_peek(exp, *exp_idx);
    if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_COMP)
            && ((exp->expr[exp->expr_pos[op_exp]] == '=') || (exp->expr[exp->expr_pos[op_exp]] == '!'))) {
        /* there is an operator */
        exp_repeat_pop(exp, *exp_idx);
        set_fill_set(&orig_set, set, ctx);
    } else {
        op_exp = 0;
    }

    /* RelationalExpr */
    if (eval_relational_expr(exp, exp_idx, cur_node, set, when_must_eval, line)) {
        lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
        return -1;
    }

    /* ('=' / '!=' RelationalExpr)* */
    while (op_exp) {
        this_op = *exp_idx;

        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        op_exp = exp_repeat_peek(exp, *exp_idx);
        if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_COMP)
                && ((exp->expr[exp->expr_pos[op_exp]] == '=') || (exp->expr[exp->expr_pos[op_exp]] == '!'))) {
            /* there is another operator */
            exp_repeat_pop(exp, *exp_idx);
        } else {
            op_exp = 0;
        }

        if (!set) {
            if (eval_relational_expr(exp, exp_idx, cur_node, NULL, when_must_eval, line)) {
                return -1;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set, ctx);
        if (eval_relational_expr(exp, exp_idx, cur_node, &set2, when_must_eval, line)) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, when_must_eval);
            return -1;
        }

        /* eval */
        moveto_op_comp(set, &set2, &exp->expr[exp->expr_pos[this_op]], cur_node, when_must_eval);
    }

    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
    lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, when_must_eval);
    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate AndExpr. Logs directly on error.
 *
 * [11] AndExpr ::= EqualityExpr | AndExpr 'and' EqualityExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_and_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
              int when_must_eval, uint32_t line)
{
    int is_false = 0;
    uint16_t op_exp;
    struct lyxp_set orig_set;
    struct ly_ctx *ctx;

    ctx = cur_node->schema->module->ctx;

    orig_set.type = LYXP_SET_EMPTY;

    op_exp = exp_repeat_peek(exp, *exp_idx);
    if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_LOG) && (exp->tok_len[op_exp] == 3)) {
        /* there is an operator */
        exp_repeat_pop(exp, *exp_idx);
        set_fill_set(&orig_set, set, ctx);
    } else {
        op_exp = 0;
    }

    /* EqualityExpr */
    if (eval_equality_expr(exp, exp_idx, cur_node, set, when_must_eval, line)) {
        lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
        return -1;
    }

    /* cast to boolean, we know that will be the final result */
    if (op_exp) {
        lyxp_set_cast(set, LYXP_SET_BOOLEAN, cur_node, when_must_eval);
        if (!set->value.bool) {
            is_false = 1;
        }
    }

    /* ('and' EqualityExpr)* */
    while (op_exp) {
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (!set || !set->value.bool ? "skipped" : "parsed"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        op_exp = exp_repeat_peek(exp, *exp_idx);
        if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_LOG) && (exp->tok_len[op_exp] == 3)) {
            /* there is another operator */
            exp_repeat_pop(exp, *exp_idx);
        } else {
            op_exp = 0;
        }

        /* lazy evaluation */
        if (is_false) {
            continue;
        }

        set_fill_set(set, &orig_set, ctx);
        if (eval_equality_expr(exp, exp_idx, cur_node, set, when_must_eval, line)) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
            return -1;
        }

        /* eval - just get boolean value actually */
        lyxp_set_cast(set, LYXP_SET_BOOLEAN, cur_node, when_must_eval);
        if (!set->value.bool) {
            is_false = 1;
        }
    }

    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
    return EXIT_SUCCESS;
}

/**
 * @brief Evaluate Expr. Logs directly on error.
 *
 * [10] Expr ::= AndExpr | Expr 'or' AndExpr
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in] cur_node Start node for the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 * @param[in] when_must_eval Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[in] line Line in the input file.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set, int when_must_eval,
          uint32_t line)
{
    int is_true = 0;
    uint16_t op_exp;
    struct lyxp_set orig_set;
    struct ly_ctx *ctx;

    ctx = cur_node->schema->module->ctx;

    orig_set.type = LYXP_SET_EMPTY;

    op_exp = exp_repeat_peek(exp, *exp_idx);
    if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_LOG) && (exp->tok_len[op_exp] == 2)) {
        /* there is an operator */
        exp_repeat_pop(exp, *exp_idx);
        set_fill_set(&orig_set, set, ctx);
    } else {
        op_exp = 0;
    }

    /* AndExpr */
    if (eval_and_expr(exp, exp_idx, cur_node, set, when_must_eval, line)) {
        lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
        return -1;
    }

    /* cast to boolean, we know that will be the final result */
    if (op_exp) {
        lyxp_set_cast(set, LYXP_SET_BOOLEAN, cur_node, when_must_eval);
        if (set->value.bool) {
            is_true = 1;
        }
    }

    /* ('or' AndExpr)* */
    while (op_exp) {
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (!set || set->value.bool ? "skipped" : "parsed"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        op_exp = exp_repeat_peek(exp, *exp_idx);
        if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_LOG) && (exp->tok_len[op_exp] == 2)) {
            /* there is another operator */
            exp_repeat_pop(exp, *exp_idx);
        } else {
            op_exp = 0;
        }

        /* lazy evaluation */
        if (is_true) {
            continue;
        }

        set_fill_set(set, &orig_set, ctx);
        if (eval_and_expr(exp, exp_idx, cur_node, set, when_must_eval, line)) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
            return -1;
        }

        /* eval - just get boolean value actually */
        lyxp_set_cast(set, LYXP_SET_BOOLEAN, cur_node, when_must_eval);
        if (set->value.bool) {
            is_true = 1;
        }
    }

    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, when_must_eval);
    return EXIT_SUCCESS;
}

int
lyxp_eval(const char *expr, const struct lyd_node *cur_node, struct lyxp_set *set, int when_must_eval, uint32_t line)
{
    struct lyxp_expr *exp;
    uint16_t exp_idx;
    int rc = -1;

    if (!expr || !cur_node || !set) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    exp = parse_expr(expr, line);
    if (exp) {
        exp_idx = 0;
        rc = reparse_expr(exp, &exp_idx, line);
        if (!rc && (exp->used > exp_idx)) {
            LOGVAL(LYE_SPEC, line, 0, NULL, "Unparsed characters \"%s\" left at the end of an XPath expression.",
                   &exp->expr[exp->expr_pos[exp_idx]]);
            rc = -1;
        }
        if (rc) {
            exp_free(exp);
        }
    }
    if (!rc && exp) {
        print_expr_struct_debug(exp);

        exp_idx = 0;
        lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, when_must_eval);
        set_insert_node(set, (struct lyd_node *)cur_node, LYXP_NODE_ELEM, 0);
        rc = eval_expr(exp, &exp_idx, (struct lyd_node *)cur_node, set, when_must_eval, line);

        exp_free(exp);
    }

    if (exp && rc) {
        LOGVAL(LYE_PATH, 0, LY_VLOG_LYD, cur_node);
    }

    return rc;
}

int
lyxp_syntax_check(const char *expr, uint32_t line)
{
    struct lyxp_expr *exp;
    uint16_t exp_idx;
    int rc = -1;

    if (!expr) {
        ly_errno = LY_EINVAL;
        return -1;
    }

    exp = parse_expr(expr, line);
    if (exp) {
        exp_idx = 0;
        rc = reparse_expr(exp, &exp_idx, line);
        if (!rc && (exp->used > exp_idx)) {
            LOGVAL(LYE_SPEC, line, 0, NULL, "Unparsed characters \"%s\" left at the end of an XPath expression.",
                   &exp->expr[exp->expr_pos[exp_idx]]);
            rc = -1;
        }
        exp_free(exp);
    }

    return rc;
}

void xml_print_node(struct lyout *out, int level, struct lyd_node *node, int toplevel);

void
lyxp_set_print_xml(FILE *f, struct lyxp_set *set)
{
    uint16_t i;
    char *str_num;
    struct lyout out;

    out.type = LYOUT_STREAM;
    out.method.f = f;

    switch (set->type) {
    case LYXP_SET_EMPTY:
        ly_print(&out, "Empty XPath set\n\n");
        break;
    case LYXP_SET_BOOLEAN:
        ly_print(&out, "Boolean XPath set:\n");
        ly_print(&out, "%s\n\n", set->value.bool ? "true" : "false");
        break;
    case LYXP_SET_STRING:
        ly_print(&out, "String XPath set:\n");
        ly_print(&out, "\"%s\"\n\n", set->value.str);
        break;
    case LYXP_SET_NUMBER:
        ly_print(&out, "Number XPath set:\n");

        if (isnan(set->value.num)) {
            str_num = strdup("NaN");
        } else if ((set->value.num == 0) || (set->value.num == -0)) {
            str_num = strdup("0");
        } else if (isinf(set->value.num) && !signbit(set->value.num)) {
            str_num = strdup("Infinity");
        } else if (isinf(set->value.num) && signbit(set->value.num)) {
            str_num = strdup("-Infinity");
        } else if ((long long)set->value.num == set->value.num) {
            asprintf(&str_num, "%lld", (long long)set->value.num);
        } else {
            asprintf(&str_num, "%03.1Lf", set->value.num);
        }
        if (!str_num) {
            LOGMEM;
            return;
        }
        ly_print(&out, "%s\n\n", str_num);
        free(str_num);
        break;
    case LYXP_SET_NODE_SET:
        ly_print(&out, "Node XPath set:\n");

        for (i = 0; i < set->used; ++i) {
            ly_print(&out, "%d. ", i + 1);
            switch (set->node_type[i]) {
            case LYXP_NODE_ROOT_CONFIG:
                ly_print(&out, "ROOT config\n\n");
                break;
            case LYXP_NODE_ROOT_STATE:
                ly_print(&out, "ROOT state\n\n");
                break;
            case LYXP_NODE_ROOT_NOTIF:
                ly_print(&out, "ROOT notification \"%s\"\n\n", set->value.nodes[i]->schema->name);
                break;
            case LYXP_NODE_ROOT_RPC:
                ly_print(&out, "ROOT rpc \"%s\"\n\n", set->value.nodes[i]->schema->name);
                break;
            case LYXP_NODE_ROOT_OUTPUT:
                ly_print(&out, "ROOT output of rpc \"%s\"\n\n", set->value.nodes[i]->schema->name);
                break;
            case LYXP_NODE_ELEM:
                ly_print(&out, "ELEM \"%s\"\n", set->value.nodes[i]->schema->name);
                xml_print_node(&out, 1, set->value.nodes[i], 1);
                ly_print(&out, "\n");
                break;
            case LYXP_NODE_TEXT:
                ly_print(&out, "TEXT \"%s\"\n\n", ((struct lyd_node_leaf_list *)set->value.nodes[i])->value_str);
                break;
            case LYXP_NODE_ATTR:
                ly_print(&out, "ATTR \"%s\" = \"%s\"\n\n", set->value.attrs[i]->name, set->value.attrs[i]->value);
                break;
            }
        }
        break;
    }
}

void
lyxp_set_cast(struct lyxp_set *set, enum lyxp_set_type target, const struct lyd_node *cur_node, int when_must_eval)
{
    char *str_num;
    long double num;
    const char *str;
    struct ly_ctx *ctx;

    if (!set || (set->type == target)) {
        return;
    }

    /* it's not possible to convert anything into a node set */
    if (target == LYXP_SET_NODE_SET) {
        LOGINT;
        return;
    }

    ctx = cur_node->schema->module->ctx;

    /* to STRING */
    if ((target == LYXP_SET_STRING) || ((target == LYXP_SET_NUMBER)
            && ((set->type == LYXP_SET_NODE_SET) || (set->type == LYXP_SET_EMPTY)))) {
        switch (set->type) {
        case LYXP_SET_NUMBER:
            if (isnan(set->value.num)) {
                set->value.str = lydict_insert(ctx, "NaN", 0);
            } else if ((set->value.num == 0) || (set->value.num == -0)) {
                set->value.str = lydict_insert(ctx, "0", 0);
            } else if (isinf(set->value.num) && !signbit(set->value.num)) {
                set->value.str = lydict_insert(ctx, "Infinity", 0);
            } else if (isinf(set->value.num) && signbit(set->value.num)) {
                set->value.str = lydict_insert(ctx, "-Infinity", 0);
            } else if ((long long)set->value.num == set->value.num) {
                asprintf(&str_num, "%lld", (long long)set->value.num);
                set->value.str = lydict_insert_zc(ctx, str_num);
            } else {
                asprintf(&str_num, "%03.1Lf", set->value.num);
                set->value.str = lydict_insert_zc(ctx, str_num);
            }
            break;
        case LYXP_SET_BOOLEAN:
            if (set->value.bool) {
                set->value.str = lydict_insert(ctx, "true", 0);
            } else {
                set->value.str = lydict_insert(ctx, "false", 0);
            }
            break;
        case LYXP_SET_NODE_SET:
            assert(set->used);

            str = cast_node_set_to_string(set, (struct lyd_node *)cur_node, when_must_eval);
            free(set->value.nodes);
            free(set->node_type);
            set->value.str = str;
            break;
        case LYXP_SET_EMPTY:
            set->value.str = lydict_insert(ctx, "", 0);
            break;
        default:
            LOGINT;
            break;
        }
        set->type = LYXP_SET_STRING;
    }

    /* to NUMBER */
    if (target == LYXP_SET_NUMBER) {
        switch (set->type) {
        case LYXP_SET_STRING:
            num = cast_string_to_number(set->value.str);
            lydict_remove(ctx, set->value.str);
            set->value.num = num;
            break;
        case LYXP_SET_BOOLEAN:
            if (set->value.bool) {
                set->value.num = 1;
            } else {
                set->value.num = 0;
            }
            break;
        default:
            LOGINT;
            break;
        }
        set->type = LYXP_SET_NUMBER;
    }

    /* to BOOLEAN */
    if (target == LYXP_SET_BOOLEAN) {
        switch (set->type) {
        case LYXP_SET_NUMBER:
            if ((set->value.num == 0) || (set->value.num == -0) || isnan(set->value.num)) {
                set->value.bool = 0;
            } else {
                set->value.bool = 1;
            }
            break;
        case LYXP_SET_STRING:
            if (set->value.str[0]) {
                set->value.bool = 1;
            } else {
                set->value.bool = 0;
            }
            break;
        case LYXP_SET_NODE_SET:
            free(set->value.nodes);
            free(set->node_type);

            assert(set->used);
            set->value.bool = 1;
            break;
        case LYXP_SET_EMPTY:
            set->value.bool = 0;
            break;
        default:
            LOGINT;
            break;
        }
        set->type = LYXP_SET_BOOLEAN;
    }

    /* to EMPTY */
    if (target == LYXP_SET_EMPTY) {
        switch (set->type) {
        case LYXP_SET_NUMBER:
        case LYXP_SET_BOOLEAN:
            /* nothing to do */
            break;
        case LYXP_SET_STRING:
            lydict_remove(ctx, set->value.str);
            break;
        case LYXP_SET_NODE_SET:
            free(set->value.nodes);
            free(set->node_type);
            break;
        default:
            LOGINT;
            break;
        }
        set->type = LYXP_SET_EMPTY;
    }
}

void
lyxp_set_free(struct lyxp_set *set, struct ly_ctx *ctx)
{
    if (!set) {
        return;
    }

    if (set->type == LYXP_SET_NODE_SET) {
        free(set->value.nodes);
        free(set->node_type);
    } else if (set->type == LYXP_SET_STRING) {
        lydict_remove(ctx, set->value.str);
    }
    free(set);
}
