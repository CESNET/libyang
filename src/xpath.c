/**
 * @file xpath.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YANG XPath evaluation functions
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
#include <pcre.h>

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
#include "parser.h"
#include "dict_private.h"

static const  struct lyd_node *moveto_get_root(const struct lyd_node *cur_node, int options,
                                               enum lyxp_node_type *root_type);
static int reparse_expr(struct lyxp_expr *exp, uint16_t *exp_idx);
static int eval_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
                     int options);

/**
 * @brief Frees a parsed XPath expression. \p exp should not be used afterwards.
 *
 * @param[in] exp Expression to free.
 */
static void
exp_free(struct lyxp_expr *exp)
{
    uint16_t i;

    if (!exp) {
        return;
    }

    free(exp->expr);
    free(exp->tokens);
    free(exp->expr_pos);
    free(exp->tok_len);
    if (exp->repeat) {
        for (i = 0; i < exp->used; ++i) {
            free(exp->repeat[i]);
        }
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
    case LYXP_SET_SNODE_SET:
        return "schema node set";
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

    if (!exp || (ly_log_level < LY_LLDBG)) {
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

#ifndef NDEBUG

/**
 * @brief Print XPath set content to debug output.
 *
 * @param[in] set Set to print.
 */
static void
print_set_debug(struct lyxp_set *set)
{
    uint32_t i;
    char *str_num;
    struct lyxp_set_nodes *item;
    struct lyxp_set_snodes *sitem;

    if (ly_log_level < LY_LLDBG) {
        return;
    }

    switch (set->type) {
    case LYXP_SET_NODE_SET:
        LOGDBG("XPATH: set NODE SET:");
        for (i = 0; i < set->used; ++i) {
            item = &set->val.nodes[i];

            switch (item->type) {
            case LYXP_NODE_ROOT:
                LOGDBG("XPATH:\t%d (pos %u): ROOT", i + 1, item->pos);
                break;
            case LYXP_NODE_ROOT_CONFIG:
                LOGDBG("XPATH:\t%d (pos %u): ROOT CONFIG", i + 1, item->pos);
                break;
            case LYXP_NODE_ELEM:
                if ((item->node->schema->nodetype == LYS_LIST)
                        && (item->node->child->schema->nodetype == LYS_LEAF)) {
                    LOGDBG("XPATH:\t%d (pos %u): ELEM %s (1st child val: %s)", i + 1, item->pos,
                           item->node->schema->name,
                           ((struct lyd_node_leaf_list *)item->node->child)->value_str);
                } else if (item->node->schema->nodetype == LYS_LEAFLIST) {
                    LOGDBG("XPATH:\t%d (pos %u): ELEM %s (val: %s)", i + 1, item->pos,
                           item->node->schema->name,
                           ((struct lyd_node_leaf_list *)item->node)->value_str);
                } else {
                    LOGDBG("XPATH:\t%d (pos %u): ELEM %s", i + 1, item->pos, item->node->schema->name);
                }
                break;
            case LYXP_NODE_TEXT:
                if (item->node->schema->nodetype & LYS_ANYDATA) {
                    LOGDBG("XPATH:\t%d (pos %u): TEXT <%s>", i + 1, item->pos,
                           item->node->schema->nodetype == LYS_ANYXML ? "anyxml" : "anydata");
                } else {
                    LOGDBG("XPATH:\t%d (pos %u): TEXT %s", i + 1, item->pos,
                           ((struct lyd_node_leaf_list *)item->node)->value_str);
                }
                break;
            case LYXP_NODE_ATTR:
                LOGDBG("XPATH:\t%d (pos %u): ATTR %s = %s", i + 1, item->pos, set->val.attrs[i].attr->name,
                       set->val.attrs[i].attr->value);
                break;
            }
        }
        break;

    case LYXP_SET_SNODE_SET:
        LOGDBG("XPATH: set SNODE SET:");
        for (i = 0; i < set->used; ++i) {
            sitem = &set->val.snodes[i];

            switch (sitem->type) {
            case LYXP_NODE_ROOT:
                LOGDBG("XPATH:\t%d (%u): ROOT", i + 1, sitem->in_ctx);
                break;
            case LYXP_NODE_ROOT_CONFIG:
                LOGDBG("XPATH:\t%d (%u): ROOT CONFIG", i + 1, sitem->in_ctx);
                break;
            case LYXP_NODE_ELEM:
                LOGDBG("XPATH:\t%d (%u): ELEM %s", i + 1, sitem->in_ctx, sitem->snode->name);
                break;
            default:
                LOGINT;
                break;
            }
        }
        break;

    case LYXP_SET_EMPTY:
        LOGDBG("XPATH: set EMPTY");
        break;

    case LYXP_SET_BOOLEAN:
        LOGDBG("XPATH: set BOOLEAN");
        LOGDBG("XPATH:\t%s", (set->val.bool ? "true" : "false"));
        break;

    case LYXP_SET_STRING:
        LOGDBG("XPATH: set STRING");
        LOGDBG("XPATH:\t%s", set->val.str);
        break;

    case LYXP_SET_NUMBER:
        LOGDBG("XPATH: set NUMBER");

        if (isnan(set->val.num)) {
            str_num = strdup("NaN");
        } else if ((set->val.num == 0) || (set->val.num == -0.0f)) {
            str_num = strdup("0");
        } else if (isinf(set->val.num) && !signbit(set->val.num)) {
            str_num = strdup("Infinity");
        } else if (isinf(set->val.num) && signbit(set->val.num)) {
            str_num = strdup("-Infinity");
        } else if ((long long)set->val.num == set->val.num) {
            if (asprintf(&str_num, "%lld", (long long)set->val.num) == -1) {
                str_num = NULL;
            }
        } else {
            if (asprintf(&str_num, "%03.1Lf", set->val.num) == -1) {
                str_num = NULL;
            }
        }

        if (!str_num) {
            LOGMEM;
            return;
        }

        LOGDBG("XPATH:\t%s", str_num);
        free(str_num);
    }
}

#endif

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
    struct lyd_node_anydata *any;

    if ((root_type == LYXP_NODE_ROOT_CONFIG) && (node->schema->flags & LYS_CONFIG_R)) {
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
    case LYS_ANYDATA:
        any = (struct lyd_node_anydata *)node;
        if (!(void*)any->value.tree) {
            /* no content */
            buf = strdup("");
        } else {
            switch (any->value_type) {
            case LYD_ANYDATA_CONSTSTRING:
            case LYD_ANYDATA_STRING:
                buf = strdup(any->value.str);
                if (!buf) {
                    LOGMEM;
                    return;
                }
                break;
            case LYD_ANYDATA_DATATREE:
                lyd_print_mem(&buf, any->value.tree, LYD_XML, LYP_WITHSIBLINGS);
                break;
            case LYD_ANYDATA_XML:
                lyxml_print_mem(&buf, any->value.xml, LYXML_PRINT_SIBLINGS);
                break;
            }
        }

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
 *
 * @return Element cast to dynamically-allocated string.
 */
static char *
cast_string_elem(struct lyd_node *node, int fake_cont, enum lyxp_node_type root_type)
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
    return str;
}

/**
 * @brief Cast a LYXP_SET_NODE_SET set into a string.
 *        Context position aware.
 *
 * @param[in] set Set to cast.
 * @param[in] cur_node Original context node.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return Cast string in the dictionary.
 */
static char *
cast_node_set_to_string(struct lyxp_set *set, struct lyd_node *cur_node, int options)
{
    enum lyxp_node_type root_type;

    if (set->val.nodes[0].node->validity & LYD_VAL_INUSE) {
        LOGVAL(LYE_XPATH_DUMMY, LY_VLOG_LYD, set->val.nodes[0].node, set->val.nodes[0].node->schema->name);
        return NULL;
    }

    moveto_get_root(cur_node, options, &root_type);

    switch (set->val.nodes[0].type) {
    case LYXP_NODE_ROOT:
    case LYXP_NODE_ROOT_CONFIG:
        return cast_string_elem(set->val.nodes[0].node, 1, root_type);
    case LYXP_NODE_ELEM:
    case LYXP_NODE_TEXT:
        return cast_string_elem(set->val.nodes[0].node, 0, root_type);
    case LYXP_NODE_ATTR:
        return strdup(set->val.attrs[0].attr->value);
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
 *
 * @return Copy of \p set.
 */
static struct lyxp_set *
set_copy(struct lyxp_set *set)
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
    if ((set->type == LYXP_SET_NODE_SET) || (set->type == LYXP_SET_SNODE_SET)) {
        ret->type = set->type;
        ret->val.nodes = malloc(set->used * sizeof *ret->val.nodes);
        if (!ret->val.nodes) {
            LOGMEM;
            free(ret);
            return NULL;
        }
        memcpy(ret->val.nodes, set->val.nodes, set->used * sizeof *ret->val.nodes);

        ret->used = ret->size = set->used;
        ret->ctx_pos = set->ctx_pos;
        ret->ctx_size = set->ctx_size;
    } else {
       memcpy(ret, set, sizeof *ret);
       if (set->type == LYXP_SET_STRING) {
           ret->val.str = strdup(set->val.str);
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
set_fill_string(struct lyxp_set *set, const char *string, uint16_t str_len)
{
    if (set->type == LYXP_SET_NODE_SET) {
        free(set->val.nodes);
    } else if (set->type == LYXP_SET_STRING) {
        free(set->val.str);
    }

    set->type = LYXP_SET_STRING;
    if ((str_len == 0) && (string[0] != '\0')) {
        string = "";
    }
    set->val.str = strndup(string, str_len);
}

/**
 * @brief Fill XPath set with a number. Any current data are disposed of.
 *
 * @param[in] set Set to fill.
 * @param[in] number Number to fill into \p set.
 */
static void
set_fill_number(struct lyxp_set *set, long double number)
{
    if (set->type == LYXP_SET_NODE_SET) {
        free(set->val.nodes);
    } else if (set->type == LYXP_SET_STRING) {
        free(set->val.str);
    }

    set->type = LYXP_SET_NUMBER;
    set->val.num = number;
}

/**
 * @brief Fill XPath set with a boolean. Any current data are disposed of.
 *
 * @param[in] set Set to fill.
 * @param[in] boolean Boolean to fill into \p set.
 */
static void
set_fill_boolean(struct lyxp_set *set, int boolean)
{
    if (set->type == LYXP_SET_NODE_SET) {
        free(set->val.nodes);
    } else if (set->type == LYXP_SET_STRING) {
        free(set->val.str);
    }

    set->type = LYXP_SET_BOOLEAN;
    set->val.bool = boolean;
}

/**
 * @brief Fill XPath set with the value from another set (deep assign).
 *        Any current data are disposed of.
 *
 * @param[in] trg Set to fill.
 * @param[in] src Source set to copy into \p trg.
 */
static void
set_fill_set(struct lyxp_set *trg, struct lyxp_set *src)
{
    if (!trg || !src) {
        return;
    }

    if (src->type == LYXP_SET_SNODE_SET) {
        trg->type = LYXP_SET_SNODE_SET;
        trg->used = src->used;
        trg->size = src->used;

        trg->val.snodes = ly_realloc(trg->val.snodes, trg->size * sizeof *trg->val.nodes);
        if (!trg->val.nodes) {
            LOGMEM;
            memset(trg, 0, sizeof *trg);
            return;
        }

        memcpy(trg->val.nodes, src->val.nodes, src->used * sizeof *src->val.nodes);
    } else if (src->type == LYXP_SET_BOOLEAN) {
        set_fill_boolean(trg, src->val.bool);
    } else if (src->type ==  LYXP_SET_NUMBER) {
        set_fill_number(trg, src->val.num);
    } else if (src->type == LYXP_SET_STRING) {
        set_fill_string(trg, src->val.str, strlen(src->val.str));
    } else {
        if (trg->type == LYXP_SET_NODE_SET) {
            free(trg->val.nodes);
        } else if (trg->type == LYXP_SET_STRING) {
            free(trg->val.str);
        }

        if (src->type == LYXP_SET_EMPTY) {
            trg->type = LYXP_SET_EMPTY;
        } else {
            assert(src->type == LYXP_SET_NODE_SET);

            trg->type = LYXP_SET_NODE_SET;
            trg->used = src->used;
            trg->size = src->used;
            trg->ctx_pos = src->ctx_pos;
            trg->ctx_size = src->ctx_size;

            trg->val.nodes = malloc(trg->used * sizeof *trg->val.nodes);
            if (!trg->val.nodes) {
                LOGMEM;
                memset(trg, 0, sizeof *trg);
                return;
            }

            memcpy(trg->val.nodes, src->val.nodes, src->used * sizeof *src->val.nodes);
        }
    }


}

static void
set_snode_clear_ctx(struct lyxp_set *set)
{
    uint32_t i;

    for (i = 0; i < set->used; ++i) {
        if (set->val.snodes[i].in_ctx == 1) {
            set->val.snodes[i].in_ctx = 0;
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
set_remove_node(struct lyxp_set *set, uint32_t idx)
{
    assert(set && (set->type == LYXP_SET_NODE_SET));
    assert(idx < set->used);

    --set->used;
    if (set->used) {
        memmove(&set->val.nodes[idx], &set->val.nodes[idx + 1],
                (set->used - idx) * sizeof *set->val.nodes);
    } else {
        free(set->val.nodes);
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
    uint32_t i;

    for (i = 0; i < set->used; ++i) {
        if ((skip_idx > -1) && (i == (unsigned)skip_idx)) {
            continue;
        }

        if ((set->val.nodes[i].node == node) && (set->val.nodes[i].type == node_type)) {
            return i;
        }
    }

    return -1;
}

static int
set_snode_dup_node_check(struct lyxp_set *set, const struct lys_node *node, enum lyxp_node_type node_type, int skip_idx)
{
    uint32_t i;

    for (i = 0; i < set->used; ++i) {
        if ((skip_idx > -1) && (i == (unsigned)skip_idx)) {
            continue;
        }

        if ((set->val.snodes[i].snode == node) && (set->val.snodes[i].type == node_type)) {
            return i;
        }
    }

    return -1;
}

static void
set_snode_merge(struct lyxp_set *set1, struct lyxp_set *set2)
{
    uint32_t orig_used, i, j;

    assert(((set1->type == LYXP_SET_SNODE_SET) || (set1->type == LYXP_SET_EMPTY))
        && ((set2->type == LYXP_SET_SNODE_SET) || (set2->type == LYXP_SET_EMPTY)));

    if (set2->type == LYXP_SET_EMPTY) {
        return;
    }

    if (set1->type == LYXP_SET_EMPTY) {
        memcpy(set1, set2, sizeof *set1);
        return;
    }

    if (set1->used + set2->used > set1->size) {
        set1->size = set1->used + set2->used;
        set1->val.snodes = ly_realloc(set1->val.snodes, set1->size * sizeof *set1->val.snodes);
        if (!set1->val.snodes) {
            LOGMEM;
            return;
        }
    }

    orig_used = set1->used;

    for (i = 0; i < set2->used; ++i) {
        for (j = 0; j < orig_used; ++j) {
            /* detect duplicities */
            if (set1->val.snodes[j].snode == set2->val.snodes[i].snode) {
                break;
            }
        }

        if (j == orig_used) {
            memcpy(&set1->val.snodes[set1->used], &set2->val.snodes[i], sizeof *set2->val.snodes);
            ++set1->used;
        }
    }

    free(set2->val.snodes);
    memset(set2, 0, sizeof *set2);
}

/**
 * @brief Insert a node into a set. Context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] node Node to insert to \p set.
 * @param[in] pos Sort position of \p node. If left 0, it is filled just before sorting.
 * @param[in] node_type Node type of \p node.
 * @param[in] idx Index in \p set to insert into.
 */
static void
set_insert_node(struct lyxp_set *set, const void *node, uint32_t pos, enum lyxp_node_type node_type, uint32_t idx)
{
    assert(set && ((set->type == LYXP_SET_NODE_SET) || (set->type == LYXP_SET_EMPTY)));

    if (set->type == LYXP_SET_EMPTY) {
        /* first item */
        if (idx) {
            /* no real harm done, but it is a bug */
            LOGINT;
            idx = 0;
        }
        set->val.nodes = malloc(LYXP_SET_SIZE_START * sizeof *set->val.nodes);
        if (!set->val.nodes) {
            LOGMEM;
            return;
        }
        set->type = LYXP_SET_NODE_SET;
        set->used = 0;
        set->size = LYXP_SET_SIZE_START;
        set->ctx_pos = 1;
        set->ctx_size = 1;
    } else {
        /* not an empty set */
        if (set->used == set->size) {

            /* set is full */
            set->val.nodes = ly_realloc(set->val.nodes, (set->size + LYXP_SET_SIZE_STEP) * sizeof *set->val.nodes);
            if (!set->val.nodes) {
                LOGMEM;
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
            memmove(&set->val.nodes[idx + 1], &set->val.nodes[idx], (set->used - idx) * sizeof *set->val.nodes);
        }
    }

    /* finally assign the value */
    set->val.nodes[idx].node = (struct lyd_node *)node;
    set->val.nodes[idx].type = node_type;
    set->val.nodes[idx].pos = pos;
    ++set->used;
}

static int
set_snode_insert_node(struct lyxp_set *set, const struct lys_node *node, enum lyxp_node_type node_type)
{
    int ret;

    assert(set->type == LYXP_SET_SNODE_SET);

    ret = set_snode_dup_node_check(set, node, node_type, -1);
    if (ret > -1) {
        set->val.snodes[ret].in_ctx = 1;
    } else {
        if (set->used == set->size) {
            set->val.snodes = ly_realloc(set->val.snodes, (set->size + LYXP_SET_SIZE_STEP) * sizeof *set->val.snodes);
            if (!set->val.snodes) {
                LOGMEM;
                return -1;
            }
            set->size += LYXP_SET_SIZE_STEP;
        }

        ret = set->used;
        set->val.snodes[ret].snode = (struct lys_node *)node;
        set->val.snodes[ret].type = node_type;
        set->val.snodes[ret].in_ctx = 1;
        ++set->used;
    }

    return ret;
}

static uint32_t
set_snode_new_in_ctx(struct lyxp_set *set)
{
    uint32_t ret_ctx, i;

    assert(set->type == LYXP_SET_SNODE_SET);

    ret_ctx = 3;
retry:
    for (i = 0; i < set->used; ++i) {
        if (set->val.snodes[i].in_ctx >= ret_ctx) {
            ret_ctx = set->val.snodes[i].in_ctx + 1;
            goto retry;
        }
    }
    for (i = 0; i < set->used; ++i) {
        if (set->val.snodes[i].in_ctx == 1) {
            set->val.snodes[i].in_ctx = ret_ctx;
        }
    }

    return ret_ctx;
}

/**
 * @brief Get unique \p node position in the data.
 *
 * @param[in] node Node to find.
 * @param[in] node_type Node type of \p node.
 * @param[in] root Root node.
 * @param[in] root_type Type of the XPath \p root node.
 * @param[in] prev Node that we think is before \p node in DFS from \p root. Can optionally
 * be used to increase efficiency and start the DFS from this node.
 * @param[in] prev_pos Node \p prev position. Optional, but must be set if \p prev is set.
 *
 * @return Node position.
 */
static uint32_t
get_node_pos(const struct lyd_node *node, enum lyxp_node_type node_type, const struct lyd_node *root,
             enum lyxp_node_type root_type, const struct lyd_node **prev, uint32_t *prev_pos)
{
    const struct lyd_node *next, *elem, *top_sibling;
    uint32_t pos = 1;

    assert(prev && prev_pos && !root->prev->next);

    if ((node_type == LYXP_NODE_ROOT) || (node_type == LYXP_NODE_ROOT_CONFIG)) {
        return 0;
    }

    if (*prev) {
        /* start from the previous element instead from the root */
        elem = next = *prev;
        pos = *prev_pos;
        for (top_sibling = elem; top_sibling->parent; top_sibling = top_sibling->parent);
        goto dfs_search;
    }

    LY_TREE_FOR(root, top_sibling) {
        /* TREE DFS */
        LY_TREE_DFS_BEGIN(top_sibling, next, elem) {
dfs_search:
            if ((root_type == LYXP_NODE_ROOT_CONFIG) && (elem->schema->flags & LYS_CONFIG_R)) {
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
            if (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                next = NULL;
            }
            if (!next) {
skip_children:
                /* no children */
                if (elem == top_sibling) {
                    /* we are done, root has no children */
                    elem = NULL;
                    break;
                }
                /* try siblings */
                next = elem->next;
            }
            while (!next) {
                /* no siblings, go back through parents */
                if (elem->parent == top_sibling->parent) {
                    /* we are done, no next element to process */
                    elem = NULL;
                    break;
                }
                /* parent is already processed, go to its sibling */
                elem = elem->parent;
                next = elem->next;
            }
        }

        /* node found */
        if (elem) {
            break;
        }
    }

    if (!elem) {
        if (!(*prev)) {
            /* we went from root and failed to find it, cannot be */
            LOGINT;
            return 0;
        } else {
            /* node is before prev, we assumed otherwise :( */
            //LOGDBG("XPATH: get_node_pos optimalization fail.");

            *prev = NULL;
            *prev_pos = 0;

            elem = next = top_sibling = root;
            pos = 1;
            goto dfs_search;
        }
    }

    /*if (*prev) {
        LOGDBG("XPATH: get_node_pos optimalization success.");
    }*/

    /* remember the last found node for next time */
    *prev = node;
    *prev_pos = pos;

    return pos;
}

/**
 * @brief Assign (fill) missing node positions.
 *
 * @param[in] set Set to fill positions in.
 * @param[in] root Context root node.
 * @param[in] root_type Context root type.
 *
 * @return 0 on success, -1 on error.
 */
static int
set_assign_pos(struct lyxp_set *set, const struct lyd_node *root, enum lyxp_node_type root_type)
{
    const struct lyd_node *prev = NULL, *tmp_node;
    uint32_t i, tmp_pos = 0;

    for (i = 0; i < set->used; ++i) {
        if (!set->val.nodes[i].pos) {
            tmp_node = NULL;
            switch (set->val.nodes[i].type) {
            case LYXP_NODE_ATTR:
                tmp_node = lyd_attr_parent(root, set->val.attrs[i].attr);
                if (!tmp_node) {
                    LOGINT;
                    return -1;
                }
                /* fallthrough */
            case LYXP_NODE_ELEM:
            case LYXP_NODE_TEXT:
                if (!tmp_node) {
                    tmp_node = set->val.nodes[i].node;
                }
                set->val.nodes[i].pos = get_node_pos(tmp_node, set->val.nodes[i].type, root, root_type, &prev, &tmp_pos);
                break;
            default:
                /* all roots have position 0 */
                break;
            }
        }
    }

    return 0;
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
get_attr_pos(struct lyd_attr *attr, const struct lyd_node *parent)
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
 * @param[in] idx1 Index of the 1st node in \p set1.
 * @param[in] set1 Set with the 1st node on index \p idx1.
 * @param[in] idx2 Index of the 2nd node in \p set2.
 * @param[in] set2 Set with the 2nd node on index \p idx2.
 * @param[in] root Context root node.
 *
 * @return If 1st > 2nd returns 1, 1st == 2nd returns 0, and 1st < 2nd returns -1.
 */
static int
set_sort_compare(struct lyxp_set_nodes *item1, struct lyxp_set_nodes *item2,
                 const struct lyd_node *root)
{
    const struct lyd_node *tmp_node;
    uint32_t attr_pos1 = 0, attr_pos2 = 0;

    if (item1->pos < item2->pos) {
        return -1;
    }

    if (item1->pos > item2->pos) {
        return 1;
    }

    /* node positions are equal, the fun case */

    /* 1st ELEM - == - 2nd TEXT, 1st TEXT - == - 2nd ELEM */
    /* special case since text nodes are actually saved as their parents */
    if ((item1->node == item2->node) && (item1->type != item2->type)) {
        if (item1->type == LYXP_NODE_ELEM) {
            assert(item2->type == LYXP_NODE_TEXT);
            return -1;
        } else {
            assert((item1->type == LYXP_NODE_TEXT) && (item2->type == LYXP_NODE_ELEM));
            return 1;
        }
    }

    /* we need attr positions now */
    if (item1->type == LYXP_NODE_ATTR) {
        tmp_node = lyd_attr_parent(root, (struct lyd_attr *)item1->node);
        if (!tmp_node) {
            LOGINT;
            return -1;
        }
        attr_pos1 = get_attr_pos((struct lyd_attr *)item1->node, tmp_node);
    }
    if (item2->type == LYXP_NODE_ATTR) {
        tmp_node = lyd_attr_parent(root, (struct lyd_attr *)item2->node);
        if (!tmp_node) {
            LOGINT;
            return -1;
        }
        attr_pos2 = get_attr_pos((struct lyd_attr *)item2->node, tmp_node);
    }

    /* 1st ROOT - 2nd ROOT, 1st ELEM - 2nd ELEM, 1st TEXT - 2nd TEXT, 1st ATTR - =pos= - 2nd ATTR */
    /* check for duplicates */
    if (item1->node == item2->node) {
        assert((item1->type == item2->type) && ((item1->type != LYXP_NODE_ATTR) || (attr_pos1 == attr_pos2)));
        return 0;
    }

    /* 1st ELEM - 2nd TEXT, 1st ELEM - any pos - 2nd ATTR */
    /* elem is always first, 2nd node is after it */
    if (item1->type == LYXP_NODE_ELEM) {
        assert(item2->type != LYXP_NODE_ELEM);
        return -1;
    }

    /* 1st TEXT - 2nd ELEM, 1st TEXT - any pos - 2nd ATTR, 1st ATTR - any pos - 2nd ELEM, 1st ATTR - >pos> - 2nd ATTR */
    /* 2nd is before 1st */
    if (((item1->type == LYXP_NODE_TEXT)
            && ((item2->type == LYXP_NODE_ELEM) || (item2->type == LYXP_NODE_ATTR)))
            || ((item1->type == LYXP_NODE_ATTR) && (item2->type == LYXP_NODE_ELEM))
            || (((item1->type == LYXP_NODE_ATTR) && (item2->type == LYXP_NODE_ATTR))
            && (attr_pos1 > attr_pos2))) {
        return 1;
    }

    /* 1st ATTR - any pos - 2nd TEXT, 1st ATTR <pos< - 2nd ATTR */
    /* 2nd is after 1st */
    return -1;
}

#ifndef NDEBUG

/**
 * @brief Bubble sort \p set into XPath document order.
 *        Context position aware. Unused in the 'Release' build target.
 *
 * @param[in] set Set to sort.
 * @param[in] cur_node Original context node.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return How many times the whole set was traversed.
 */
static int
set_sort(struct lyxp_set *set, const struct lyd_node *cur_node, int options)
{
    uint32_t i, j;
    int ret = 0, cmp, inverted, change;
    const struct lyd_node *root;
    enum lyxp_node_type root_type;
    struct lyxp_set_nodes item;

    if ((set->type != LYXP_SET_NODE_SET) || (set->used == 1)) {
        return 0;
    }

    /* get root */
    root = moveto_get_root(cur_node, options, &root_type);

    /* fill positions */
    if (set_assign_pos(set, root, root_type)) {
        return -1;
    }

    LOGDBG("XPATH: SORT BEGIN");
    print_set_debug(set);

    for (i = 0; i < set->used; ++i) {
        inverted = 0;
        change = 0;

        for (j = 1; j < set->used - i; ++j) {
            /* compare node positions */
            if (inverted) {
                cmp = set_sort_compare(&set->val.nodes[j], &set->val.nodes[j - 1], root);
            } else {
                cmp = set_sort_compare(&set->val.nodes[j - 1], &set->val.nodes[j], root);
            }

            /* swap if needed */
            if ((inverted && (cmp < 0)) || (!inverted && (cmp > 0))) {
                change = 1;

                item = set->val.nodes[j - 1];
                set->val.nodes[j - 1] = set->val.nodes[j];
                set->val.nodes[j] = item;
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
    uint32_t i = 0;
    int ret = EXIT_SUCCESS;

    if (set->used > 1) {
        while (i < set->used - 1) {
            if ((set->val.nodes[i].node == set->val.nodes[i + 1].node)
                    && (set->val.nodes[i].type == set->val.nodes[i + 1].type)) {
                set_remove_node(set, i + 1);
            ret = EXIT_FAILURE;
            } else {
                ++i;
            }
        }
    }

    return ret;
}

#endif

/**
 * @brief Merge 2 sorted sets into one.
 *
 * @param[in,out] trg Set to merge into. Duplicates are removed.
 * @param[in] src Set to be merged into \p trg. It is cast to #LYXP_SET_EMPTY on success.
 * @param[in] cur_node Original context node.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return 0 on success, -1 on error.
 */
static int
set_sorted_merge(struct lyxp_set *trg, struct lyxp_set *src, struct lyd_node *cur_node, int options)
{
    uint32_t i, j, count, dup_count;
    int cmp;
    const struct lyd_node *root;
    enum lyxp_node_type root_type;

    if (((trg->type != LYXP_SET_NODE_SET) && (trg->type != LYXP_SET_EMPTY))
            || ((src->type != LYXP_SET_NODE_SET) && (src->type != LYXP_SET_EMPTY))) {
        return -1;
    }

    if (src->type == LYXP_SET_EMPTY) {
        return 0;
    } else if (trg->type == LYXP_SET_EMPTY) {
        set_fill_set(trg, src);
        lyxp_set_cast(src, LYXP_SET_EMPTY, cur_node, options);
        return 0;
    }

    /* get root */
    root = moveto_get_root(cur_node, options, &root_type);

    /* fill positions */
    if (set_assign_pos(trg, root, root_type) || set_assign_pos(src, root, root_type)) {
        return -1;
    }

#ifndef NDEBUG
    LOGDBG("XPATH: MERGE target");
    print_set_debug(trg);
    LOGDBG("XPATH: MERGE source");
    print_set_debug(src);
#endif

    /* make memory for the merge (duplicates are not detected yet, so space
     * will likely be wasted on them, too bad) */
    if (trg->size - trg->used < src->used) {
        trg->size = trg->used + src->used;

        trg->val.nodes = ly_realloc(trg->val.nodes, trg->size * sizeof *trg->val.nodes);
        if (!trg->val.nodes) {
            LOGMEM;
            return -1;
        }
    }

    i = 0;
    j = 0;
    count = 0;
    dup_count = 0;
    do {
        cmp = set_sort_compare(&src->val.nodes[i], &trg->val.nodes[j], root);
        if (!cmp) {
            if (!count) {
                /* duplicate, just skip it */
                ++i;
                ++j;
            } else {
                /* we are copying something already, so let's copy the duplicate too,
                 * we are hoping that afterwards there are some more nodes to
                 * copy and this way we can copy them all together */
                ++count;
                ++dup_count;
                ++i;
                ++j;
            }
        } else if (cmp < 0) {
            /* inserting src node into trg, just remember it for now */
            ++count;
            ++i;
        } else if (count) {
copy_nodes:
            /* time to actually copy the nodes, we have found the largest block of nodes */
            memmove(&trg->val.nodes[j + (count - dup_count)],
                    &trg->val.nodes[j],
                    (trg->used - j) * sizeof *trg->val.nodes);
            memcpy(&trg->val.nodes[j - dup_count], &src->val.nodes[i - count], count * sizeof *src->val.nodes);

            trg->used += count - dup_count;
            /* do not change i, except the copying above, we are basically doing exactly what is in the else branch below */
            j += count - dup_count;

            count = 0;
            dup_count = 0;
        } else {
            ++j;
        }
    } while ((i < src->used) && (j < trg->used));

    if ((i < src->used) || count) {
        /* loop ended, but we need to copy something at trg end */
        count += src->used - i;
        i = src->used;
        goto copy_nodes;
    }

#ifndef NDEBUG
    LOGDBG("XPATH: MERGE result");
    print_set_debug(trg);
#endif

    lyxp_set_cast(src, LYXP_SET_EMPTY, cur_node, options);
    return 0;
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
 * @param[in] strict Whether the token is strictly required (print error if
 * not the next one) or we simply want to check whether it is the next or not.
 *
 * @return EXIT_SUCCESS if the current token matches the expected one,
 *         -1 otherwise.
 */
static int
exp_check_token(struct lyxp_expr *exp, uint16_t exp_idx, enum lyxp_token want_tok, int strict)
{
    if (exp->used == exp_idx) {
        if (strict) {
            LOGVAL(LYE_XPATH_EOF, LY_VLOG_NONE, NULL);
        }
        return -1;
    }

    if (want_tok && (exp->tokens[exp_idx] != want_tok)) {
        if (strict) {
            LOGVAL(LYE_XPATH_INTOK, LY_VLOG_NONE, NULL,
                   print_token(exp->tokens[exp_idx]), &exp->expr[exp->expr_pos[exp_idx]]);
        }
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
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_predicate(struct lyxp_expr *exp, uint16_t *exp_idx)
{
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_BRACK1, 1)) {
        return -1;
    }
    ++(*exp_idx);

    if (reparse_expr(exp, exp_idx)) {
        return -1;
    }

    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_BRACK2, 1)) {
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
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on forward reference, -1 on error.
 */
static int
reparse_relative_location_path(struct lyxp_expr *exp, uint16_t *exp_idx)
{
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_NONE, 1)) {
        return -1;
    }

    goto step;
    do {
        /* '/' or '//' */
        ++(*exp_idx);

        if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_NONE, 1)) {
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

            if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_NONE, 1)) {
                return -1;
            }
            if ((exp->tokens[*exp_idx] != LYXP_TOKEN_NAMETEST) && (exp->tokens[*exp_idx] != LYXP_TOKEN_NODETYPE)) {
                LOGVAL(LYE_XPATH_INTOK, LY_VLOG_NONE, NULL,
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
            if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_PAR1, 1)) {
                return -1;
            }
            ++(*exp_idx);

            /* ')' */
            if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_PAR2, 1)) {
                return -1;
            }
            ++(*exp_idx);

reparse_predicate:
            /* Predicate* */
            while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_BRACK1)) {
                if (reparse_predicate(exp, exp_idx)) {
                    return -1;
                }
            }
            break;
        default:
            LOGVAL(LYE_XPATH_INTOK, LY_VLOG_NONE, NULL,
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
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_absolute_location_path(struct lyxp_expr *exp, uint16_t *exp_idx)
{
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_PATH, 1)) {
        return -1;
    }

    /* '/' RelativeLocationPath? */
    if (exp->tok_len[*exp_idx] == 1) {
        /* '/' */
        ++(*exp_idx);

        if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_NONE, 0)) {
            return EXIT_SUCCESS;
        }
        switch (exp->tokens[*exp_idx]) {
        case LYXP_TOKEN_DOT:
        case LYXP_TOKEN_DDOT:
        case LYXP_TOKEN_AT:
        case LYXP_TOKEN_NAMETEST:
        case LYXP_TOKEN_NODETYPE:
            if (reparse_relative_location_path(exp, exp_idx)) {
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

        if (reparse_relative_location_path(exp, exp_idx)) {
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
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_function_call(struct lyxp_expr *exp, uint16_t *exp_idx)
{
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_FUNCNAME, 1)) {
        return -1;
    }
    ++(*exp_idx);

    /* '(' */
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_PAR1, 1)) {
        return -1;
    }
    ++(*exp_idx);

    /* ( Expr ( ',' Expr )* )? */
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_NONE, 1)) {
        return -1;
    }
    if (exp->tokens[*exp_idx] != LYXP_TOKEN_PAR2) {
        if (reparse_expr(exp, exp_idx)) {
            return -1;
        }
    }
    while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_COMMA)) {
        ++(*exp_idx);

        if (reparse_expr(exp, exp_idx)) {
            return -1;
        }
    }

    /* ')' */
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_PAR2, 1)) {
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
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_path_expr(struct lyxp_expr *exp, uint16_t *exp_idx)
{
    if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_NONE, 1)) {
        return -1;
    }

    switch (exp->tokens[*exp_idx]) {
    case LYXP_TOKEN_PAR1:
        /* '(' Expr ')' Predicate* */
        ++(*exp_idx);

        if (reparse_expr(exp, exp_idx)) {
            return -1;
        }

        if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_PAR2, 1)) {
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
        if (reparse_relative_location_path(exp, exp_idx)) {
            return -1;
        }
        break;
    case LYXP_TOKEN_FUNCNAME:
        /* FunctionCall */
        if (reparse_function_call(exp, exp_idx)) {
            return -1;
        }
        goto predicate;
        break;
    case LYXP_TOKEN_OPERATOR_PATH:
        /* AbsoluteLocationPath */
        if (reparse_absolute_location_path(exp, exp_idx)) {
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
        LOGVAL(LYE_XPATH_INTOK, LY_VLOG_NONE, NULL,
               print_token(exp->tokens[*exp_idx]), &exp->expr[exp->expr_pos[*exp_idx]]);
        return -1;
    }

    return EXIT_SUCCESS;

predicate:
    /* Predicate* */
    while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_BRACK1)) {
        if (reparse_predicate(exp, exp_idx)) {
            return -1;
        }
    }

    /* ('/' or '//') RelativeLocationPath */
    if ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_OPERATOR_PATH)) {

        /* '/' or '//' */
        ++(*exp_idx);

        if (reparse_relative_location_path(exp, exp_idx)) {
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
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_unary_expr(struct lyxp_expr *exp, uint16_t *exp_idx)
{
    uint16_t prev_exp;

    /* ('-')* */
    while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_MATH, 0)
            && (exp->expr[exp->expr_pos[*exp_idx]] == '-')) {
        ++(*exp_idx);
    }

    /* PathExpr */
    prev_exp = *exp_idx;
    if (reparse_path_expr(exp, exp_idx)) {
        return -1;
    }

    /* ('|' PathExpr)* */
    while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_UNI, 0)) {
        exp_repeat_push(exp, prev_exp, *exp_idx);
        ++(*exp_idx);

        prev_exp = *exp_idx;
        if (reparse_path_expr(exp, exp_idx)) {
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
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_additive_expr(struct lyxp_expr *exp, uint16_t *exp_idx)
{
    uint16_t prev_add_exp, prev_mul_exp;

    goto reparse_multiplicative_expr;

    /* ('+' / '-' MultiplicativeExpr)* */
    while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_MATH, 0)
            && ((exp->expr[exp->expr_pos[*exp_idx]] == '+') || (exp->expr[exp->expr_pos[*exp_idx]] == '-'))) {
        exp_repeat_push(exp, prev_add_exp, *exp_idx);
        ++(*exp_idx);

reparse_multiplicative_expr:
        prev_add_exp = *exp_idx;
        prev_mul_exp = *exp_idx;

        /* UnaryExpr */
        if (reparse_unary_expr(exp, exp_idx)) {
            return -1;
        }

        /* ('*' / 'div' / 'mod' UnaryExpr)* */
        while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_MATH, 0)
                && ((exp->expr[exp->expr_pos[*exp_idx]] == '*') || (exp->tok_len[*exp_idx] == 3))) {
            exp_repeat_push(exp, prev_mul_exp, *exp_idx);
            ++(*exp_idx);

            prev_mul_exp = *exp_idx;
            if (reparse_unary_expr(exp, exp_idx)) {
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
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_equality_expr(struct lyxp_expr *exp, uint16_t *exp_idx)
{
    uint16_t prev_eq_exp, prev_rel_exp;

    goto reparse_additive_expr;

    /* ('=' / '!=' RelationalExpr)* */
    while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_COMP, 0)
            && ((exp->expr[exp->expr_pos[*exp_idx]] == '=') || (exp->expr[exp->expr_pos[*exp_idx]] == '!'))) {
        exp_repeat_push(exp, prev_eq_exp, *exp_idx);
        ++(*exp_idx);

reparse_additive_expr:
        prev_eq_exp = *exp_idx;
        prev_rel_exp = *exp_idx;

        /* AdditiveExpr */
        if (reparse_additive_expr(exp, exp_idx)) {
            return -1;
        }

        /* ('<' / '>' / '<=' / '>=' AdditiveExpr)* */
        while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_COMP, 0)
                && ((exp->expr[exp->expr_pos[*exp_idx]] == '<') || (exp->expr[exp->expr_pos[*exp_idx]] == '>'))) {
            exp_repeat_push(exp, prev_rel_exp, *exp_idx);
            ++(*exp_idx);

            prev_rel_exp = *exp_idx;
            if (reparse_additive_expr(exp, exp_idx)) {
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
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
reparse_expr(struct lyxp_expr *exp, uint16_t *exp_idx)
{
    uint16_t prev_or_exp, prev_and_exp;

    goto reparse_equality_expr;

    /* ('or' AndExpr)* */
    while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_LOG, 0) && (exp->tok_len[*exp_idx] == 2)) {
        exp_repeat_push(exp, prev_or_exp, *exp_idx);
        ++(*exp_idx);

reparse_equality_expr:
        prev_or_exp = *exp_idx;
        prev_and_exp = *exp_idx;

        /* EqualityExpr */
        if (reparse_equality_expr(exp, exp_idx)) {
            return -1;
        }

        /* ('and' EqualityExpr)* */
        while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_LOG, 0) && (exp->tok_len[*exp_idx] == 3)) {
            exp_repeat_push(exp, prev_and_exp, *exp_idx);
            ++(*exp_idx);

            prev_and_exp = *exp_idx;
            if (reparse_equality_expr(exp, exp_idx)) {
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

    uc = lyxml_getutf8(&ncname[parsed], &size);
    if (!is_xmlnamestartchar(uc) || (uc == ':')) {
       return parsed;
    }

    do {
        parsed += size;
        if (!ncname[parsed]) {
            break;
        }
        uc = lyxml_getutf8(&ncname[parsed], &size);
    } while (is_xmlnamechar(uc) && (uc != ':'));

    return parsed;
}

/**
 * @brief Parse an XPath expression into a structure of tokens.
 *        Logs directly.
 *
 * http://www.w3.org/TR/1999/REC-xpath-19991116/ section 3.7
 *
 * @param[in] expr XPath expression to parse. It is duplicated.
 *
 * @return Filled expression structure or NULL on error.
 */
static struct lyxp_expr *
parse_expr(const char *expr)
{
    struct lyxp_expr *ret;
    uint16_t parsed = 0, tok_len, ncname_len;
    enum lyxp_token tok_type;
    int prev_function_check = 0;

    /* init lyxp_expr structure */
    ret = calloc(1, sizeof *ret);
    if (!ret) {
        LOGMEM;
        goto error;
    }
    ret->expr = strdup(expr);
    if (!ret->expr) {
        LOGMEM;
        goto error;
    }
    ret->used = 0;
    ret->size = LYXP_EXPR_SIZE_START;
    ret->tokens = malloc(ret->size * sizeof *ret->tokens);
    if (!ret->tokens) {
        LOGMEM;
        goto error;
    }
    ret->expr_pos = malloc(ret->size * sizeof *ret->expr_pos);
    if (!ret->expr_pos) {
        LOGMEM;
        goto error;
    }
    ret->tok_len = malloc(ret->size * sizeof *ret->tok_len);
    if (!ret->tok_len) {
        LOGMEM;
        goto error;
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
                LOGVAL(LYE_INCHAR, LY_VLOG_NONE, NULL, expr[parsed], &expr[parsed]);
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
                LOGVAL(LYE_INCHAR, LY_VLOG_NONE, NULL, expr[parsed], &expr[parsed]);
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
                        LOGVAL(LYE_INCHAR, LY_VLOG_NONE, NULL, expr[parsed], &expr[parsed]);
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
    exp_free(ret);
    return NULL;
}

/*
 * XPath functions
 */

/**
 * @brief Execute the YANG 1.1 bit-is-set(node-set, string) function. Returns LYXP_SET_BOOLEAN
 *        depending on whether the first node bit value from the second argument is set.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_bit_is_set(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                 int options)
{
    struct lyd_node_leaf_list *leaf;
    int i, bits_count;

    if (arg_count != 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "bit-is-set(node-set, string)");
        return -1;
    }

    if ((args[0]->type != LYXP_SET_NODE_SET) && (args[0]->type != LYXP_SET_EMPTY)) {
        LOGVAL(LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "bit-is-set(node-set, string)");
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }

    set_fill_boolean(set, 0);
    if (args[0]->type == LYXP_SET_NODE_SET) {
        leaf = (struct lyd_node_leaf_list *)args[0]->val.nodes[0].node;
        if ((leaf->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))
                && (((struct lys_node_leaf *)leaf->schema)->type.base == LY_TYPE_BITS)) {
            bits_count = ((struct lys_node_leaf *)leaf->schema)->type.info.bits.count;
            for (i = 0; i < bits_count; ++i) {
                if (leaf->value.bit[i] && ly_strequal(leaf->value.bit[i]->name, args[1]->val.str, 0)) {
                    set_fill_boolean(set, 1);
                    break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the XPath boolean(object) function. Returns LYXP_SET_BOOLEAN
 *        with the argument converted to boolean.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_boolean(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
              int options)
{
    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "boolean(object)");
        return -1;
    }

    lyxp_set_cast(args[0], LYXP_SET_BOOLEAN, cur_node, options);
    set_fill_set(set, args[0]);

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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_ceiling(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
              int options)
{
    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "ceiling(number)");
        return -1;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_NUMBER, cur_node, options)) {
        return -1;
    }
    if ((long long)args[0]->val.num != args[0]->val.num) {
        set_fill_number(set, ((long long)args[0]->val.num) + 1);
    } else {
        set_fill_number(set, args[0]->val.num);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_concat(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
             int options)
{
    uint16_t i;
    char *str = NULL;
    size_t used = 1;

    if (arg_count < 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "concat(string, string, string*)");
        return -1;
    }

    for (i = 0; i < arg_count; ++i) {
        if (lyxp_set_cast(args[i], LYXP_SET_STRING, cur_node, options)) {
            free(str);
            return -1;
        }

        str = ly_realloc(str, (used + strlen(args[i]->val.str)) * sizeof(char));
        if (!str) {
            LOGMEM;
            return -1;
        }
        strcpy(str + used - 1, args[i]->val.str);
        used += strlen(args[i]->val.str);
    }

    /* free, kind of */
    lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, options);
    set->type = LYXP_SET_STRING;
    set->val.str = str;

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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_contains(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
               int options)
{
    if (arg_count != 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "contains(string, string)");
        return -1;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }

    if (strstr(args[0]->val.str, args[1]->val.str)) {
        set_fill_boolean(set, 1);
    } else {
        set_fill_boolean(set, 0);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_count(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *UNUSED(cur_node), struct lyxp_set *set,
            int UNUSED(options))
{
    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "count(node-set)");
        return -1;
    }

    if (args[0]->type == LYXP_SET_EMPTY) {
        set_fill_number(set, 0);
        return EXIT_SUCCESS;
    }

    if (args[0]->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "count(node-set)");
        return -1;
    }

    set_fill_number(set, args[0]->used);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_current(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
              int options)
{
    if (arg_count || args) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "current()");
        return -1;
    }

    lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, options);

    if (options & LYXP_SNODE_ALL) {
        set_snode_clear_ctx(set);

        set_snode_insert_node(set, (struct lys_node *)cur_node, LYXP_NODE_ELEM);
    } else {
        /* position is filled later */
        set_insert_node(set, cur_node, 0, LYXP_NODE_ELEM, 0);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the YANG 1.1 deref(node-set) function. Returns LYXP_SET_NODE_SET with either
 *        leafref or instance-identifier target node(s).
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_deref(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
            int options)
{
    struct lyd_node_leaf_list *leaf;
    struct lys_node_leaf *sleaf;

    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "deref(node-set)");
        return -1;
    }

    if ((args[0]->type != LYXP_SET_NODE_SET) && (args[0]->type != LYXP_SET_SNODE_SET)
            && (args[0]->type != LYXP_SET_EMPTY)) {
        LOGVAL(LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "deref(node-set)");
        return -1;
    }

    if (options & LYXP_SNODE_ALL) {
        assert(args[0]->type == LYXP_SET_SNODE_SET);
        set_snode_clear_ctx(set);

        sleaf = (struct lys_node_leaf *)args[0]->val.snodes[0].snode;
        if ((sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST)) && (sleaf->type.base == LY_TYPE_LEAFREF)) {
            assert(sleaf->type.info.lref.path && sleaf->type.info.lref.target);
            set_insert_node(set, sleaf->type.info.lref.target, 0, LYXP_NODE_ELEM, 0);
        }
        set_snode_insert_node(set, (struct lys_node *)cur_node, LYXP_NODE_ELEM);
    } else {
        lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, options);
        if (args[0]->type != LYXP_SET_EMPTY) {
            leaf = (struct lyd_node_leaf_list *)args[0]->val.nodes[0].node;
            sleaf = (struct lys_node_leaf *)leaf->schema;
            if ((sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST))
                    && ((sleaf->type.base == LY_TYPE_LEAFREF) || (sleaf->type.base == LY_TYPE_INST))) {
                if (leaf->value_type & LY_TYPE_LEAFREF_UNRES) {
                    /* this is bad */
                    LOGINT;
                    return -1;
                }
                /* works for both leafref and instid */
                set_insert_node(set, leaf->value.leafref, 0, LYXP_NODE_ELEM, 0);
            }
        }
    }

    return EXIT_SUCCESS;
}

/* return 0 - match, 1 - mismatch */
static int
xpath_derived_from_ident_cmp(struct lys_ident *ident, const char *ident_str)
{
    const char *ptr;
    int len;

    ptr = strchr(ident_str, ':');
    if (ptr) {
        len = ptr - ident_str;
        if (strncmp(ident->module->name, ident_str, len)
                || ident->module->name[len]) {
            /* module name mismatch BUG we expect JSON format prefix, but if the 2nd argument was
             * not a literal, we may easily be mistaken */
            return 1;
        }
        ++ptr;
    } else {
        ptr = ident_str;
    }

    len = strlen(ptr);
    if (strncmp(ident->name, ptr, len) || ident->name[len]) {
        /* name mismatch */
        return 1;
    }

    return 0;
}

/**
 * @brief Execute the YANG 1.1 derived-from(node-set, string) function. Returns LYXP_SET_BOOLEAN depending
 *        on whether the first argument nodes contain a node of an identity derived from the second
 *        argument identity.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_derived_from(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                   int options)
{
    uint16_t i, j;
    struct lyd_node_leaf_list *leaf;
    struct lys_node_leaf *sleaf;

    if (arg_count != 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "derived-from(node-set, string)");
        return -1;
    }

    if ((args[0]->type != LYXP_SET_NODE_SET) && (args[0]->type != LYXP_SET_EMPTY)) {
        LOGVAL(LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "derived-from(node-set, string)");
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }

    set_fill_boolean(set, 0);
    if (args[0]->type != LYXP_SET_EMPTY) {
        for (i = 0; i < args[0]->used; ++i) {
            leaf = (struct lyd_node_leaf_list *)args[0]->val.nodes[i].node;
            sleaf = (struct lys_node_leaf *)leaf->schema;
            if ((sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST)) && (sleaf->type.base == LY_TYPE_IDENT)) {
                for (j = 0; j < leaf->value.ident->base_size; ++j) {
                    if (!xpath_derived_from_ident_cmp(leaf->value.ident->base[j], args[1]->val.str)) {
                        set_fill_boolean(set, 1);
                        break;
                    }
                }

                if (j < leaf->value.ident->base_size) {
                    break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the YANG 1.1 derived-from-or-self(node-set, string) function. Returns LYXP_SET_BOOLEAN depending
 *        on whether the first argument nodes contain a node of an identity that either is or is derived from
 *        the second argument identity.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_derived_from_or_self(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                           int options)
{
    uint16_t i, j;
    struct lyd_node_leaf_list *leaf;
    struct lys_node_leaf *sleaf;

    if (arg_count != 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "derived-from-or-self(node-set, string)");
        return -1;
    }

    if ((args[0]->type != LYXP_SET_NODE_SET) && (args[0]->type != LYXP_SET_EMPTY)) {
        LOGVAL(LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "derived-from-or-self(node-set, string)");
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }

    set_fill_boolean(set, 0);
    if (args[0]->type != LYXP_SET_EMPTY) {
        for (i = 0; i < args[0]->used; ++i) {
            leaf = (struct lyd_node_leaf_list *)args[0]->val.nodes[i].node;
            sleaf = (struct lys_node_leaf *)leaf->schema;
            if ((sleaf->nodetype & (LYS_LEAF | LYS_LEAFLIST)) && (sleaf->type.base == LY_TYPE_IDENT)) {
                if (!xpath_derived_from_ident_cmp(leaf->value.ident, args[1]->val.str)) {
                    set_fill_boolean(set, 1);
                    break;
                }

                for (j = 0; j < leaf->value.ident->base_size; ++j) {
                    if (!xpath_derived_from_ident_cmp(leaf->value.ident->base[j], args[1]->val.str)) {
                        set_fill_boolean(set, 1);
                        break;
                    }
                }

                if (j < leaf->value.ident->base_size) {
                    break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Execute the YANG 1.1 enum-value(node-set) function. Returns LYXP_SET_NUMBER
 *        with the integer value of the first node's enum value, otherwise NaN.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_enum_value(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *UNUSED(cur_node), struct lyxp_set *set,
                 int UNUSED(options))
{
    struct lyd_node_leaf_list *leaf;

    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "enum-value(node-set)");
        return -1;
    }

    if ((args[0]->type != LYXP_SET_NODE_SET) && (args[0]->type != LYXP_SET_EMPTY)) {
        LOGVAL(LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "enum-value(node-set)");
        return -1;
    }

    set_fill_number(set, NAN);
    if (args[0]->type == LYXP_SET_NODE_SET) {
        leaf = (struct lyd_node_leaf_list *)args[0]->val.nodes[0].node;
        if ((leaf->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))
                && (((struct lys_node_leaf *)leaf->schema)->type.base == LY_TYPE_ENUM)) {
            set_fill_number(set, leaf->value.enm->value);
        }
    }

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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_false(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *UNUSED(cur_node), struct lyxp_set *set,
            int UNUSED(options))
{
    if (arg_count || args) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "false()");
        return -1;
    }

    set_fill_boolean(set, 0);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_floor(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
            int options)
{
    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "floor(number)");
        return -1;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_NUMBER, cur_node, options)) {
        return -1;
    }
    if (isfinite(args[0]->val.num)) {
        set_fill_number(set, (long long)args[0]->val.num);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_lang(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
           int options)
{
    const struct lyd_node *node, *root;
    struct lyd_attr *attr = NULL;
    int i;

    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "lang(string)");
        return -1;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }

    if (set->type == LYXP_SET_EMPTY) {
        set_fill_boolean(set, 0);
        return EXIT_SUCCESS;
    }
    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INCTX, LY_VLOG_NONE, NULL, print_set_type(set), "lang(string)");
        return -1;
    }

    switch (set->val.nodes[0].type) {
    case LYXP_NODE_ELEM:
    case LYXP_NODE_TEXT:
        node = set->val.nodes[0].node;
        break;
    case LYXP_NODE_ATTR:
        root = moveto_get_root(cur_node, options, NULL);
        node = lyd_attr_parent(root, set->val.attrs[0].attr);
        break;
    default:
        /* nothing to do with roots */
        set_fill_boolean(set, 0);
        return EXIT_SUCCESS;
    }

    /* find lang attribute */
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
        set_fill_boolean(set, 0);
    } else {
        for (i = 0; args[0]->val.str[i]; ++i) {
            if (tolower(args[0]->val.str[i]) != tolower(attr->value[i])) {
                set_fill_boolean(set, 0);
                break;
            }
        }
        if (!args[0]->val.str[i]) {
            if (!attr->value[i] || (attr->value[i] == '-')) {
                set_fill_boolean(set, 1);
            } else {
                set_fill_boolean(set, 0);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_last(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *UNUSED(cur_node), struct lyxp_set *set,
           int UNUSED(options))
{
    if (arg_count || args) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "last()");
        return -1;
    }

    if (set->type == LYXP_SET_EMPTY) {
        set_fill_number(set, 0);
        return EXIT_SUCCESS;
    }
    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INCTX, LY_VLOG_NONE, NULL, print_set_type(set), "last()");
        return -1;
    }

    set_fill_number(set, set->ctx_size);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_local_name(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                 int options)
{
    struct lyxp_set_nodes *item;

    if (arg_count > 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "local-name(node-set?)");
        return -1;
    }

    if (arg_count) {
        if (args[0]->type == LYXP_SET_EMPTY) {
            set_fill_string(set, "", 0);
            return EXIT_SUCCESS;
        }
        if (args[0]->type != LYXP_SET_NODE_SET) {
            LOGVAL(LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "local-name(node-set?)");
            return -1;
        }

#ifndef NDEBUG
        /* we need the set sorted, it affects the result */
        if (set_sort(args[0], cur_node, options) > 1) {
            LOGERR(LY_EINT, "XPath set was expected to be sorted, but is not (%s).", __func__);
        }
#else
    /* suppress unused variable warning */
    (void)cur_node;
#endif

        item = &args[0]->val.nodes[0];
    } else {
        if (set->type == LYXP_SET_EMPTY) {
            set_fill_string(set, "", 0);
            return EXIT_SUCCESS;
        }
        if (set->type != LYXP_SET_NODE_SET) {
            LOGVAL(LYE_XPATH_INCTX, LY_VLOG_NONE, NULL, print_set_type(set), "local-name(node-set?)");
            return -1;
        }

#ifndef NDEBUG
        /* we need the set sorted, it affects the result */
        if (set_sort(set, cur_node, options) > 1) {
            LOGERR(LY_EINT, "XPath set was expected to be sorted, but is not (%s).", __func__);
        }
#endif

        item = &set->val.nodes[0];
    }

    switch (item->type) {
    case LYXP_NODE_ROOT:
    case LYXP_NODE_ROOT_CONFIG:
    case LYXP_NODE_TEXT:
        set_fill_string(set, "", 0);
        break;
    case LYXP_NODE_ELEM:
        set_fill_string(set, item->node->schema->name, strlen(item->node->schema->name));
        break;
    case LYXP_NODE_ATTR:
        set_fill_string(set, ((struct lyd_attr *)item->node)->name, strlen(((struct lyd_attr *)item->node)->name));
        break;
    }

    /* UNUSED in 'Release' build type */
    (void)options;
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_namespace_uri(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                    int options)
{
    struct lyxp_set_nodes *item;
    struct lys_module *module;

    if (arg_count > 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "namespace-uri(node-set?)");
        return -1;
    }

    if (arg_count) {
        if (args[0]->type == LYXP_SET_EMPTY) {
            set_fill_string(set, "", 0);
            return EXIT_SUCCESS;
        }
        if (args[0]->type != LYXP_SET_NODE_SET) {
            LOGVAL(LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "namespace-uri(node-set?)");
            return -1;
        }

#ifndef NDEBUG
        /* we need the set sorted, it affects the result */
        if (set_sort(args[0], cur_node, options) > 1) {
            LOGERR(LY_EINT, "XPath set was expected to be sorted, but is not (%s).", __func__);
        }
#else
    /* suppress unused variable warning */
    (void)cur_node;
#endif

        item = &args[0]->val.nodes[0];
    } else {
        if (set->type == LYXP_SET_EMPTY) {
            set_fill_string(set, "", 0);
            return EXIT_SUCCESS;
        }
        if (set->type != LYXP_SET_NODE_SET) {
            LOGVAL(LYE_XPATH_INCTX, LY_VLOG_NONE, NULL, print_set_type(set), "namespace-uri(node-set?)");
            return -1;
        }

#ifndef NDEBUG
        /* we need the set sorted, it affects the result */
        if (set_sort(set, cur_node, options) > 1) {
            LOGERR(LY_EINT, "XPath set was expected to be sorted, but is not (%s).", __func__);
        }
#endif

        item = &set->val.nodes[0];
    }

    switch (item->type) {
    case LYXP_NODE_ROOT:
    case LYXP_NODE_ROOT_CONFIG:
    case LYXP_NODE_TEXT:
        set_fill_string(set, "", 0);
        break;
    case LYXP_NODE_ELEM:
    case LYXP_NODE_ATTR:
        if (item->type == LYXP_NODE_ELEM) {
            module =  item->node->schema->module;
        } else { /* LYXP_NODE_ATTR */
            module = ((struct lyd_attr *)item->node)->module;
        }

        module = lys_main_module(module);

        set_fill_string(set, module->ns, strlen(module->ns));
        break;
    }

    /* UNUSED in 'Release' build type */
    (void)options;
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_node(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
           int options)
{
    if (arg_count || args) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "node()");
        return -1;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, options);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_normalize_space(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                      int options)
{
    uint16_t i, new_used;
    char *new;
    int have_spaces = 0, space_before = 0;

    if (arg_count > 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "normalize-space(string?)");
        return -1;
    }

    if (arg_count) {
        set_fill_set(set, args[0]);
    }
    if (lyxp_set_cast(set, LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }

    /* is there any normalization necessary? */
    for (i = 0; set->val.str[i]; ++i) {
        if (is_xmlws(set->val.str[i])) {
            if ((i == 0) || space_before || (!set->val.str[i + 1])) {
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
        new = malloc(strlen(set->val.str) * sizeof(char));
        if (!new) {
            LOGMEM;
            return -1;
        }
        new_used = 0;

        space_before = 0;
        for (i = 0; set->val.str[i]; ++i) {
            if (is_xmlws(set->val.str[i])) {
                if ((i == 0) || space_before) {
                    space_before = 1;
                    continue;
                } else {
                    space_before = 1;
                }
            } else {
                space_before = 0;
            }

            new[new_used] = (space_before ? ' ' : set->val.str[i]);
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

        free(set->val.str);
        set->val.str = new;
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_not(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
          int options)
{
    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "not(boolean)");
        return -1;
    }

    lyxp_set_cast(args[0], LYXP_SET_BOOLEAN, cur_node, options);
    if (args[0]->val.bool) {
        set_fill_boolean(set, 0);
    } else {
        set_fill_boolean(set, 1);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_number(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
             int options)
{
    if (arg_count > 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "number(object?)");
        return -1;
    }

    if (arg_count) {
        if (lyxp_set_cast(args[0], LYXP_SET_NUMBER, cur_node, options)) {
            return -1;
        }
        set_fill_set(set, args[0]);
    } else {
        if (lyxp_set_cast(set, LYXP_SET_NUMBER, cur_node, options)) {
            return -1;
        }
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_position(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *UNUSED(cur_node), struct lyxp_set *set,
               int options)
{
    if (arg_count || args) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "position()");
        return -1;
    }

    if (set->type == LYXP_SET_EMPTY) {
        set_fill_number(set, 0);
        return EXIT_SUCCESS;
    }
    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INCTX, LY_VLOG_NONE, NULL, print_set_type(set), "position()");
        return -1;
    }

    set_fill_number(set, set->ctx_pos);

    /* UNUSED in 'Release' build type */
    (void)options;
    return EXIT_SUCCESS;
}

/**
 * @brief Execute the YANG 1.1 re-match(string, string) function. Returns LYXP_SET_BOOLEAN
 *        depending on whether the second argument regex matches the first argument string. For details refer to
 *        YANG 1.1 RFC section 10.2.1.
 *
 * @param[in] args Array of arguments.
 * @param[in] arg_count Count of elements in \p args.
 * @param[in] cur_node Original context node.
 * @param[in,out] set Context and result set at the same time.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_re_match(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
               int options)
{
    pcre *precomp;

    if (arg_count != 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "re-match(string, string)");
        return -1;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }

    if (lyp_check_pattern(args[1]->val.str, &precomp)) {
        return -1;
    }
    if (pcre_exec(precomp, NULL, args[0]->val.str, strlen(args[0]->val.str), 0, 0, NULL, 0)) {
        set_fill_boolean(set, 0);
    } else {
        set_fill_boolean(set, 1);
    }
    free(precomp);

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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_round(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
            int options)
{
    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "round(number)");
        return -1;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_NUMBER, cur_node, options)) {
        return -1;
    }

    /* cover only the cases where floor can't be used */
    if ((args[0]->val.num == -0.0f) || ((args[0]->val.num < 0) && (args[0]->val.num >= -0.5))) {
        set_fill_number(set, -0.0f);
    } else {
        args[0]->val.num += 0.5;
        if (xpath_floor(args, 1, cur_node, args[0], options)) {
            return -1;
        }
        set_fill_number(set, args[0]->val.num);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_starts_with(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                  int options)
{
    if (arg_count != 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "starts-with(string, string)");
        return -1;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }

    if (strncmp(args[0]->val.str, args[1]->val.str, strlen(args[1]->val.str))) {
        set_fill_boolean(set, 0);
    } else {
        set_fill_boolean(set, 1);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_string(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
             int options)
{
    if (arg_count > 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "string(object?)");
        return -1;
    }

    if (arg_count) {
        if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, options)) {
            return -1;
        }
        set_fill_set(set, args[0]);
    } else {
        if (lyxp_set_cast(set, LYXP_SET_STRING, cur_node, options)) {
            return -1;
        }
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_string_length(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                    int options)
{
    if (arg_count > 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "string-length(string?)");
        return -1;
    }

    if (arg_count) {
        if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, options)) {
            return -1;
        }
        set_fill_number(set, strlen(args[0]->val.str));
    } else {
        if (lyxp_set_cast(set, LYXP_SET_STRING, cur_node, options)) {
            return -1;
        }
        set_fill_number(set, strlen(set->val.str));
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_substring(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                int options)
{
    int start, len;
    uint16_t str_start, str_len, pos;

    if ((arg_count < 2) || (arg_count > 3)) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "substring(string, number, number?)");
        return -1;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }

    /* start */
    if (xpath_round(&args[1], 1, cur_node, args[1], options)) {
        return -1;
    }
    if (isfinite(args[1]->val.num)) {
        start = args[1]->val.num - 1;
    } else if (isinf(args[1]->val.num) && signbit(args[1]->val.num)) {
        start = INT_MIN;
    } else {
        start = INT_MAX;
    }

    /* len */
    if (arg_count == 3) {
        if (xpath_round(&args[2], 1, cur_node, args[2], options)) {
            return -1;
        }
        if (isfinite(args[2]->val.num)) {
            len = args[2]->val.num;
        } else if (isnan(args[2]->val.num) || signbit(args[2]->val.num)) {
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
    for (pos = 0; args[0]->val.str[pos]; ++pos) {
        if (pos < start) {
            ++str_start;
        } else if (pos < start + len) {
            ++str_len;
        } else {
            break;
        }
    }

    set_fill_string(set, args[0]->val.str + str_start, str_len);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_substring_after(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                      int options)
{
    char *ptr;

    if (arg_count != 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "substring-after(string, string)");
        return -1;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }

    ptr = strstr(args[0]->val.str, args[1]->val.str);
    if (ptr) {
        set_fill_string(set, ptr + strlen(args[1]->val.str), strlen(ptr + strlen(args[1]->val.str)));
    } else {
        set_fill_string(set, "", 0);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_substring_before(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                       int options)
{
    char *ptr;

    if (arg_count != 2) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "substring-before(string, string)");
        return -1;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }

    ptr = strstr(args[0]->val.str, args[1]->val.str);
    if (ptr) {
        set_fill_string(set, args[0]->val.str, ptr - args[0]->val.str);
    } else {
        set_fill_string(set, "", 0);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_sum(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
          int options)
{
    long double num;
    char *str;
    uint16_t i;
    struct lyxp_set set_item;

    if (arg_count != 1) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "sum(node-set)");
        return -1;
    }

    set_fill_number(set, 0);
    if (args[0]->type == LYXP_SET_EMPTY) {
        return EXIT_SUCCESS;
    }

    if (args[0]->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INARGTYPE, LY_VLOG_NONE, NULL, 1, print_set_type(args[0]), "sum(node-set)");
        return -1;
    }

    set_item.type = LYXP_SET_NODE_SET;
    set_item.val.nodes = malloc(sizeof *set_item.val.nodes);
    if (!set_item.val.nodes) {
        LOGMEM;
        return -1;
    }

    set_item.used = 1;
    set_item.size = 1;

    for (i = 0; i < args[0]->used; ++i) {
        set_item.val.nodes[0] = args[0]->val.nodes[i];

        str = cast_node_set_to_string(&set_item, cur_node, options);
        if (!str) {
            return -1;
        }
        num = cast_string_to_number(str);
        free(str);
        set->val.num += num;
    }

    free(set_item.val.nodes);

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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_text(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *UNUSED(cur_node), struct lyxp_set *set,
           int UNUSED(options))
{
    uint32_t i;

    if (arg_count || args) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "text()");
        return -1;
    }
    if (set->type == LYXP_SET_EMPTY) {
        return EXIT_SUCCESS;
    }
    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INCTX, LY_VLOG_NONE, NULL, print_set_type(set), "text()");
        return -1;
    }

    for (i = 0; i < set->used;) {
        switch (set->val.nodes[i].type) {
        case LYXP_NODE_ELEM:
            if (set->val.nodes[i].node->validity & LYD_VAL_INUSE) {
                LOGVAL(LYE_XPATH_DUMMY, LY_VLOG_LYD, set->val.nodes[i].node, set->val.nodes[i].node->schema->name);
                return -1;
            }
            if ((set->val.nodes[i].node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))
                    && ((struct lyd_node_leaf_list *)set->val.nodes[i].node)->value_str) {
                set->val.nodes[i].type = LYXP_NODE_TEXT;
                ++i;
                break;
            }
            /* fall through */
        case LYXP_NODE_ROOT:
        case LYXP_NODE_ROOT_CONFIG:
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_translate(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *cur_node, struct lyxp_set *set,
                int options)
{
    uint16_t i, j, new_used;
    char *new;
    int found, have_removed;

    if (arg_count != 3) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "translate(string, string, string)");
        return -1;
    }

    if (lyxp_set_cast(args[0], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }
    if (lyxp_set_cast(args[1], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }
    if (lyxp_set_cast(args[2], LYXP_SET_STRING, cur_node, options)) {
        return -1;
    }

    new = malloc((strlen(args[0]->val.str) + 1) * sizeof(char));
    if (!new) {
        LOGMEM;
        return -1;
    }
    new_used = 0;

    have_removed = 0;
    for (i = 0; args[0]->val.str[i]; ++i) {
        found = 0;

        for (j = 0; args[1]->val.str[j]; ++j) {
            if (args[0]->val.str[i] == args[1]->val.str[j]) {
                /* removing this char */
                if (j >= strlen(args[2]->val.str)) {
                    have_removed = 1;
                    found = 1;
                    break;
                }
                /* replacing this char */
                new[new_used] = args[2]->val.str[j];
                ++new_used;
                found = 1;
                break;
            }
        }

        /* copying this char */
        if (!found) {
            new[new_used] = args[0]->val.str[i];
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

    lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, options);
    set->type = LYXP_SET_STRING;
    set->val.str = new;

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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
xpath_true(struct lyxp_set **args, uint16_t arg_count, struct lyd_node *UNUSED(cur_node), struct lyxp_set *set,
           int UNUSED(options))
{
    if (arg_count || args) {
        LOGVAL(LYE_XPATH_INARGCOUNT, LY_VLOG_NONE, NULL, arg_count, "true()");
        return -1;
    }

    set_fill_boolean(set, 1);
    return EXIT_SUCCESS;
}

/*
 * moveto functions
 *
 * They and only they actually change the context (set).
 */

/**
 * @brief Resolve and find a specific model. Does not log.
 *
 * \p cur_snode is required in 2 quite specific cases concerning
 * XPath on schema. Problem is when we are parsing a submodule
 * and referencing something in the main module or parsing
 * a module importing another module that references back
 * the original module. Then the target module is still being
 * parsed and it not yet in the context - it fails to resolve.
 * In these cases we can find the module using \p cur_snode.
 *
 * @param[in] mod_name_ns Either module name or namespace.
 * @param[in] mon_nam_ns_len Length of \p mod_name_ns.
 * @param[in] ctx libyang context.
 * @param[in] cur_snode Current schema node, on data XPath leave NULL.
 * @param[in] is_name Whether \p mod_name_ns is module name (1) or namespace (0).
 *
 * @return Corresponding module or NULL on error.
 */
static struct lys_module *
moveto_resolve_model(const char *mod_name_ns, uint16_t mod_nam_ns_len, struct ly_ctx *ctx, struct lys_node *cur_snode,
                     int is_name)
{
    uint16_t i;
    const char *str;
    struct lys_module *mod;

    if (cur_snode) {
        mod = lys_node_module(cur_snode);
        str = (is_name ? mod->name : mod->ns);
        if (!strncmp(str, mod_name_ns, mod_nam_ns_len) && !str[mod_nam_ns_len]) {
            return lys_node_module(cur_snode);
        }

        for (i = 0; i < mod->imp_size; ++i) {
            str = (is_name ? mod->imp[i].module->name : mod->imp[i].module->ns);
            if (!strncmp(str, mod_name_ns, mod_nam_ns_len) && !str[mod_nam_ns_len]) {
                return mod->imp[i].module;
            }
        }
    }

    for (i = 0; i < ctx->models.used; ++i) {
        str = (is_name ? ctx->models.list[i]->name : ctx->models.list[i]->ns);
        if (!strncmp(str, mod_name_ns, mod_nam_ns_len) && !str[mod_nam_ns_len]) {
            return ctx->models.list[i];
        }
    }

    return NULL;
}

/**
 * @brief Get the context root.
 *
 * @param[in] cur_node Original context node.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 * @param[out] root_type Root type, differs only in when, must evaluation.
 *
 * @return Context root.
 */
static const struct lyd_node *
moveto_get_root(const struct lyd_node *cur_node, int options, enum lyxp_node_type *root_type)
{
    const struct lyd_node *root;

    if (!cur_node) {
        return NULL;
    }

    if (!options) {
        /* special kind of root that can access everything */
        for (root = cur_node; root->parent; root = root->parent);
        for (; root->prev->next; root = root->prev);
        *root_type = LYXP_NODE_ROOT;
        return root;
    }

    if (cur_node->schema->flags & LYS_CONFIG_W) {
        *root_type = LYXP_NODE_ROOT_CONFIG;
    } else {
        *root_type = LYXP_NODE_ROOT;
    }

    for (root = cur_node; root->parent; root = root->parent);
    for (; root->prev->next; root = root->prev);

    return root;
}

static const struct lys_node *
moveto_snode_get_root(const struct lys_node *cur_node, int options, enum lyxp_node_type *root_type)
{
    const struct lys_node *root;

    assert(cur_node && root_type);

    if (options & LYXP_SNODE) {
        /* general root that can access everything */
        for (root = cur_node; lys_parent(root); root = lys_parent(root));
        root = lys_getnext(NULL, NULL, root->module, 0);
        *root_type = LYXP_NODE_ROOT;
        return root;
    }

    if (cur_node->flags & LYS_CONFIG_W) {
        *root_type = LYXP_NODE_ROOT_CONFIG;
    } else {
        *root_type = LYXP_NODE_ROOT;
    }

    for (root = cur_node; lys_parent(root); root = lys_parent(root));
    root = lys_getnext(NULL, NULL, lys_node_module(root), 0);

    return root;
}

/**
 * @brief Move context \p set to the root. Handles absolute path.
 *        Result is LYXP_SET_NODE_SET.
 *
 * @param[in,out] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 */
static void
moveto_root(struct lyxp_set *set, struct lyd_node *cur_node, int options)
{
    const struct lyd_node *root;
    enum lyxp_node_type root_type;

    if (!set) {
        return;
    }

    root = moveto_get_root(cur_node, options, &root_type);

    lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, options);
    if (root) {
        set_insert_node(set, root, 0, root_type, 0);
    }
}

static void
moveto_snode_root(struct lyxp_set *set, struct lys_node *cur_node, int options)
{
    const struct lys_node *root;
    enum lyxp_node_type root_type;

    if (!set) {
        return;
    }

    if (!cur_node) {
        LOGINT;
        return;
    }

    root = moveto_snode_get_root(cur_node, options, &root_type);
    set_snode_clear_ctx(set);
    set_snode_insert_node(set, root, root_type);
}

/**
 * @brief Check \p node as a part of NameTest processing.
 *
 * @param[in] node Node to check.
 * @param[in] node_name Node name to move to. Must be in the dictionary!
 * @param[in] moveto_mod Expected module of the node.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
moveto_node_check(struct lyd_node *node, enum lyxp_node_type root_type, const char *node_name,
                  struct lys_module *moveto_mod, int options)
{
    /* module check */
    if (moveto_mod && (lys_node_module(node->schema) != moveto_mod)) {
        return -1;
    }

    /* context check */
    if ((root_type == LYXP_NODE_ROOT_CONFIG) && (node->schema->flags & LYS_CONFIG_R)) {
        return -1;
    }

    /* name check */
    if (!ly_strequal(node->schema->name, node_name, 1) && strcmp(node_name, "*")) {
        return -1;
    }

    /* when check */
    if ((options & LYXP_WHEN) && !LYD_WHEN_DONE(node->when_status)) {
        return EXIT_FAILURE;
    }

    /* match */
    return EXIT_SUCCESS;
}

static int
moveto_snode_check(const struct lys_node *node, enum lyxp_node_type root_type, const char *node_name,
                   struct lys_module *moveto_mod, int options)
{
    /* RPC input/output check */
    if (options & LYXP_SNODE_OUTPUT) {
        if (lys_parent(node) && (lys_parent(node)->nodetype == LYS_INPUT)) {
            return -1;
        }
    } else {
        if (lys_parent(node) && (lys_parent(node)->nodetype == LYS_OUTPUT)) {
            return -1;
        }
    }

    /* module check */
    if (moveto_mod && (lys_node_module(node) != moveto_mod)) {
        return -1;
    }

    /* context check */
    if ((root_type == LYXP_NODE_ROOT_CONFIG) && (node->flags & LYS_CONFIG_R)) {
        return -1;
    }

    /* name check */
    if (!ly_strequal(node->name, node_name, 1) && strcmp(node_name, "*")) {
        return -1;
    }

    /* match */
    return EXIT_SUCCESS;
}

/**
 * @brief Add \p node into \p set as a part of NameTest processing.
 *
 * @param[in] node Node to add.
 * @param[in] pos Node sort position.
 * @param[in,out] set Set to use.
 * @param[in] i Desired index of \p node in \p set.
 * @param[in,out] replaced Whether the node in \p set has already been replaced.
 */
static void
moveto_node_add(struct lyxp_set *set, struct lyd_node *node, uint32_t pos, uint32_t i, int *replaced)
{
    if (!(*replaced)) {
        set->val.nodes[i].node = node;
        set->val.nodes[i].type = LYXP_NODE_ELEM;
        set->val.nodes[i].pos = pos;
        *replaced = 1;
    } else {
        set_insert_node(set, node, pos, LYXP_NODE_ELEM, i);
    }
}

/**
 * @brief Move context \p set to a node. Handles '/' and '*', 'NAME', 'PREFIX:*', or 'PREFIX:NAME'.
 *        Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY). Context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] qname Qualified node name to move to.
 * @param[in] qname_len Length of \p qname.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
moveto_node(struct lyxp_set *set, struct lyd_node *cur_node, const char *qname, uint16_t qname_len, int options)
{
    uint32_t i;
    int replaced, pref_len, ret;
    const char *ptr, *name_dict = NULL; /* optimalization - so we can do (==) instead (!strncmp(...)) in moveto_node_check() */
    struct lys_module *moveto_mod;
    struct lyd_node *sub;
    struct ly_ctx *ctx;
    enum lyxp_node_type root_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    assert(cur_node);

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    ctx = cur_node->schema->module->ctx;
    moveto_get_root(cur_node, options, &root_type);

    /* prefix */
    if ((ptr = strnchr(qname, ':', qname_len))) {
        pref_len = ptr - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, ctx, NULL, 1);
        if (!moveto_mod) {
            LOGINT;
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        moveto_mod = NULL;
    }

    /* name */
    name_dict = lydict_insert(ctx, qname, qname_len);

    for (i = 0; i < set->used; ) {
        replaced = 0;

        if ((set->val.nodes[i].type == LYXP_NODE_ROOT_CONFIG) || (set->val.nodes[i].type == LYXP_NODE_ROOT)) {
            LY_TREE_FOR(set->val.nodes[i].node, sub) {
                ret = moveto_node_check(sub, root_type, name_dict, moveto_mod, options);
                if (!ret) {
                    /* pos filled later */
                    moveto_node_add(set, sub, 0, i, &replaced);
                    ++i;
                } else if (ret == EXIT_FAILURE) {
                    lydict_remove(ctx, name_dict);
                    return EXIT_FAILURE;
                }
            }

        /* skip nodes without children - leaves, leaflists, anyxmls, and dummy nodes (ouput root will eval to true) */
        } else if (!(set->val.nodes[i].node->validity & LYD_VAL_INUSE)
                && !(set->val.nodes[i].node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA))) {

            LY_TREE_FOR(set->val.nodes[i].node->child, sub) {
                ret = moveto_node_check(sub, root_type, name_dict, moveto_mod, options);
                if (!ret) {
                    moveto_node_add(set, sub, 0, i, &replaced);
                    ++i;
                } else if (ret == EXIT_FAILURE) {
                    lydict_remove(ctx, name_dict);
                    return EXIT_FAILURE;
                }
            }
        }

        if (!replaced) {
            /* no match */
            set_remove_node(set, i);
        }
    }
    lydict_remove(ctx, name_dict);

    return EXIT_SUCCESS;
}

static int
moveto_snode(struct lyxp_set *set, struct lys_node *cur_node, const char *qname, uint16_t qname_len, int options)
{
    int i, orig_used, pref_len, idx, temp_ctx = 0;
    const char *ptr, *name_dict = NULL; /* optimalization - so we can do (==) instead (!strncmp(...)) in moveto_node_check() */
    struct lys_module *moveto_mod;
    const struct lys_node *sub;
    struct ly_ctx *ctx;
    enum lyxp_node_type root_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_SNODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    ctx = cur_node->module->ctx;
    moveto_snode_get_root(cur_node, options, &root_type);

    /* prefix */
    if ((ptr = strnchr(qname, ':', qname_len))) {
        pref_len = ptr - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, ctx, cur_node, 1);
        if (!moveto_mod) {
            LOGINT;
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        moveto_mod = NULL;
    }

    /* name */
    name_dict = lydict_insert(ctx, qname, qname_len);

    orig_used = set->used;
    for (i = 0; i < orig_used; ++i) {
        if (set->val.snodes[i].in_ctx != 1) {
            continue;
        }
        set->val.snodes[i].in_ctx = 0;

        if ((set->val.snodes[i].type == LYXP_NODE_ROOT_CONFIG) || (set->val.snodes[i].type == LYXP_NODE_ROOT)) {
            /* it can actually be in any module, it's all <running>, but we know it's moveto_mod (if set),
             * so use it directly (root node itself is useless in this case) */
            sub = NULL;
            while ((sub = lys_getnext(sub, NULL, (moveto_mod ? moveto_mod : lys_node_module(set->val.snodes[i].snode)), 0))) {
                if (!moveto_snode_check(sub, root_type, name_dict, moveto_mod, options)) {
                    idx = set_snode_insert_node(set, sub, LYXP_NODE_ELEM);
                    /* we need to prevent these nodes to be considered in this moveto */
                    if ((idx < orig_used) && (idx > i)) {
                        set->val.snodes[idx].in_ctx = 2;
                        temp_ctx = 1;
                    }
                }
            }

        /* skip nodes without children - leaves, leaflists, and anyxmls (ouput root will eval to true) */
        } else if (!(set->val.snodes[i].snode->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA))) {
            sub = NULL;
            while ((sub = lys_getnext(sub, set->val.snodes[i].snode, NULL, 0))) {
                if (!moveto_snode_check(sub, root_type, name_dict, moveto_mod, options)) {
                    idx = set_snode_insert_node(set, sub, LYXP_NODE_ELEM);
                    if ((idx < orig_used) && (idx > i)) {
                        set->val.snodes[idx].in_ctx = 2;
                        temp_ctx = 1;
                    }
                }
            }
        }
    }
    lydict_remove(ctx, name_dict);

    /* correct temporary in_ctx values */
    if (temp_ctx) {
        for (i = 0; i < orig_used; ++i) {
            if (set->val.snodes[i].in_ctx == 2) {
                set->val.snodes[i].in_ctx = 1;
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to a node and all its descendants. Handles '//' and '*', 'NAME',
 *        'PREFIX:*', or 'PREFIX:NAME'. Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY).
 *        Context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] qname Qualified node name to move to.
 * @param[in] qname_len Length of \p qname.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, ECIT_FAILURE on unresolved when, -1 on error.
 */
static int
moveto_node_alldesc(struct lyxp_set *set, struct lyd_node *cur_node, const char *qname, uint16_t qname_len,
                    int options)
{
    uint32_t i;
    int pref_len, all = 0, replace, match, ret;
    struct lyd_node *next, *elem, *start;
    struct lys_module *moveto_mod;
    enum lyxp_node_type root_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    moveto_get_root(cur_node, options, &root_type);

    /* prefix */
    if (strnchr(qname, ':', qname_len) && cur_node) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, cur_node->schema->module->ctx, NULL, 1);
        if (!moveto_mod) {
            LOGINT;
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        moveto_mod = NULL;
    }

    /* replace the original nodes (and throws away all text and attr nodes, root is replaced by a child) */
    ret = moveto_node(set, cur_node, "*", 1, options);
    if (ret) {
        return ret;
    }

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    /* this loop traverses all the nodes in the set and addds/keeps only
     * those that match qname */
    for (i = 0; i < set->used; ) {
        /* TREE DFS */
        start = set->val.nodes[i].node;
        replace = 0;
        for (elem = next = start; elem; elem = next) {

            /* dummy and context check */
            if ((elem->validity & LYD_VAL_INUSE) || ((root_type == LYXP_NODE_ROOT_CONFIG) && (elem->schema->flags & LYS_CONFIG_R))) {
                goto skip_children;
            }

            match = 1;

            /* module check */
            if (moveto_mod && (lys_node_module(elem->schema) != moveto_mod)) {
                match = 0;
            }

            /* name check */
            if (!all && (strncmp(elem->schema->name, qname, qname_len) || elem->schema->name[qname_len])) {
                match = 0;
            }

            /* when check */
            if ((options & LYXP_WHEN) && !LYD_WHEN_DONE(elem->when_status)) {
                return EXIT_FAILURE;
            }

            if (match && (elem != start)) {
                if (set_dup_node_check(set, elem, LYXP_NODE_ELEM, i) > -1) {
                    /* we'll process it later */
                    goto skip_children;
                } else if (replace) {
                    set->val.nodes[i].node = elem;
                    assert(set->val.nodes[i].type == LYXP_NODE_ELEM);
                    set->val.nodes[i].pos = 0;
                    replace = 0;
                } else {
                    set_insert_node(set, elem, 0, LYXP_NODE_ELEM, i + 1);
                    ++i;
                }
            } else if (!match && (elem == start)) {
                /* we need to replace a node that is already in the set */
                replace = 1;
            }

            /* TREE DFS NEXT ELEM */
            /* select element for the next run - children first */
            next = elem->child;
            if (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
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

    return EXIT_SUCCESS;
}

static int
moveto_snode_alldesc(struct lyxp_set *set, struct lys_node *cur_node, const char *qname, uint16_t qname_len,
                     int options)
{
    int i, orig_used, pref_len, all = 0, match, idx;
    struct lys_node *next, *elem, *start;
    struct lys_module *moveto_mod;
    struct ly_ctx *ctx;
    enum lyxp_node_type root_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_SNODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    ctx = cur_node->module->ctx;
    moveto_snode_get_root(cur_node, options, &root_type);

    /* add all matching direct descendant nodes */
    idx = moveto_snode(set, cur_node, qname, qname_len, options);
    if (idx) {
        return idx;
    }

    /* prefix */
    if (strnchr(qname, ':', qname_len)) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, ctx, cur_node, 1);
        if (!moveto_mod) {
            LOGINT;
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        moveto_mod = NULL;
    }

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    orig_used = set->used;
    for (i = 0; i < orig_used; ++i) {
        if (set->val.snodes[i].in_ctx != 1) {
            continue;
        }

        /* TREE DFS */
        start = set->val.snodes[i].snode;
        for (elem = next = start; elem; elem = next) {

            /* context/nodetype check */
            if ((root_type == LYXP_NODE_ROOT_CONFIG) && (elem->flags & LYS_CONFIG_R)) {
                /* valid node, but it is hidden in this context */
                goto skip_children;
            }
            switch (elem->nodetype) {
            case LYS_USES:
            case LYS_CHOICE:
            case LYS_CASE:
                /* schema-only nodes */
                goto next_iter;
            case LYS_INPUT:
                if (options & LYXP_SNODE_OUTPUT) {
                    goto skip_children;
                }
                goto next_iter;
            case LYS_OUTPUT:
                if (!(options & LYXP_SNODE_OUTPUT)) {
                    goto skip_children;
                }
                goto next_iter;
            case LYS_GROUPING:
                goto skip_children;
            default:
                break;
            }

            match = 1;

            /* module check */
            if (moveto_mod && (lys_node_module(elem) != moveto_mod)) {
                match = 0;
            }

            /* name check */
            if (!all && (strncmp(elem->name, qname, qname_len) || elem->name[qname_len])) {
                match = 0;
            }

            if (match && (elem != start)) {
                if ((idx = set_snode_dup_node_check(set, elem, LYXP_NODE_ELEM, i)) > -1) {
                    set->val.snodes[idx].in_ctx = 1;
                    if (idx > i) {
                        /* we will process it later in the set */
                        goto skip_children;
                    }
                } else {
                    set_snode_insert_node(set, elem, LYXP_NODE_ELEM);
                }
            } else if (!match && (elem == start)) {
                /* start node must match! */
                LOGINT;
            }

next_iter:
            /* TREE DFS NEXT ELEM */
            /* select element for the next run - children first */
            next = elem->child;
            if (elem->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
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
                if (lys_parent(elem) == start) {
                    /* we are done, no next element to process */
                    break;
                }
                /* parent is already processed, go to its sibling */
                elem = lys_parent(elem);
                next = elem->next;
            }
        }
    }

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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
moveto_attr(struct lyxp_set *set, struct lyd_node *cur_node, const char *qname, uint16_t qname_len, int UNUSED(options))
{
    uint32_t i;
    int replaced, all = 0, pref_len;
    struct lys_module *moveto_mod;
    struct lyd_attr *sub;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    /* prefix */
    if (strnchr(qname, ':', qname_len) && cur_node) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, cur_node->schema->module->ctx, NULL, 1);
        if (!moveto_mod) {
            LOGINT;
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        moveto_mod = NULL;
    }

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    for (i = 0; i < set->used; ) {
        replaced = 0;

        /* only attributes of an elem (not dummy) can be in the result, skip all the rest;
         * our attributes are always qualified */
        if ((set->val.nodes[i].type == LYXP_NODE_ELEM) && !(set->val.nodes[i].node->validity & LYD_VAL_INUSE)) {
            LY_TREE_FOR(set->val.nodes[i].node->attr, sub) {

                /* check "namespace" */
                if (moveto_mod && (sub->module != moveto_mod)) {
                    /* no match */
                    continue;
                }

                if (all || (!strncmp(sub->name, qname, qname_len) && !sub->name[qname_len])) {
                    /* match */
                    if (!replaced) {
                        set->val.attrs[i].attr = sub;
                        set->val.attrs[i].type = LYXP_NODE_ATTR;
                        /* pos does not change */
                        replaced = 1;
                    } else {
                        set_insert_node(set, (struct lyd_node *)sub, set->val.nodes[i].pos, LYXP_NODE_ATTR, i + 1);
                    }
                    ++i;
                }
            }
        }

        if (!replaced) {
            /* no match */
            set_remove_node(set, i);
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set1 to union with \p set2. \p set2 is emptied afterwards.
 *        Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY). Context position aware.
 *
 * @param[in,out] set1 Set to use for the result.
 * @param[in] set2 Set that is copied to \p set1.
 * @param[in] cur_node Original context node.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
moveto_union(struct lyxp_set *set1, struct lyxp_set *set2, struct lyd_node *cur_node, int options)
{
    if (((set1->type != LYXP_SET_NODE_SET) && (set1->type != LYXP_SET_EMPTY))
            || ((set2->type != LYXP_SET_NODE_SET) && (set2->type != LYXP_SET_EMPTY))) {
        LOGVAL(LYE_XPATH_INOP_2, LY_VLOG_NONE, NULL, "union", print_set_type(set1), print_set_type(set2));
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

#ifndef NDEBUG
    /* we assume sets are sorted */
    if ((set_sort(set1, cur_node, options) > 1) || (set_sort(set2, cur_node, options) > 1)) {
        LOGERR(LY_EINT, "XPath set was expected to be sorted, but is not (%s).", __func__);
    }
#endif

    /* sort, remove duplicates */
    if (set_sorted_merge(set1, set2, cur_node, options)) {
        return -1;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to an attribute in any of the descendants. Handles '//' and '@*',
 *        '@NAME', '@PREFIX:*', or '@PREFIX:NAME'. Result is LYXP_SET_NODE_SET (or LYXP_SET_EMPTY).
 *        Context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] qname Qualified node name to move to.
 * @param[in] qname_len Length of \p qname.
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
moveto_attr_alldesc(struct lyxp_set *set, struct lyd_node *cur_node, const char *qname, uint16_t qname_len,
                    int options)
{
    uint32_t i;
    int pref_len, replaced, all = 0, ret;
    struct lyd_attr *sub;
    struct lys_module *moveto_mod;
    struct lyxp_set *set_all_desc = NULL;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    /* prefix */
    if (strnchr(qname, ':', qname_len)) {
        pref_len = strnchr(qname, ':', qname_len) - qname;
        moveto_mod = moveto_resolve_model(qname, pref_len, cur_node->schema->module->ctx, NULL, 1);
        if (!moveto_mod) {
            LOGINT;
            return -1;
        }
        qname += pref_len + 1;
        qname_len -= pref_len + 1;
    } else {
        moveto_mod = NULL;
    }

    /* can be optimized similarly to moveto_node_alldesc() and save considerable amount of memory,
     * but it likely won't be used much, so it's a waste of time */
    /* copy the context */
    set_all_desc = set_copy(set);
    /* get all descendant nodes (the original context nodes are removed) */
    ret = moveto_node_alldesc(set_all_desc, cur_node, "*", 1, options);
    if (ret) {
        lyxp_set_free(set_all_desc);
        return ret;
    }
    /* prepend the original context nodes */
    if (moveto_union(set, set_all_desc, cur_node, options)) {
        lyxp_set_free(set_all_desc);
        return -1;
    }
    lyxp_set_free(set_all_desc);

    if ((qname_len == 1) && (qname[0] == '*')) {
        all = 1;
    }

    for (i = 0; i < set->used; ) {
        replaced = 0;

        /* only attributes of an elem can be in the result, skip all the rest,
         * we have all attributes qualified in lyd tree */
        if (set->val.nodes[i].type == LYXP_NODE_ELEM) {
            LY_TREE_FOR(set->val.nodes[i].node->attr, sub) {
                /* check "namespace" */
                if (moveto_mod && (sub->module != moveto_mod)) {
                    /* no match */
                    continue;
                }

                if (all || (!strncmp(sub->name, qname, qname_len) && !sub->name[qname_len])) {
                    /* match */
                    if (!replaced) {
                        set->val.attrs[i].attr = sub;
                        set->val.attrs[i].type = LYXP_NODE_ATTR;
                        /* pos does not change */
                        replaced = 1;
                    } else {
                        set_insert_node(set, (struct lyd_node *)sub, set->val.attrs[i].pos, LYXP_NODE_ATTR, i + 1);
                    }
                    ++i;
                }
            }
        }

        if (!replaced) {
            /* no match */
            set_remove_node(set, i);
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to self. Handles '/' or '//' and '.'. Result is LYXP_SET_NODE_SET
 *        (or LYXP_SET_EMPTY). Context position aware.
 *
 * @param[in,out] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] all_desc Whether to go to all descendants ('//') or not ('/').
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
moveto_self(struct lyxp_set *set, struct lyd_node *cur_node, int all_desc, int options)
{
    struct lyd_node *sub;
    uint32_t i, cont_i;
    enum lyxp_node_type root_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    /* nothing to do */
    if (!all_desc) {
        return EXIT_SUCCESS;
    }

    moveto_get_root(cur_node, options, &root_type);

    /* add all the children, they get added recursively */
    for (i = 0; i < set->used; ++i) {
        cont_i = 0;

        /* do not touch attributes and text nodes */
        if ((set->val.nodes[i].type == LYXP_NODE_TEXT) || (set->val.nodes[i].type == LYXP_NODE_ATTR)) {
            continue;
        }

        /* skip anydata/anyxml and dummy nodes */
        if ((set->val.nodes[i].node->schema->nodetype & LYS_ANYDATA) || (set->val.nodes[i].node->validity & LYD_VAL_INUSE)) {
            continue;
        }

        /* add all the children ... */
        if (!(set->val.nodes[i].node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST))) {
            LY_TREE_FOR(set->val.nodes[i].node->child, sub) {
                /* context check */
                if ((root_type == LYXP_NODE_ROOT_CONFIG) && (sub->schema->flags & LYS_CONFIG_R)) {
                    continue;
                }

                /* when check */
                if ((options & LYXP_WHEN) && !LYD_WHEN_DONE(sub->when_status)) {
                    return EXIT_FAILURE;
                }

                if (set_dup_node_check(set, sub, LYXP_NODE_ELEM, -1) == -1) {
                    set_insert_node(set, sub, 0, LYXP_NODE_ELEM, i + cont_i + 1);
                    ++cont_i;
                }
            }

        /* ... or add their text node, ... */
        } else {
            /* ... but only non-empty */
            sub = set->val.nodes[i].node;
            if (((struct lyd_node_leaf_list *)sub)->value_str) {
                if (set_dup_node_check(set, sub, LYXP_NODE_TEXT, -1) == -1) {
                    set_insert_node(set, sub, set->val.nodes[i].pos, LYXP_NODE_TEXT, i + 1);
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

static int
moveto_snode_self(struct lyxp_set *set, struct lys_node *cur_node, int all_desc, int options)
{
    const struct lys_node *sub;
    uint32_t i;
    enum lyxp_node_type root_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_SNODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    /* nothing to do */
    if (!all_desc) {
        return EXIT_SUCCESS;
    }

    moveto_snode_get_root(cur_node, options, &root_type);

    /* add all the children, they get added recursively */
    for (i = 0; i < set->used; ++i) {
        if (set->val.snodes[i].in_ctx != 1) {
            continue;
        }

        /* add all the children */
        if (set->val.snodes[i].snode->nodetype & (LYS_LIST | LYS_CONTAINER)) {
            sub = NULL;
            while ((sub = lys_getnext(sub, set->val.snodes[i].snode, NULL, 0))) {
                /* RPC input/output check */
                if (options & LYXP_SNODE_OUTPUT) {
                    if (lys_parent(sub)->nodetype == LYS_INPUT) {
                        continue;
                    }
                } else {
                    if (lys_parent(sub)->nodetype == LYS_OUTPUT) {
                        continue;
                    }
                }

                /* context check */
                if ((root_type == LYXP_NODE_ROOT_CONFIG) && (sub->flags & LYS_CONFIG_R)) {
                    continue;
                }

                set_snode_insert_node(set, sub, LYXP_NODE_ELEM);
                /* throw away the insert index, we want to consider that node again, recursively */
            }
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Move context \p set to parent. Handles '/' or '//' and '..'. Result is LYXP_SET_NODE_SET
 *        (or LYXP_SET_EMPTY). Context position aware.
 *
 * @param[in] set Set to use.
 * @param[in] cur_node Original context node.
 * @param[in] all_desc Whether to go to all descendants ('//') or not ('/').
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
moveto_parent(struct lyxp_set *set, struct lyd_node *cur_node, int all_desc, int options)
{
    int ret;
    uint32_t i;
    struct lyd_node *node, *new_node;
    const struct lyd_node *root;
    enum lyxp_node_type root_type, new_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_NODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    if (all_desc) {
        /* <path>//.. == <path>//./.. */
        ret = moveto_self(set, cur_node, 1, options);
        if (ret) {
            return ret;
        }
    }

    root = moveto_get_root(cur_node, options, &root_type);

    for (i = 0; i < set->used; ) {
        node = set->val.nodes[i].node;

        if (set->val.nodes[i].type == LYXP_NODE_ELEM) {
            new_node = node->parent;
        } else if (set->val.nodes[i].type == LYXP_NODE_TEXT) {
            new_node = node;
        } else if (set->val.nodes[i].type == LYXP_NODE_ATTR) {
            new_node = (struct lyd_node *)lyd_attr_parent(root, set->val.attrs[i].attr);
            if (!new_node) {
                LOGINT;
                return -1;
            }
        } else {
            /* root does not have a parent */
            set_remove_node(set, i);
            continue;
        }

        /* when check */
        if ((options & LYXP_WHEN) && new_node && !LYD_WHEN_DONE(new_node->when_status)) {
            return EXIT_FAILURE;
        }

        /* node already there can also be the root */
        if (root == node) {
            if (options && (cur_node->schema->flags & LYS_CONFIG_W)) {
                new_type = LYXP_NODE_ROOT_CONFIG;
            } else {
                new_type = LYXP_NODE_ROOT;
            }
            new_node = node;

        /* node has no parent */
        } else if (!new_node) {
            if (options && (cur_node->schema->flags & LYS_CONFIG_W)) {
                new_type = LYXP_NODE_ROOT_CONFIG;
            } else {
                new_type = LYXP_NODE_ROOT;
            }
#ifndef NDEBUG
            for (; node->prev->next; node = node->prev);
            if (node != root) {
                LOGINT;
            }
#endif
            new_node = (struct lyd_node *)root;

        /* node has a standard parent (it can equal the root, it's not the root yet since they are fake) */
        } else {
            new_type = LYXP_NODE_ELEM;
        }

        assert((new_type == LYXP_NODE_ELEM) || ((new_type == root_type) && (new_node == root)));

        if (set_dup_node_check(set, new_node, new_type, -1) > -1) {
            set_remove_node(set, i);
        } else {
            set->val.nodes[i].node = new_node;
            set->val.nodes[i].type = new_type;
            set->val.nodes[i].pos = 0;

            ++i;
        }
    }

#ifndef NDEBUG
    if (set_sort(set, cur_node, options) > 1) {
        LOGERR(LY_EINT, "XPath set was expected to be sorted, but is not (%s).", __func__);
    }
    if (set_sorted_dup_node_clean(set)) {
        LOGERR(LY_EINT, "XPath set includes duplicates (%s).", __func__);
    }
#endif

    return EXIT_SUCCESS;
}

static int
moveto_snode_parent(struct lyxp_set *set, struct lys_node *cur_node, int all_desc, int options)
{
    int idx, i, orig_used, temp_ctx = 0;
    struct lys_node *node, *new_node;
    const struct lys_node *root;
    enum lyxp_node_type root_type, new_type;

    if (!set || (set->type == LYXP_SET_EMPTY)) {
        return EXIT_SUCCESS;
    }

    if (set->type != LYXP_SET_SNODE_SET) {
        LOGVAL(LYE_XPATH_INOP_1, LY_VLOG_NONE, NULL, "path operator", print_set_type(set));
        return -1;
    }

    if (all_desc) {
        /* <path>//.. == <path>//./.. */
        idx = moveto_snode_self(set, cur_node, 1, options);
        if (idx) {
            return idx;
        }
    }

    root = moveto_snode_get_root(cur_node, options, &root_type);

    orig_used = set->used;
    for (i = 0; i < orig_used; ++i) {
        if (set->val.snodes[i].in_ctx != 1) {
            continue;
        }
        set->val.snodes[i].in_ctx = 0;

        node = set->val.snodes[i].snode;

        if (set->val.snodes[i].type == LYXP_NODE_ELEM) {
            for (new_node = lys_parent(node);
                 new_node && (new_node->nodetype & (LYS_USES | LYS_CHOICE | LYS_CASE));
                 new_node = lys_parent(new_node));
        } else {
            /* root does not have a parent */
            continue;
        }

        /* node already there can also be the root */
        if (root == node) {
            if ((options & (LYXP_SNODE_MUST | LYXP_SNODE_WHEN)) && (cur_node->flags & LYS_CONFIG_W)) {
                new_type = LYXP_NODE_ROOT_CONFIG;
            } else {
                new_type = LYXP_NODE_ROOT;
            }
            new_node = node;

        /* node has no parent */
        } else if (!new_node) {
            if ((options & (LYXP_SNODE_MUST | LYXP_SNODE_WHEN)) && (cur_node->flags & LYS_CONFIG_W)) {
                new_type = LYXP_NODE_ROOT_CONFIG;
            } else {
                new_type = LYXP_NODE_ROOT;
            }
#ifndef NDEBUG
            node = (struct lys_node *)lys_getnext(NULL, NULL, lys_node_module(node), 0);
            if (node != root) {
                LOGINT;
            }
#endif
            new_node = (struct lys_node *)root;

        /* node has a standard parent (it can equal the root, it's not the root yet since they are fake) */
        } else {
            new_type = LYXP_NODE_ELEM;
        }

        assert((new_type == LYXP_NODE_ELEM) || ((new_type == root_type) && (new_node == root)));

        idx = set_snode_insert_node(set, new_node, new_type);
        if ((idx < orig_used) && (idx > i)) {
            set->val.snodes[idx].in_ctx = 2;
            temp_ctx = 1;
        }
    }

    if (temp_ctx) {
        for (i = 0; i < orig_used; ++i) {
            if (set->val.snodes[i].in_ctx == 2) {
                set->val.snodes[i].in_ctx = 1;
            }
        }
    }

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
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
moveto_op_comp(struct lyxp_set *set1, struct lyxp_set *set2, const char *op, struct lyd_node *cur_node,
               int options)
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

    /* we can evaluate it immediately */
    if ((set1->type == set2->type) && (set1->type != LYXP_SET_EMPTY) && (set1->type != LYXP_SET_NODE_SET)
            && (((op[0] == '=') || (op[0] == '!')) || ((set1->type != LYXP_SET_BOOLEAN) && (set1->type != LYXP_SET_STRING)))) {

        /* compute result */
        if (op[0] == '=') {
            if (set1->type == LYXP_SET_BOOLEAN) {
                result = (set1->val.bool == set2->val.bool);
            } else if (set1->type == LYXP_SET_NUMBER) {
                result = (set1->val.num == set2->val.num);
            } else {
                result = (ly_strequal(set1->val.str, set2->val.str, 0));
            }
        } else if (op[0] == '!') {
            if (set1->type == LYXP_SET_BOOLEAN) {
                result = (set1->val.bool != set2->val.bool);
            } else if (set1->type == LYXP_SET_NUMBER) {
                result = (set1->val.num != set2->val.num);
            } else {
                result = (!ly_strequal(set1->val.str, set2->val.str, 0));
            }
        } else {
            if (set1->type != LYXP_SET_NUMBER) {
                LOGINT;
                return -1;
            }

            if (op[0] == '<') {
                if (op[1] == '=') {
                    result = (set1->val.num <= set2->val.num);
                } else {
                    result = (set1->val.num < set2->val.num);
                }
            } else {
                if (op[1] == '=') {
                    result = (set1->val.num >= set2->val.num);
                } else {
                    result = (set1->val.num > set2->val.num);
                }
            }
        }

        /* assign result */
        if (result) {
            set_fill_boolean(set1, 1);
        } else {
            set_fill_boolean(set1, 0);
        }

        lyxp_set_cast(set2, LYXP_SET_EMPTY, cur_node, options);
        return EXIT_SUCCESS;
    }

    /* convert first */
    if (((set1->type == LYXP_SET_NODE_SET) || (set1->type == LYXP_SET_EMPTY) || (set1->type == LYXP_SET_STRING))
            && ((set2->type == LYXP_SET_NODE_SET) || (set2->type == LYXP_SET_EMPTY) || (set2->type == LYXP_SET_STRING))
            && ((set1->type != LYXP_SET_STRING) || (set2->type != LYXP_SET_STRING))) {
        if (lyxp_set_cast(set1, LYXP_SET_STRING, cur_node, options)) {
            return -1;
        }
        if (lyxp_set_cast(set2, LYXP_SET_STRING, cur_node, options)) {
            return -1;
        }

    } else if ((((set1->type == LYXP_SET_NODE_SET) || (set1->type == LYXP_SET_EMPTY) || (set1->type == LYXP_SET_BOOLEAN))
            && ((set2->type == LYXP_SET_NODE_SET) || (set2->type == LYXP_SET_EMPTY) || (set2->type == LYXP_SET_BOOLEAN)))
            || (((op[0] == '=') || (op[0] == '!')) && ((set1->type == LYXP_SET_BOOLEAN) || (set2->type == LYXP_SET_BOOLEAN)))) {
        lyxp_set_cast(set1, LYXP_SET_BOOLEAN, cur_node, options);
        lyxp_set_cast(set2, LYXP_SET_BOOLEAN, cur_node, options);

    } else {
        if (lyxp_set_cast(set1, LYXP_SET_NUMBER, cur_node, options)) {
            return -1;
        }
        if (lyxp_set_cast(set2, LYXP_SET_NUMBER, cur_node, options)) {
            return -1;
        }
    }

    /* now we can evaluate */
    return moveto_op_comp(set1, set2, op, cur_node, options);
}

/**
 * @brief Move context \p set to the result of a basic operation. Handles '+', '-', unary '-', '*', 'div',
 *        or 'mod'. Result is LYXP_SET_NUMBER. Indirectly context position aware.
 *
 * @param[in,out] set1 Set to use for the result.
 * @param[in] set2 Set acting as the second operand for \p op.
 * @param[in] op Operator to process.
 * @param[in] cur_node Original context node.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
moveto_op_math(struct lyxp_set *set1, struct lyxp_set *set2, const char *op, struct lyd_node *cur_node,
               int options)
{
    /* unary '-' */
    if (!set2 && (op[0] == '-')) {
        if (lyxp_set_cast(set1, LYXP_SET_NUMBER, cur_node, options)) {
            return -1;
        }
        set1->val.num *= -1;
        lyxp_set_free(set2);
        return EXIT_SUCCESS;
    }

    assert(set1 && set2);

    if (lyxp_set_cast(set1, LYXP_SET_NUMBER, cur_node, options)) {
        return -1;
    }
    if (lyxp_set_cast(set2, LYXP_SET_NUMBER, cur_node, options)) {
        return -1;
    }

    switch (op[0]) {
    /* '+' */
    case '+':
        set1->val.num += set2->val.num;
        break;

    /* '-' */
    case '-':
        set1->val.num -= set2->val.num;
        break;

    /* '*' */
    case '*':
        set1->val.num *= set2->val.num;
        break;

    /* 'div' */
    case 'd':
        set1->val.num /= set2->val.num;
        break;

    /* 'mod' */
    case 'm':
        set1->val.num = ((long long)set1->val.num) % ((long long)set2->val.num);
        break;

    default:
        LOGINT;
        return -1;
    }

    return EXIT_SUCCESS;
}

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
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static void
eval_literal(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyxp_set *set)
{
    if (set) {
        if (exp->tok_len[*exp_idx] == 2) {
            set_fill_string(set, "", 0);
        } else {
            set_fill_string(set, &exp->expr[exp->expr_pos[*exp_idx] + 1], exp->tok_len[*exp_idx] - 2);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_node_test(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, int attr_axis, int all_desc,
               struct lyxp_set *set, int options)
{
    int i, rc = 0;

    switch (exp->tokens[*exp_idx]) {
    case LYXP_TOKEN_NAMETEST:
        if (attr_axis) {
            if (set && (options & LYXP_SNODE_ALL)) {
                set_snode_clear_ctx(set);
            } else {
                if (all_desc) {
                    rc = moveto_attr_alldesc(set, cur_node, &exp->expr[exp->expr_pos[*exp_idx]],
                                             exp->tok_len[*exp_idx], options);
                } else {
                    rc = moveto_attr(set, cur_node, &exp->expr[exp->expr_pos[*exp_idx]], exp->tok_len[*exp_idx],
                                     options);
                }
            }
        } else {
            if (all_desc) {
                if (set && (options & LYXP_SNODE_ALL)) {
                    rc = moveto_snode_alldesc(set, (struct lys_node *)cur_node, &exp->expr[exp->expr_pos[*exp_idx]],
                                              exp->tok_len[*exp_idx], options);
                } else {
                    rc = moveto_node_alldesc(set, cur_node, &exp->expr[exp->expr_pos[*exp_idx]],
                                             exp->tok_len[*exp_idx], options);
                }
            } else {
                if (set && (set->type == LYXP_SET_SNODE_SET)) {
                    rc = moveto_snode(set, (struct lys_node *)cur_node, &exp->expr[exp->expr_pos[*exp_idx]],
                                      exp->tok_len[*exp_idx], options);
                } else {
                    rc = moveto_node(set, cur_node, &exp->expr[exp->expr_pos[*exp_idx]], exp->tok_len[*exp_idx],
                                     options);
                }
            }

            if (!rc && set && (options & LYXP_SNODE_ALL)) {
                for (i = set->used - 1; i > -1; --i) {
                    if (set->val.snodes[i].in_ctx) {
                        break;
                    }
                }
                if (i == -1) {
                    LOGVAL(LYE_XPATH_INSNODE, LY_VLOG_NONE, NULL,
                           exp->tok_len[*exp_idx], &exp->expr[exp->expr_pos[*exp_idx]],
                           exp->expr_pos[*exp_idx] + exp->tok_len[*exp_idx], exp->expr);
                    return -1;
                }
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
            if (set->type == LYXP_SET_SNODE_SET) {
                set_snode_clear_ctx(set);
                /* just for the debug message underneath */
                set = NULL;
            } else {
                if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "node", 4)) {
                    if (xpath_node(NULL, 0, cur_node, set, options)) {
                        return -1;
                    }
                } else {
                    assert(!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "text", 4));
                    if (xpath_text(NULL, 0, cur_node, set, options)) {
                        return -1;
                    }
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_predicate(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
               int options)
{
    int ret;
    uint16_t i, j, orig_exp, brack2_exp;
    uint32_t orig_pos, orig_size, pred_in_ctx;
    uint8_t **pred_repeat, rep_size;
    struct lyxp_set set2;

    /* '[' */
    LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
           print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
    ++(*exp_idx);

    if (!set) {
        ret = eval_expr(exp, exp_idx, cur_node, NULL, options);
        if (ret) {
            return ret;
        }
    } else if (set->type == LYXP_SET_NODE_SET) {
#ifndef NDEBUG
        /* we (possibly) need the set sorted, it can affect the result (if the predicate result is a number) */
        if (set_sort(set, cur_node, options) > 1) {
            LOGERR(LY_EINT, "XPath set was expected to be sorted, but is not (%s).", __func__);
        }
#endif

        orig_exp = *exp_idx;

        /* find the predicate end */
        for (brack2_exp = orig_exp; exp->tokens[brack2_exp] != LYXP_TOKEN_BRACK2; ++brack2_exp);

        /* copy predicate repeats, since they get deleted each time (probably not an ideal solution) */
        pred_repeat = calloc(brack2_exp - orig_exp, sizeof *pred_repeat);
        if (!pred_repeat) {
            LOGMEM;
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
                    return -1;
                }
                memcpy(pred_repeat[j], exp->repeat[orig_exp + j], rep_size * sizeof **pred_repeat);
            }
        }

        orig_size = set->used;
        for (i = 0, orig_pos = 1; i < set->used; ++orig_pos) {
            set2.type = LYXP_SET_EMPTY;
            set_insert_node(&set2, set->val.nodes[i].node, set->val.nodes[i].pos, set->val.nodes[i].type, 0);
            /* remember the node context position for position() and context size for last() */
            set2.ctx_pos = orig_pos;
            set2.ctx_size = orig_size;
            *exp_idx = orig_exp;

            /* replace repeats */
            for (j = 0; j < brack2_exp - orig_exp; ++j) {
                if (pred_repeat[j]) {
                    for (rep_size = 0; pred_repeat[j][rep_size]; ++rep_size);
                    ++rep_size;
                    memcpy(exp->repeat[orig_exp + j], pred_repeat[j], rep_size * sizeof **pred_repeat);
                }
            }

            ret = eval_expr(exp, exp_idx, cur_node, &set2, options);
            if (ret) {
                for (j = 0; j < brack2_exp - orig_exp; ++j) {
                    free(pred_repeat[j]);
                }
                free(pred_repeat);
                lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, options);
                return ret;
            }

            /* number is a position */
            if (set2.type == LYXP_SET_NUMBER) {
                if ((long long)set2.val.num == orig_pos) {
                    set2.val.num = 1;
                } else {
                    set2.val.num = 0;
                }
            }
            lyxp_set_cast(&set2, LYXP_SET_BOOLEAN, cur_node, options);

            /* predicate satisfied or not? */
            if (set2.val.bool) {
                ++i;
            } else {
                set_remove_node(set, i);
            }
        }

        /* free predicate repeats */
        for (j = 0; j < brack2_exp - orig_exp; ++j) {
            free(pred_repeat[j]);
        }
        free(pred_repeat);

    } else if (set->type == LYXP_SET_SNODE_SET) {
        orig_exp = *exp_idx;

        /* find the predicate end */
        for (brack2_exp = orig_exp; exp->tokens[brack2_exp] != LYXP_TOKEN_BRACK2; ++brack2_exp);

        /* copy predicate repeats, since they get deleted each time (probably not an ideal solution) */
        pred_repeat = calloc(brack2_exp - orig_exp, sizeof *pred_repeat);
        if (!pred_repeat) {
            LOGMEM;
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
                    return -1;
                }
                memcpy(pred_repeat[j], exp->repeat[orig_exp + j], rep_size * sizeof **pred_repeat);
            }
        }

        /* set special in_ctx to all the valid snodes */
        pred_in_ctx = set_snode_new_in_ctx(set);

        /* use the valid snodes one-by-one */
        for (i = 0; i < set->used; ++i) {
            if (set->val.snodes[i].in_ctx != pred_in_ctx) {
                continue;
            }
            set->val.snodes[i].in_ctx = 1;

            *exp_idx = orig_exp;

            /* replace repeats */
            for (j = 0; j < brack2_exp - orig_exp; ++j) {
                if (pred_repeat[j]) {
                    for (rep_size = 0; pred_repeat[j][rep_size]; ++rep_size);
                    ++rep_size;
                    memcpy(exp->repeat[orig_exp + j], pred_repeat[j], rep_size * sizeof **pred_repeat);
                }
            }

            ret = eval_expr(exp, exp_idx, cur_node, set, options);
            if (ret) {
                for (j = 0; j < brack2_exp - orig_exp; ++j) {
                    free(pred_repeat[j]);
                }
                free(pred_repeat);
                return ret;
            }

            set->val.snodes[i].in_ctx = pred_in_ctx;
        }

        /* restore the state as it was before the predicate */
        for (i = 0; i < set->used; ++i) {
            if (set->val.snodes[i].in_ctx == 1) {
                set->val.snodes[i].in_ctx = 0;
            } else if (set->val.snodes[i].in_ctx == pred_in_ctx) {
                set->val.snodes[i].in_ctx = 1;
            }
        }

        /* free predicate repeats */
        for (j = 0; j < brack2_exp - orig_exp; ++j) {
            free(pred_repeat[j]);
        }
        free(pred_repeat);
    } else {
        set2.type = LYXP_SET_EMPTY;
        set_fill_set(&set2, set);

        ret = eval_expr(exp, exp_idx, cur_node, &set2, options);
        if (ret) {
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, options);
            return ret;
        }

        lyxp_set_cast(&set2, LYXP_SET_BOOLEAN, cur_node, options);
        if (!set2.val.bool) {
            lyxp_set_cast(set, LYXP_SET_EMPTY, cur_node, options);
        }
        lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, options);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_relative_location_path(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, int all_desc,
                            struct lyxp_set *set, int options)
{
    int attr_axis, ret;

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
            if (set && (options & LYXP_SNODE_ALL)) {
                ret = moveto_snode_self(set, (struct lys_node *)cur_node, all_desc, options);
            } else {
                ret = moveto_self(set, cur_node, all_desc, options);
            }
            if (ret) {
                return ret;
            }
            LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
            ++(*exp_idx);
            break;
        case LYXP_TOKEN_DDOT:
            /* evaluate '..' */
            if (set && (options & LYXP_SNODE_ALL)) {
                ret = moveto_snode_parent(set, (struct lys_node *)cur_node, all_desc, options);
            } else {
                ret = moveto_parent(set, cur_node, all_desc, options);
            }
            if (ret) {
                return ret;
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
            ret = eval_node_test(exp, exp_idx, cur_node, attr_axis, all_desc, set, options);
            if (ret) {
                return ret;
            }
            while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_BRACK1)) {
                ret = eval_predicate(exp, exp_idx, cur_node, set, options);
                if (ret) {
                    return ret;
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_absolute_location_path(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node,
                            struct lyxp_set *set, int options)
{
    int all_desc, ret;

    if (set) {
        /* no matter what tokens follow, we need to be at the root */
        if (options & LYXP_SNODE_ALL) {
            moveto_snode_root(set, (struct lys_node *)cur_node, options);
        } else {
            moveto_root(set, cur_node, options);
        }
    }

    /* '/' RelativeLocationPath? */
    if (exp->tok_len[*exp_idx] == 1) {
        /* evaluate '/' - deferred */
        all_desc = 0;
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        if (exp_check_token(exp, *exp_idx, LYXP_TOKEN_NONE, 0)) {
            return EXIT_SUCCESS;
        }
        switch (exp->tokens[*exp_idx]) {
        case LYXP_TOKEN_DOT:
        case LYXP_TOKEN_DDOT:
        case LYXP_TOKEN_AT:
        case LYXP_TOKEN_NAMETEST:
        case LYXP_TOKEN_NODETYPE:
            ret = eval_relative_location_path(exp, exp_idx, cur_node, all_desc, set, options);
            if (ret) {
                return ret;
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

        ret =  eval_relative_location_path(exp, exp_idx, cur_node, all_desc, set, options);
        if (ret) {
            return ret;
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_function_call(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
                   int options)
{
    int rc = EXIT_FAILURE;
    int (*xpath_func)(struct lyxp_set **, uint16_t, struct lyd_node *, struct lyxp_set *, int) = NULL;
    uint16_t arg_count = 0, i;
    struct lyxp_set **args = NULL, **args_aux;

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
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "deref", 5)) {
                xpath_func = &xpath_deref;
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
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "re-match", 8)) {
                xpath_func = &xpath_re_match;
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
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "enum-value", 10)) {
                xpath_func = &xpath_enum_value;
            } else if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "bit-is-set", 10)) {
                xpath_func = &xpath_bit_is_set;
            }
            break;
        case 11:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "starts-with", 11)) {
                xpath_func = &xpath_starts_with;
            }
            break;
        case 12:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "derived-from", 12)) {
                xpath_func = &xpath_derived_from;
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
        case 20:
            if (!strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "derived-from-or-self", 20)) {
                xpath_func = &xpath_derived_from_or_self;
            }
            break;
        }

        if (!xpath_func) {
            LOGVAL(LYE_XPATH_INTOK, LY_VLOG_NONE, NULL, "Unknown", &exp->expr[exp->expr_pos[*exp_idx]]);
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
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
            args = malloc(sizeof *args);
            if (!args) {
                LOGMEM;
                goto cleanup;
            }
            arg_count = 1;
            args[0] = set_copy(set);
            if (!args[0]) {
                goto cleanup;
            }

            if ((rc = eval_expr(exp, exp_idx, cur_node, args[0], options))) {
                goto cleanup;
            }
        } else {
            if ((rc = eval_expr(exp, exp_idx, cur_node, NULL, options))) {
                goto cleanup;
            }
        }
    }
    while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_COMMA)) {
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        if (set) {
            ++arg_count;
            args_aux = realloc(args, arg_count * sizeof *args);
            if (!args_aux) {
                arg_count--;
                LOGMEM;
                goto cleanup;
            }
            args = args_aux;
            args[arg_count - 1] = set_copy(set);
            if (!args[arg_count - 1]) {
                goto cleanup;
            }

            if ((rc = eval_expr(exp, exp_idx, cur_node, args[arg_count - 1], options))) {
                goto cleanup;
            }
        } else {
            if ((rc = eval_expr(exp, exp_idx, cur_node, NULL, options))) {
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
        rc = xpath_func(args, arg_count, cur_node, set, options);
    } else {
        rc = EXIT_SUCCESS;
    }

cleanup:
    for (i = 0; i < arg_count; ++i) {
        lyxp_set_free(args[i]);
    }
    free(args);

    return rc;
}

/**
 * @brief Evaluate Number. Logs directly on error.
 *
 * @param[in] exp Parsed XPath expression.
 * @param[in] exp_idx Position in the expression \p exp.
 * @param[in,out] set Context and result set. On NULL the rule is only parsed.
 *
 * @return EXIT_SUCCESS on success, -1 on error.
 */
static int
eval_number(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyxp_set *set)
{
    long double num;
    char *endptr;

    if (set) {
        errno = 0;
        num = strtold(&exp->expr[exp->expr_pos[*exp_idx]], &endptr);
        if (errno) {
            LOGVAL(LYE_XPATH_INTOK, LY_VLOG_NONE, NULL, "Unknown", &exp->expr[exp->expr_pos[*exp_idx]]);
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Failed to convert \"%.*s\" into a long double (%s).",
                   exp->tok_len[*exp_idx], &exp->expr[exp->expr_pos[*exp_idx]], strerror(errno));
            return -1;
        } else if (endptr - &exp->expr[exp->expr_pos[*exp_idx]] != exp->tok_len[*exp_idx]) {
            LOGVAL(LYE_XPATH_INTOK, LY_VLOG_NONE, NULL, "Unknown", &exp->expr[exp->expr_pos[*exp_idx]]);
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Failed to convert \"%.*s\" into a long double.",
                   exp->tok_len[*exp_idx], &exp->expr[exp->expr_pos[*exp_idx]]);
            return -1;
        }

        set_fill_number(set, num);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_path_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
               int options)
{
    int all_desc, ret;

    switch (exp->tokens[*exp_idx]) {
    case LYXP_TOKEN_PAR1:
        /* '(' Expr ')' */

        /* '(' */
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        /* Expr */
        ret = eval_expr(exp, exp_idx, cur_node, set, options);
        if (ret) {
            return ret;
        }

        /* ')' */
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (set ? "parsed" : "skipped"),
               print_token(exp->tokens[*exp_idx]), exp->expr_pos[*exp_idx]);
        ++(*exp_idx);

        goto predicate;

    case LYXP_TOKEN_DOT:
    case LYXP_TOKEN_DDOT:
    case LYXP_TOKEN_AT:
    case LYXP_TOKEN_NAMETEST:
    case LYXP_TOKEN_NODETYPE:
        /* RelativeLocationPath */
        ret = eval_relative_location_path(exp, exp_idx, cur_node, 0, set, options);
        if (ret) {
            return ret;
        }
        break;

    case LYXP_TOKEN_FUNCNAME:
        /* FunctionCall */
        if (!set || (options & LYXP_SNODE_ALL)) {
            if (set) {
                /* the only function returning node-set - thus relevant */
                if ((exp->tok_len[*exp_idx] == 7) && !strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "current", 7)) {
                    xpath_current(NULL, 0, cur_node, set, options);
                } else if ((exp->tok_len[*exp_idx] == 5) && !strncmp(&exp->expr[exp->expr_pos[*exp_idx]], "deref", 5)) {
                    ret = eval_function_call(exp, exp_idx, cur_node, set, options);
                    if (ret) {
                        return ret;
                    }
                    goto predicate;
                } else {
                    set_snode_clear_ctx(set);
                }
            }
            ret = eval_function_call(exp, exp_idx, cur_node, NULL, options);
        } else {
            ret = eval_function_call(exp, exp_idx, cur_node, set, options);
        }
        if (ret) {
            return ret;
        }

        goto predicate;

    case LYXP_TOKEN_OPERATOR_PATH:
        /* AbsoluteLocationPath */
        ret = eval_absolute_location_path(exp, exp_idx, cur_node, set, options);
        if (ret) {
            return ret;
        }
        break;

    case LYXP_TOKEN_LITERAL:
        /* Literal */
        if (!set || (options & LYXP_SNODE_ALL)) {
            if (set) {
                set_snode_clear_ctx(set);
            }
            eval_literal(exp, exp_idx, NULL);
        } else {
            eval_literal(exp, exp_idx, set);
        }

        goto predicate;

    case LYXP_TOKEN_NUMBER:
        /* Number */
        if (!set || (options & LYXP_SNODE_ALL)) {
            if (set) {
                set_snode_clear_ctx(set);
            }
            ret = eval_number(exp, exp_idx, NULL);
        } else {
            ret = eval_number(exp, exp_idx, set);
        }
        if (ret) {
            return ret;
        }

        goto predicate;

    default:
        LOGVAL(LYE_XPATH_INTOK, LY_VLOG_NONE, NULL,
               print_token(exp->tokens[*exp_idx]), &exp->expr[exp->expr_pos[*exp_idx]]);
        return -1;
    }

    return EXIT_SUCCESS;

predicate:
    /* Predicate* */
    while ((exp->used > *exp_idx) && (exp->tokens[*exp_idx] == LYXP_TOKEN_BRACK1)) {
        ret = eval_predicate(exp, exp_idx, cur_node, set, options);
        if (ret) {
            return ret;
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

        ret = eval_relative_location_path(exp, exp_idx, cur_node, all_desc, set, options);
        if (ret) {
            return ret;
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_unary_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
                int options)
{
    int unary_minus, ret;
    uint16_t op_exp;
    struct lyxp_set orig_set, set2;

    /* ('-')* */
    unary_minus = -1;
    while (!exp_check_token(exp, *exp_idx, LYXP_TOKEN_OPERATOR_MATH, 0)
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

    memset(&orig_set, 0, sizeof orig_set);
    memset(&set2, 0, sizeof set2);

    op_exp = exp_repeat_peek(exp, *exp_idx);
    if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_UNI)) {
        /* there is an operator */
        exp_repeat_pop(exp, *exp_idx);
        set_fill_set(&orig_set, set);
    } else {
        op_exp = 0;
    }

    /* PathExpr */
    ret = eval_path_expr(exp, exp_idx, cur_node, set, options);
    if (ret) {
        lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
        return ret;
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
            ret = eval_path_expr(exp, exp_idx, cur_node, NULL, options);
            if (ret) {
                return ret;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set);
        ret = eval_path_expr(exp, exp_idx, cur_node, &set2, options);
        if (ret) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, options);
            return ret;
        }

        /* eval */
        if (options & LYXP_SNODE_ALL) {
            set_snode_merge(set, &set2);
        } else if (moveto_union(set, &set2, cur_node, options)) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, options);
            return -1;
        }
    }

    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
    /* now we have all the unions in set and no other memory allocated */

    if (set && (unary_minus > -1) && !(options & LYXP_SNODE_ALL)) {
        if (moveto_op_math(set, NULL, &exp->expr[exp->expr_pos[unary_minus]], cur_node, options)) {
            return -1;
        }
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_multiplicative_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
                         int options)
{
    int ret;
    uint16_t this_op, op_exp;
    struct lyxp_set orig_set, set2;

    memset(&orig_set, 0, sizeof orig_set);
    memset(&set2, 0, sizeof set2);

    op_exp = exp_repeat_peek(exp, *exp_idx);
    if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_MATH)
            && ((exp->expr[exp->expr_pos[op_exp]] == '*') || (exp->tok_len[op_exp] == 3))) {
        /* there is an operator */
        exp_repeat_pop(exp, *exp_idx);
        set_fill_set(&orig_set, set);
    } else {
        op_exp = 0;
    }

    /* UnaryExpr */
    ret = eval_unary_expr(exp, exp_idx, cur_node, set, options);
    if (ret) {
        lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
        return ret;
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
            ret = eval_unary_expr(exp, exp_idx, cur_node, NULL, options);
            if (ret) {
                return ret;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set);
        ret = eval_unary_expr(exp, exp_idx, cur_node, &set2, options);
        if (ret) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, options);
            return ret;
        }

        /* eval */
        if (options & LYXP_SNODE_ALL) {
            set_snode_merge(set, &set2);
            set_snode_clear_ctx(set);
        } else {
            if (moveto_op_math(set, &set2, &exp->expr[exp->expr_pos[this_op]], cur_node, options)) {
                lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
                lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, options);
                return -1;
            }
        }
    }

    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_additive_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
                   int options)
{
    int ret;
    uint16_t this_op, op_exp;
    struct lyxp_set orig_set, set2;

    memset(&orig_set, 0, sizeof orig_set);
    memset(&set2, 0, sizeof set2);

    op_exp = exp_repeat_peek(exp, *exp_idx);
    if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_MATH)
            && ((exp->expr[exp->expr_pos[op_exp]] == '+') || (exp->expr[exp->expr_pos[op_exp]] == '-'))) {
        /* there is an operator */
        exp_repeat_pop(exp, *exp_idx);
        set_fill_set(&orig_set, set);
    } else {
        op_exp = 0;
    }

    /* MultiplicativeExpr */
    ret = eval_multiplicative_expr(exp, exp_idx, cur_node, set, options);
    if (ret) {
        lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
        return ret;
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
            ret = eval_multiplicative_expr(exp, exp_idx, cur_node, NULL, options);
            if (ret) {
                return ret;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set);
        ret = eval_multiplicative_expr(exp, exp_idx, cur_node, &set2, options);
        if (ret) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, options);
            return ret;
        }

        /* eval */
        if (options & LYXP_SNODE_ALL) {
            set_snode_merge(set, &set2);
            set_snode_clear_ctx(set);
        } else {
            if (moveto_op_math(set, &set2, &exp->expr[exp->expr_pos[this_op]], cur_node, options)) {
                lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
                lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, options);
                return -1;
            }
        }
    }

    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_relational_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
                     int options)
{
    int ret;
    uint16_t this_op, op_exp;
    struct lyxp_set orig_set, set2;

    memset(&orig_set, 0, sizeof orig_set);
    memset(&set2, 0, sizeof set2);

    op_exp = exp_repeat_peek(exp, *exp_idx);
    if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_COMP)
            && ((exp->expr[exp->expr_pos[op_exp]] == '<') || (exp->expr[exp->expr_pos[op_exp]] == '>'))) {
        /* there is an operator */
        exp_repeat_pop(exp, *exp_idx);
        set_fill_set(&orig_set, set);
    } else {
        op_exp = 0;
    }

    /* AdditiveExpr */
    ret = eval_additive_expr(exp, exp_idx, cur_node, set, options);
    if (ret) {
        lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
        return ret;
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
            ret = eval_additive_expr(exp, exp_idx, cur_node, NULL, options);
            if (ret) {
                return ret;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set);
        ret = eval_additive_expr(exp, exp_idx, cur_node, &set2, options);
        if (ret) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, options);
            return ret;
        }

        /* eval */
        if (options & LYXP_SNODE_ALL) {
            set_snode_merge(set, &set2);
            set_snode_clear_ctx(set);
        } else {
            if (moveto_op_comp(set, &set2, &exp->expr[exp->expr_pos[this_op]], cur_node, options)) {
                lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
                lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, options);
                return -1;
            }
        }
    }

    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_equality_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
                   int options)
{
    int ret;
    uint16_t this_op, op_exp;
    struct lyxp_set orig_set, set2;

    memset(&orig_set, 0, sizeof orig_set);
    memset(&set2, 0, sizeof set2);

    op_exp = exp_repeat_peek(exp, *exp_idx);
    if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_COMP)
            && ((exp->expr[exp->expr_pos[op_exp]] == '=') || (exp->expr[exp->expr_pos[op_exp]] == '!'))) {
        /* there is an operator */
        exp_repeat_pop(exp, *exp_idx);
        set_fill_set(&orig_set, set);
    } else {
        op_exp = 0;
    }

    /* RelationalExpr */
    ret = eval_relational_expr(exp, exp_idx, cur_node, set, options);
    if (ret) {
        lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
        return ret;
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
            ret = eval_relational_expr(exp, exp_idx, cur_node, NULL, options);
            if (ret) {
                return ret;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set);
        ret = eval_relational_expr(exp, exp_idx, cur_node, &set2, options);
        if (ret) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, options);
            return ret;
        }

        /* eval */
        if (options & LYXP_SNODE_ALL) {
            set_snode_merge(set, &set2);
            set_snode_clear_ctx(set);
        } else {
            if (moveto_op_comp(set, &set2, &exp->expr[exp->expr_pos[this_op]], cur_node, options)) {
                lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
                lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, options);
                return -1;
            }
        }
    }

    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_and_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set,
              int options)
{
    int ret;
    uint16_t op_exp;
    struct lyxp_set orig_set, set2;

    memset(&orig_set, 0, sizeof orig_set);
    memset(&set2, 0, sizeof set2);

    op_exp = exp_repeat_peek(exp, *exp_idx);
    if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_LOG) && (exp->tok_len[op_exp] == 3)) {
        /* there is an operator */
        exp_repeat_pop(exp, *exp_idx);
        set_fill_set(&orig_set, set);
    } else {
        op_exp = 0;
    }

    /* EqualityExpr */
    ret = eval_equality_expr(exp, exp_idx, cur_node, set, options);
    if (ret) {
        lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
        return ret;
    }

    /* cast to boolean, we know that will be the final result */
    if (op_exp) {
        if (set && (options & LYXP_SNODE_ALL)) {
            set_snode_clear_ctx(set);
        } else {
            lyxp_set_cast(set, LYXP_SET_BOOLEAN, cur_node, options);
        }
    }

    /* ('and' EqualityExpr)* */
    while (op_exp) {
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (!set || !set->val.bool ? "skipped" : "parsed"),
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
        if (!set || ((set->type == LYXP_SET_BOOLEAN) && !set->val.bool)) {
            ret = eval_equality_expr(exp, exp_idx, cur_node, NULL, options);
            if (ret) {
                return ret;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set);
        ret = eval_equality_expr(exp, exp_idx, cur_node, &set2, options);
        if (ret) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, options);
            return ret;
        }

        /* eval - just get boolean value actually */
        if (set->type == LYXP_SET_SNODE_SET) {
            set_snode_clear_ctx(&set2);
            set_snode_merge(set, &set2);
        } else {
            lyxp_set_cast(&set2, LYXP_SET_BOOLEAN, cur_node, options);
            set_fill_set(set, &set2);
        }
    }

    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
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
 * @param[in] options Whether to apply data node access restrictions defined for 'when' and 'must' evaluation.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on unresolved when, -1 on error.
 */
static int
eval_expr(struct lyxp_expr *exp, uint16_t *exp_idx, struct lyd_node *cur_node, struct lyxp_set *set, int options)
{
    int ret;
    uint16_t op_exp;
    struct lyxp_set orig_set, set2;

    memset(&orig_set, 0, sizeof orig_set);
    memset(&set2, 0, sizeof set2);

    op_exp = exp_repeat_peek(exp, *exp_idx);
    if (op_exp && (exp->tokens[op_exp] == LYXP_TOKEN_OPERATOR_LOG) && (exp->tok_len[op_exp] == 2)) {
        /* there is an operator */
        exp_repeat_pop(exp, *exp_idx);
        set_fill_set(&orig_set, set);
    } else {
        op_exp = 0;
    }

    /* AndExpr */
    ret = eval_and_expr(exp, exp_idx, cur_node, set, options);
    if (ret) {
        lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
        return ret;
    }

    /* cast to boolean, we know that will be the final result */
    if (op_exp) {
        if (set && (options & LYXP_SNODE_ALL)) {
            set_snode_clear_ctx(set);
        } else {
            lyxp_set_cast(set, LYXP_SET_BOOLEAN, cur_node, options);
        }
    }

    /* ('or' AndExpr)* */
    while (op_exp) {
        LOGDBG("XPATH: %-27s %s %s[%u]", __func__, (!set || set->val.bool ? "skipped" : "parsed"),
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
        if (!set || ((set->type == LYXP_SET_BOOLEAN) && set->val.bool)) {
            ret = eval_and_expr(exp, exp_idx, cur_node, NULL, options);
            if (ret) {
                return ret;
            }
            continue;
        }

        set_fill_set(&set2, &orig_set);
        ret = eval_and_expr(exp, exp_idx, cur_node, &set2, options);
        if (ret) {
            lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
            lyxp_set_cast(&set2, LYXP_SET_EMPTY, cur_node, options);
            return ret;
        }

        /* eval - just get boolean value actually */
        if (set->type == LYXP_SET_SNODE_SET) {
            set_snode_clear_ctx(&set2);
            set_snode_merge(set, &set2);
        } else {
            lyxp_set_cast(&set2, LYXP_SET_BOOLEAN, cur_node, options);
            set_fill_set(set, &set2);
        }
    }

    lyxp_set_cast(&orig_set, LYXP_SET_EMPTY, cur_node, options);
    return EXIT_SUCCESS;
}

int
lyxp_eval(const char *expr, const struct lyd_node *cur_node, enum lyxp_node_type cur_node_type, struct lyxp_set *set,
          int options)
{
    struct lyxp_expr *exp;
    uint16_t exp_idx = 0;
    int rc = -1;

    if (!expr || !set) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    exp = parse_expr(expr);
    if (!exp) {
        rc = -1;
        goto finish;
    }

    rc = reparse_expr(exp, &exp_idx);
    if (rc) {
        goto finish;
    } else if (exp->used > exp_idx) {
        LOGVAL(LYE_XPATH_INTOK, LY_VLOG_NONE, NULL, "Unknown", &exp->expr[exp->expr_pos[exp_idx]]);
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Unparsed characters \"%s\" left at the end of an XPath expression.",
               &exp->expr[exp->expr_pos[exp_idx]]);
        rc = -1;
        goto finish;
    }

    print_expr_struct_debug(exp);

    exp_idx = 0;
    memset(set, 0, sizeof *set);
    if (cur_node) {
        set_insert_node(set, (struct lyd_node *)cur_node, 0, cur_node_type, 0);
    }

    rc = eval_expr(exp, &exp_idx, (struct lyd_node *)cur_node, set, options);
    if ((rc == -1) && cur_node) {
        LOGPATH(LY_VLOG_LYD, cur_node);
    }

finish:
    exp_free(exp);
    return rc;
}

#if 0

/* full xml printing of set elements, not used currently */

void xml_print_node(struct lyout *out, int level, struct lyd_node *node, int toplevel);

void
lyxp_set_print_xml(FILE *f, struct lyxp_set *set)
{
    uint32_t i;
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
        } else if ((set->value.num == 0) || (set->value.num == -0.0f)) {
            str_num = strdup("0");
        } else if (isinf(set->value.num) && !signbit(set->value.num)) {
            str_num = strdup("Infinity");
        } else if (isinf(set->value.num) && signbit(set->value.num)) {
            str_num = strdup("-Infinity");
        } else if ((long long)set->value.num == set->value.num) {
            if (asprintf(&str_num, "%lld", (long long)set->value.num) == -1) {
                str_num = NULL;
            }
        } else {
            if (asprintf(&str_num, "%03.1Lf", set->value.num) == -1) {
                str_num = NULL;
            }
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
            case LYXP_NODE_ROOT_ALL:
                ly_print(&out, "ROOT all\n\n");
                break;
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
                ly_print(&out, "ROOT output \"%s\"\n\n", set->value.nodes[i]->schema->name);
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

#endif

int
lyxp_set_cast(struct lyxp_set *set, enum lyxp_set_type target, const struct lyd_node *cur_node, int options)
{
    long double num;
    char *str;

    if (!set || (set->type == target)) {
        return EXIT_SUCCESS;
    }

    /* it's not possible to convert anything into a node set */
    assert((target != LYXP_SET_NODE_SET) && ((set->type != LYXP_SET_SNODE_SET) || (target == LYXP_SET_EMPTY)));

    if (set->type == LYXP_SET_SNODE_SET) {
        free(set->val.snodes);
        return -1;
    }

    /* to STRING */
    if ((target == LYXP_SET_STRING) || ((target == LYXP_SET_NUMBER)
            && ((set->type == LYXP_SET_NODE_SET) || (set->type == LYXP_SET_EMPTY)))) {
        switch (set->type) {
        case LYXP_SET_NUMBER:
            if (isnan(set->val.num)) {
                set->val.str = strdup("NaN");
            } else if ((set->val.num == 0) || (set->val.num == -0.0f)) {
                set->val.str = strdup("0");
            } else if (isinf(set->val.num) && !signbit(set->val.num)) {
                set->val.str = strdup("Infinity");
            } else if (isinf(set->val.num) && signbit(set->val.num)) {
                set->val.str = strdup("-Infinity");
            } else if ((long long)set->val.num == set->val.num) {
                if (asprintf(&str, "%lld", (long long)set->val.num) == -1) {
                    LOGMEM;
                    return -1;
                }
                set->val.str = str;
            } else {
                if (asprintf(&str, "%03.1Lf", set->val.num) == -1) {
                    LOGMEM;
                    return -1;
                }
                set->val.str = str;
            }
            break;
        case LYXP_SET_BOOLEAN:
            if (set->val.bool) {
                set->val.str = strdup("true");
            } else {
                set->val.str = strdup("false");
            }
            break;
        case LYXP_SET_NODE_SET:
            assert(set->used);

#ifndef NDEBUG
            /* we need the set sorted, it affects the result */
            if (set_sort(set, cur_node, options) > 1) {
                LOGERR(LY_EINT, "XPath set was expected to be sorted, but is not (%s).", __func__);
            }
#endif

            str = cast_node_set_to_string(set, (struct lyd_node *)cur_node, options);
            if (!str) {
                return -1;
            }
            free(set->val.nodes);
            set->val.str = str;
            break;
        case LYXP_SET_EMPTY:
            set->val.str = strdup("");
            if (!set->val.str) {
                LOGMEM;
                return -1;
            }
            break;
        default:
            LOGINT;
            return -1;
        }
        set->type = LYXP_SET_STRING;
    }

    /* to NUMBER */
    if (target == LYXP_SET_NUMBER) {
        switch (set->type) {
        case LYXP_SET_STRING:
            num = cast_string_to_number(set->val.str);
            free(set->val.str);
            set->val.num = num;
            break;
        case LYXP_SET_BOOLEAN:
            if (set->val.bool) {
                set->val.num = 1;
            } else {
                set->val.num = 0;
            }
            break;
        default:
            LOGINT;
            return -1;
        }
        set->type = LYXP_SET_NUMBER;
    }

    /* to BOOLEAN */
    if (target == LYXP_SET_BOOLEAN) {
        switch (set->type) {
        case LYXP_SET_NUMBER:
            if ((set->val.num == 0) || (set->val.num == -0.0f) || isnan(set->val.num)) {
                set->val.bool = 0;
            } else {
                set->val.bool = 1;
            }
            break;
        case LYXP_SET_STRING:
            if (set->val.str[0]) {
                free(set->val.str);
                set->val.bool = 1;
            } else {
                free(set->val.str);
                set->val.bool = 0;
            }
            break;
        case LYXP_SET_NODE_SET:
            free(set->val.nodes);

            assert(set->used);
            set->val.bool = 1;
            break;
        case LYXP_SET_EMPTY:
            set->val.bool = 0;
            break;
        default:
            LOGINT;
            return -1;
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
            free(set->val.str);
            break;
        case LYXP_SET_NODE_SET:
            free(set->val.nodes);
            break;
        default:
            LOGINT;
            return -1;
        }
        set->type = LYXP_SET_EMPTY;
    }

    return EXIT_SUCCESS;
}

void
lyxp_set_free(struct lyxp_set *set)
{
    if (!set) {
        return;
    }

    if (set->type == LYXP_SET_NODE_SET) {
        free(set->val.nodes);
    } else if (set->type == LYXP_SET_SNODE_SET) {
        free(set->val.snodes);
    } else if (set->type == LYXP_SET_STRING) {
        free(set->val.str);
    }
    free(set);
}

int
lyxp_atomize(const char *expr, const struct lys_node *cur_snode, enum lyxp_node_type cur_snode_type,
             struct lyxp_set *set, int options)
{
    struct lyxp_expr *exp;
    uint16_t exp_idx = 0;
    int rc = -1;

    exp = parse_expr(expr);
    if (!exp) {
        rc = -1;
        goto finish;
    }

    rc = reparse_expr(exp, &exp_idx);
    if (rc) {
        goto finish;
    } else if (exp->used > exp_idx) {
        LOGVAL(LYE_XPATH_INTOK, LY_VLOG_NONE, NULL, "Unknown", &exp->expr[exp->expr_pos[exp_idx]]);
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Unparsed characters \"%s\" left at the end of an XPath expression.",
               &exp->expr[exp->expr_pos[exp_idx]]);
        rc = -1;
        goto finish;
    }

    print_expr_struct_debug(exp);

    exp_idx = 0;
    memset(set, 0, sizeof *set);
    set->type = LYXP_SET_SNODE_SET;
    set_snode_insert_node(set, cur_snode, cur_snode_type);

    rc = eval_expr(exp, &exp_idx, (struct lyd_node *)cur_snode, set, options);
    if (rc == -1) {
        LOGPATH(LY_VLOG_LYS, cur_snode);
    }

finish:
    exp_free(exp);
    return rc;
}

int
lyxp_node_atomize(const struct lys_node *node, struct lyxp_set *set)
{
    struct lys_node *ctx_snode;
    enum lyxp_node_type ctx_snode_type;
    struct lyxp_set tmp_set;
    uint8_t must_size = 0;
    uint32_t i;
    int opts;
    struct lys_when *when = NULL;
    struct lys_restr *must = NULL;

    memset(&tmp_set, 0, sizeof tmp_set);
    memset(set, 0, sizeof *set);

    /* check if we will be traversing RPC output */
    opts = 0;
    for (ctx_snode = lys_parent(node); ctx_snode && (ctx_snode->nodetype != LYS_OUTPUT); ctx_snode = lys_parent(ctx_snode));
    if (ctx_snode) {
        opts |= LYXP_SNODE_OUTPUT;
    }

    switch (node->nodetype) {
    case LYS_CONTAINER:
        when = ((struct lys_node_container *)node)->when;
        must = ((struct lys_node_container *)node)->must;
        must_size = ((struct lys_node_container *)node)->must_size;
        break;
    case LYS_CHOICE:
        when = ((struct lys_node_choice *)node)->when;
        break;
    case LYS_LEAF:
        when = ((struct lys_node_leaf *)node)->when;
        must = ((struct lys_node_leaf *)node)->must;
        must_size = ((struct lys_node_leaf *)node)->must_size;
        break;
    case LYS_LEAFLIST:
        when = ((struct lys_node_leaflist *)node)->when;
        must = ((struct lys_node_leaflist *)node)->must;
        must_size = ((struct lys_node_leaflist *)node)->must_size;
        break;
    case LYS_LIST:
        when = ((struct lys_node_list *)node)->when;
        must = ((struct lys_node_list *)node)->must;
        must_size = ((struct lys_node_list *)node)->must_size;
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        when = ((struct lys_node_anydata *)node)->when;
        must = ((struct lys_node_anydata *)node)->must;
        must_size = ((struct lys_node_anydata *)node)->must_size;
        break;
    case LYS_CASE:
        when = ((struct lys_node_case *)node)->when;
        break;
    case LYS_NOTIF:
        must = ((struct lys_node_notif *)node)->must;
        must_size = ((struct lys_node_notif *)node)->must_size;
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        must = ((struct lys_node_inout *)node)->must;
        must_size = ((struct lys_node_inout *)node)->must_size;
        break;
    case LYS_USES:
        when = ((struct lys_node_uses *)node)->when;
        break;
    case LYS_AUGMENT:
        when = ((struct lys_node_augment *)node)->when;
        break;
    default:
        LOGINT;
        return -1;
    }

    /* check "when" */
    if (when) {
        resolve_when_ctx_snode(node, &ctx_snode, &ctx_snode_type);
        if (lyxp_atomize(when->cond, ctx_snode, ctx_snode_type, &tmp_set, LYXP_SNODE_WHEN | opts)) {
            free(tmp_set.val.snodes);
            if ((ly_errno == LY_EVALID) && (ly_vecode == LYVE_XPATH_INSNODE)) {
                return EXIT_FAILURE;
            } else {
                return -1;
            }
        }

        set_snode_merge(set, &tmp_set);
        memset(&tmp_set, 0, sizeof tmp_set);
    }

    /* check "must" */
    for (i = 0; i < must_size; ++i) {
        if (lyxp_atomize(must[i].expr, node, LYXP_NODE_ELEM, &tmp_set, LYXP_SNODE_MUST | opts)) {
            free(tmp_set.val.snodes);
            free(set->val.snodes);
            if ((ly_errno == LY_EVALID) && (ly_vecode == LYVE_XPATH_INSNODE)) {
                return EXIT_FAILURE;
            } else {
                return -1;
            }
        }

        set_snode_merge(set, &tmp_set);
        memset(&tmp_set, 0, sizeof tmp_set);
    }

    return EXIT_SUCCESS;
}
