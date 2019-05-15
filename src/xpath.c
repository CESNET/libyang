/**
 * @file xpath.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief YANG XPath evaluation functions
 *
 * Copyright (c) 2015 - 2017 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "common.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xpath.h"
#include "dict.h"
#include "xml.h"

/**
 * @brief Parse NCName.
 *
 * @param[in] ncname Name to parse.
 *
 * @return Length of \p ncname valid bytes.
 */
static long int
parse_ncname(const char *ncname)
{
    unsigned int uc;
    size_t size;
    long int len = 0;

    LY_CHECK_RET(ly_getutf8(&ncname, &uc, &size), 0);
    if (!is_xmlqnamestartchar(uc) || (uc == ':')) {
        return len;
    }

    do {
        len += size;
        if (!*ncname) {
            break;
        }
        LY_CHECK_RET(ly_getutf8(&ncname, &uc, &size), -len);
    } while (is_xmlqnamechar(uc) && (uc != ':'));

    return len;
}

/**
 * @brief Add \p token into the expression \p exp.
 *
 * @param[in] ctx libyang context to log in.
 * @param[in] exp Expression to use.
 * @param[in] token Token to add.
 * @param[in] expr_pos Token position in the XPath expression.
 * @param[in] tok_len Token length in the XPath expression.
 * @return LY_ERR value
 */
static LY_ERR
exp_add_token(struct ly_ctx *ctx, struct lyxp_expr *exp, enum lyxp_token token, uint16_t expr_pos, uint16_t tok_len)
{
    uint32_t prev;

    if (exp->used == exp->size) {
        prev = exp->size;
        exp->size += LYXP_EXPR_SIZE_STEP;
        if (prev > exp->size) {
            LOGINT(ctx);
            return LY_EINT;
        }

        exp->tokens = ly_realloc(exp->tokens, exp->size * sizeof *exp->tokens);
        LY_CHECK_ERR_RET(!exp->tokens, LOGMEM(ctx), LY_EMEM);
        exp->tok_pos = ly_realloc(exp->tok_pos, exp->size * sizeof *exp->tok_pos);
        LY_CHECK_ERR_RET(!exp->tok_pos, LOGMEM(ctx), LY_EMEM);
        exp->tok_len = ly_realloc(exp->tok_len, exp->size * sizeof *exp->tok_len);
        LY_CHECK_ERR_RET(!exp->tok_len, LOGMEM(ctx), LY_EMEM);
    }

    exp->tokens[exp->used] = token;
    exp->tok_pos[exp->used] = expr_pos;
    exp->tok_len[exp->used] = tok_len;
    ++exp->used;
    return LY_SUCCESS;
}

void
lyxp_expr_free(struct ly_ctx *ctx, struct lyxp_expr *expr)
{
    uint16_t i;

    if (!expr) {
        return;
    }

    lydict_remove(ctx, expr->expr);
    free(expr->tokens);
    free(expr->tok_pos);
    free(expr->tok_len);
    if (expr->repeat) {
        for (i = 0; i < expr->used; ++i) {
            free(expr->repeat[i]);
        }
    }
    free(expr->repeat);
    free(expr);
}

struct lyxp_expr *
lyxp_expr_parse(struct ly_ctx *ctx, const char *expr)
{
    struct lyxp_expr *ret;
    size_t parsed = 0, tok_len;
    long int ncname_len;
    enum lyxp_token tok_type;
    int prev_function_check = 0;

    if (strlen(expr) > UINT16_MAX) {
        LOGERR(ctx, LY_EINVAL, "XPath expression cannot be longer than %ud characters.", UINT16_MAX);
        return NULL;
    }

    /* init lyxp_expr structure */
    ret = calloc(1, sizeof *ret);
    LY_CHECK_ERR_GOTO(!ret, LOGMEM(ctx), error);
    ret->expr = lydict_insert(ctx, expr, strlen(expr));
    LY_CHECK_ERR_GOTO(!ret->expr, LOGMEM(ctx), error);
    ret->used = 0;
    ret->size = LYXP_EXPR_SIZE_START;
    ret->tokens = malloc(ret->size * sizeof *ret->tokens);
    LY_CHECK_ERR_GOTO(!ret->tokens, LOGMEM(ctx), error);

    ret->tok_pos = malloc(ret->size * sizeof *ret->tok_pos);
    LY_CHECK_ERR_GOTO(!ret->tok_pos, LOGMEM(ctx), error);

    ret->tok_len = malloc(ret->size * sizeof *ret->tok_len);
    LY_CHECK_ERR_GOTO(!ret->tok_len, LOGMEM(ctx), error);

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
                        && (!strncmp(&expr[ret->tok_pos[ret->used - 1]], "node", 4)
                        || !strncmp(&expr[ret->tok_pos[ret->used - 1]], "text", 4))) ||
                        ((ret->tok_len[ret->used - 1] == 7)
                        && !strncmp(&expr[ret->tok_pos[ret->used - 1]], "comment", 7))) {
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

        } else if ((expr[parsed] == '.') && (!isdigit(expr[parsed + 1]))) {

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
            for (tok_len = 1; (expr[parsed + tok_len] != '\0') && (expr[parsed + tok_len] != '\''); ++tok_len);
            LY_CHECK_ERR_GOTO(expr[parsed + tok_len] == '\0',
                              LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_XP_EOE, expr[parsed], &expr[parsed]), error);
            ++tok_len;
            tok_type = LYXP_TOKEN_LITERAL;

        } else if (expr[parsed] == '\"') {

            /* Literal with " */
            for (tok_len = 1; (expr[parsed + tok_len] != '\0') && (expr[parsed + tok_len] != '\"'); ++tok_len);
            LY_CHECK_ERR_GOTO(expr[parsed + tok_len] == '\0',
                              LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_XP_EOE, expr[parsed], &expr[parsed]), error);
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

            } else if (prev_function_check) {
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LYVE_XPATH, "Invalid character 0x%x, perhaps \"%.*s\" is supposed to be a function call.",
                       expr[parsed], &expr[parsed], ret->tok_len[ret->used - 1], &ret->expr[ret->tok_pos[ret->used - 1]]);
                goto error;
            } else {
                LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_XP_INEXPR, parsed + 1, expr);
                goto error;
            }
        } else if (expr[parsed] == '*') {

            /* NameTest '*' */
            tok_len = 1;
            tok_type = LYXP_TOKEN_NAMETEST;

        } else {

            /* NameTest (NCName ':' '*' | QName) or NodeType/FunctionName */
            ncname_len = parse_ncname(&expr[parsed]);
            LY_CHECK_ERR_GOTO(ncname_len < 0, LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_XP_INEXPR, parsed - ncname_len + 1, expr), error);
            tok_len = ncname_len;

            if (expr[parsed + tok_len] == ':') {
                ++tok_len;
                if (expr[parsed + tok_len] == '*') {
                    ++tok_len;
                } else {
                    ncname_len = parse_ncname(&expr[parsed + tok_len]);
                    LY_CHECK_ERR_GOTO(ncname_len < 0, LOGVAL(ctx, LY_VLOG_NONE, NULL, LY_VCODE_XP_INEXPR, parsed - ncname_len + 1, expr), error);
                    tok_len += ncname_len;
                }
                /* remove old flag to prevent ambiguities */
                prev_function_check = 0;
                tok_type = LYXP_TOKEN_NAMETEST;
            } else {
                /* there is no prefix so it can still be NodeType/FunctionName, we can't finally decide now */
                prev_function_check = 1;
                tok_type = LYXP_TOKEN_NAMETEST;
            }
        }

        /* store the token, move on to the next one */
        LY_CHECK_GOTO(exp_add_token(ctx, ret, tok_type, parsed, tok_len), error);
        parsed += tok_len;
        while (is_xmlws(expr[parsed])) {
            ++parsed;
        }

    } while (expr[parsed]);

    /* prealloc repeat */
    ret->repeat = calloc(ret->size, sizeof *ret->repeat);
    LY_CHECK_ERR_GOTO(!ret->repeat, LOGMEM(ctx), error);

    return ret;

error:
    lyxp_expr_free(ctx, ret);
    return NULL;
}

