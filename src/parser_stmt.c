/**
 * @file parser_stmt.c
 * @author Radek Krejčí <rkrejci@cesnet.cz>
 * @brief Parser of the extension substatements.
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "dict.h"
#include "in.h"
#include "in_internal.h"
#include "log.h"
#include "parser_schema.h"
#include "path.h"
#include "schema_compile.h"
#include "tree.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

static LY_ERR
lysp_stmt_validate_value(struct lys_parser_ctx *ctx, enum yang_arg val_type, const char *val)
{
    uint8_t prefix = 0;
    ly_bool first = 1;
    uint32_t c;
    size_t utf8_char_len;

    while (*val) {
        LY_CHECK_ERR_RET(ly_getutf8(&val, &c, &utf8_char_len),
                LOGVAL_PARSER(ctx, LY_VCODE_INCHAR, (val)[-utf8_char_len]), LY_EVALID);

        switch (val_type) {
        case Y_IDENTIF_ARG:
            LY_CHECK_RET(lysp_check_identifierchar(ctx, c, first, NULL));
            break;
        case Y_PREF_IDENTIF_ARG:
            LY_CHECK_RET(lysp_check_identifierchar(ctx, c, first, &prefix));
            break;
        case Y_STR_ARG:
        case Y_MAYBE_STR_ARG:
            LY_CHECK_RET(lysp_check_stringchar(ctx, c));
            break;
        }
        first = 0;
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse extension instance.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] ext_name Extension instance substatement name (keyword).
 * @param[in] ext_name_len Extension instance substatement name length.
 * @param[in] insubstmt Type of the keyword this extension instance is a substatement of.
 * @param[in] insubstmt_index Index of the keyword instance this extension instance is a substatement of.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_ext(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, LYEXT_SUBSTMT insubstmt,
        LY_ARRAY_COUNT_TYPE insubstmt_index, struct lysp_ext_instance **exts)
{
    struct lysp_ext_instance *e;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *exts, e, LY_EMEM);

    /* store name and insubstmt info */
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->stmt, 0, &e->name));
    e->insubstmt = insubstmt;
    e->insubstmt_index = insubstmt_index;
    /* TODO (duplicate) e->child = stmt->child; */

    /* get optional argument */
    if (stmt->arg) {
        LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &e->argument));
    }

    return LY_SUCCESS;
}

/**
 * @brief Parse a generic text field without specific constraints. Those are contact, organization,
 * description, etc...
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in] stmt Statement structure.
 * @param[in] substmt Type of this substatement.
 * @param[in] substmt_index Index of this substatement.
 * @param[in,out] value Place to store the parsed value.
 * @param[in] arg Type of the YANG keyword argument (of the value).
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_text_field(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, LYEXT_SUBSTMT substmt, uint32_t substmt_index,
        const char **value, enum yang_arg arg, struct lysp_ext_instance **exts)
{
    const struct lysp_stmt *child;

    if (*value) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, lyext_substmt2str(substmt));
        return LY_EVALID;
    }

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, arg, stmt->arg));
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, value));

    for (child = stmt->child; child; child = child->next) {
        struct ly_in *in;
        LY_CHECK_RET(ly_in_new_memory(child->stmt, &in));
        enum ly_stmt kw = lysp_match_kw(in, NULL);
        ly_in_free(in, 0);

        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, substmt, substmt_index, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), lyext_substmt2str(substmt));
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse a qname that can have more instances such as if-feature.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] substmt Type of this substatement.
 * @param[in,out] qnames Parsed qnames to add to.
 * @param[in] arg Type of the expected argument.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_qnames(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, LYEXT_SUBSTMT substmt,
        struct lysp_qname **qnames, enum yang_arg arg, struct lysp_ext_instance **exts)
{
    struct lysp_qname *item;
    const struct lysp_stmt *child;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, arg, stmt->arg));

    /* allocate new pointer */
    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *qnames, item, LY_EMEM);
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &item->str));
    item->mod = ctx->parsed_mod;

    for (child = stmt->child; child; child = child->next) {
        struct ly_in *in;
        LY_CHECK_RET(ly_in_new_memory(child->stmt, &in));
        enum ly_stmt kw = lysp_match_kw(in, NULL);
        ly_in_free(in, 0);

        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, substmt, LY_ARRAY_COUNT(*qnames) - 1, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), lyext_substmt2str(substmt));
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse a generic text field that can have more instances such as base.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] substmt Type of this substatement.
 * @param[in,out] texts Parsed values to add to.
 * @param[in] arg Type of the expected argument.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_text_fields(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, LYEXT_SUBSTMT substmt,
        const char ***texts, enum yang_arg arg, struct lysp_ext_instance **exts)
{
    const char **item;
    const struct lysp_stmt *child;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, arg, stmt->arg));

    /* allocate new pointer */
    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *texts, item, LY_EMEM);
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, item));

    for (child = stmt->child; child; child = child->next) {
        struct ly_in *in;
        LY_CHECK_RET(ly_in_new_memory(child->stmt, &in));
        enum ly_stmt kw = lysp_match_kw(in, NULL);
        ly_in_free(in, 0);

        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, substmt, LY_ARRAY_COUNT(*texts) - 1, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), lyext_substmt2str(substmt));
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the status statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] flags Flags to add to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_status(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, uint16_t *flags, struct lysp_ext_instance **exts)
{
    size_t arg_len;
    const struct lysp_stmt *child;

    if (*flags & LYS_STATUS_MASK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "status");
        return LY_EVALID;
    }

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    arg_len = strlen(stmt->arg);
    if ((arg_len == ly_strlen_const("current")) && !strncmp(stmt->arg, "current", arg_len)) {
        *flags |= LYS_STATUS_CURR;
    } else if ((arg_len == ly_strlen_const("deprecated")) && !strncmp(stmt->arg, "deprecated", arg_len)) {
        *flags |= LYS_STATUS_DEPRC;
    } else if ((arg_len == ly_strlen_const("obsolete")) && !strncmp(stmt->arg, "obsolete", arg_len)) {
        *flags |= LYS_STATUS_OBSLT;
    } else {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "status");
        return LY_EVALID;
    }

    for (child = stmt->child; child; child = child->next) {
        struct ly_in *in;
        LY_CHECK_RET(ly_in_new_memory(child->stmt, &in));
        enum ly_stmt kw = lysp_match_kw(in, NULL);
        ly_in_free(in, 0);

        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LYEXT_SUBSTMT_STATUS, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "status");
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse a restriction such as range or length.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] restr_kw Type of this particular restriction.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_restr(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, enum ly_stmt restr_kw, struct lysp_restr *restr)
{
    const struct lysp_stmt *child;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &restr->arg.str));
    restr->arg.mod = ctx->parsed_mod;

    for (child = stmt->child; child; child = child->next) {
        struct ly_in *in;
        LY_CHECK_RET(ly_in_new_memory(child->stmt, &in));
        enum ly_stmt kw = lysp_match_kw(in, NULL);
        ly_in_free(in, 0);

        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, LYEXT_SUBSTMT_DESCRIPTION, 0, &restr->dsc, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, LYEXT_SUBSTMT_REFERENCE, 0, &restr->ref, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_ERROR_APP_TAG:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, LYEXT_SUBSTMT_ERRTAG, 0, &restr->eapptag, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_ERROR_MESSAGE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, LYEXT_SUBSTMT_ERRMSG, 0, &restr->emsg, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LYEXT_SUBSTMT_SELF, 0, &restr->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), ly_stmt2str(restr_kw));
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse a restriction that can have more instances such as must.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] restr_kw Type of this particular restriction.
 * @param[in,out] restrs Restrictions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_restrs(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, enum ly_stmt restr_kw, struct lysp_restr **restrs)
{
    struct lysp_restr *restr;

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *restrs, restr, LY_EMEM);
    return lysp_stmt_restr(ctx, stmt, restr_kw, restr);
}

/**
 * @brief Parse the value or position statement. Substatement of type enum statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] val_kw Type of this particular keyword.
 * @param[in,out] value Value to write to.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_type_enum_value_pos(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, enum ly_stmt val_kw, int64_t *value, uint16_t *flags,
        struct lysp_ext_instance **exts)
{
    size_t arg_len;
    char *ptr = NULL;
    long int num = 0;
    unsigned long int unum = 0;
    struct lysp_stmt *child;

    if (*flags & LYS_SET_VALUE) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(val_kw));
        return LY_EVALID;
    }
    *flags |= LYS_SET_VALUE;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));

    arg_len = strlen(stmt->arg);
    if (!arg_len || (stmt->arg[0] == '+') || ((stmt->arg[0] == '0') && (arg_len > 1)) || ((val_kw == LY_STMT_POSITION) && !strncmp(stmt->arg, "-0", 2))) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, ly_stmt2str(val_kw));
        goto error;
    }

    errno = 0;
    if (val_kw == LY_STMT_VALUE) {
        num = strtol(stmt->arg, &ptr, LY_BASE_DEC);
        if ((num < INT64_C(-2147483648)) || (num > INT64_C(2147483647))) {
            LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, ly_stmt2str(val_kw));
            goto error;
        }
    } else {
        unum = strtoul(stmt->arg, &ptr, LY_BASE_DEC);
        if (unum > UINT64_C(4294967295)) {
            LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, ly_stmt2str(val_kw));
            goto error;
        }
    }
    /* we have not parsed the whole argument */
    if ((size_t)(ptr - stmt->arg) != arg_len) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, ly_stmt2str(val_kw));
        goto error;
    }
    if (errno == ERANGE) {
        LOGVAL_PARSER(ctx, LY_VCODE_OOB, arg_len, stmt->arg, ly_stmt2str(val_kw));
        goto error;
    }
    if (val_kw == LY_STMT_VALUE) {
        *value = num;
    } else {
        *value = unum;
    }

    for (child = stmt->child; child; child = child->next) {
        struct ly_in *in;
        LY_CHECK_RET(ly_in_new_memory(child->stmt, &in));
        enum ly_stmt kw = lysp_match_kw(in, NULL);
        ly_in_free(in, 0);

        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, val_kw == LY_STMT_VALUE ? LYEXT_SUBSTMT_VALUE : LYEXT_SUBSTMT_POSITION, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), ly_stmt2str(val_kw));
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;

error:
    return LY_EVALID;
}

/**
 * @brief Parse the enum or bit statement. Substatement of type statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in] enum_kw Type of this particular keyword.
 * @param[in,out] enums Enums or bits to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_type_enum(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, enum ly_stmt enum_kw, struct lysp_type_enum **enums)
{
    struct lysp_type_enum *enm;
    const struct lysp_stmt *child;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, enum_kw == LY_STMT_ENUM ? Y_STR_ARG : Y_IDENTIF_ARG, stmt->arg));

    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *enums, enm, LY_EMEM);

    if (enum_kw == LY_STMT_ENUM) {
        LY_CHECK_RET(lysp_check_enum_name(ctx, stmt->arg, strlen(stmt->arg)));
    } /* else nothing specific for YANG_BIT */

    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &enm->name));
    CHECK_UNIQUENESS(ctx, *enums, name, ly_stmt2str(enum_kw), enm->name);

    for (child = stmt->child; child; child = child->next) {
        struct ly_in *in;
        LY_CHECK_RET(ly_in_new_memory(child->stmt, &in));
        enum ly_stmt kw = lysp_match_kw(in, NULL);
        ly_in_free(in, 0);

        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, LYEXT_SUBSTMT_DESCRIPTION, 0, &enm->dsc, Y_STR_ARG, &enm->exts));
            break;
        case LY_STMT_IF_FEATURE:
            PARSER_CHECK_STMTVER2_RET(ctx, "if-feature", ly_stmt2str(enum_kw));
            LY_CHECK_RET(lysp_stmt_qnames(ctx, child, LYEXT_SUBSTMT_IFFEATURE, &enm->iffeatures, Y_STR_ARG, &enm->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, LYEXT_SUBSTMT_REFERENCE, 0, &enm->ref, Y_STR_ARG, &enm->exts));
            break;
        case LY_STMT_STATUS:
            LY_CHECK_RET(lysp_stmt_status(ctx, child, &enm->flags, &enm->exts));
            break;
        case LY_STMT_VALUE:
            LY_CHECK_ERR_RET(enum_kw == LY_STMT_BIT, LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw),
                    ly_stmt2str(enum_kw)), LY_EVALID);
            LY_CHECK_RET(lysp_stmt_type_enum_value_pos(ctx, child, kw, &enm->value, &enm->flags, &enm->exts));
            break;
        case LY_STMT_POSITION:
            LY_CHECK_ERR_RET(enum_kw == LY_STMT_ENUM, LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw),
                    ly_stmt2str(enum_kw)), LY_EVALID);
            LY_CHECK_RET(lysp_stmt_type_enum_value_pos(ctx, child, kw, &enm->value, &enm->flags, &enm->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LYEXT_SUBSTMT_SELF, 0, &enm->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), ly_stmt2str(enum_kw));
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the fraction-digits statement. Substatement of type statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] fracdig Value to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_type_fracdigits(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, uint8_t *fracdig, struct lysp_ext_instance **exts)
{
    char *ptr;
    size_t arg_len;
    unsigned long int num;
    const struct lysp_stmt *child;

    if (*fracdig) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "fraction-digits");
        return LY_EVALID;
    }

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    arg_len = strlen(stmt->arg);
    if (!arg_len || (stmt->arg[0] == '0') || !isdigit(stmt->arg[0])) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "fraction-digits");
        return LY_EVALID;
    }

    errno = 0;
    num = strtoul(stmt->arg, &ptr, LY_BASE_DEC);
    /* we have not parsed the whole argument */
    if ((size_t)(ptr - stmt->arg) != arg_len) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "fraction-digits");
        return LY_EVALID;
    }
    if ((errno == ERANGE) || (num > LY_TYPE_DEC64_FD_MAX)) {
        LOGVAL_PARSER(ctx, LY_VCODE_OOB, arg_len, stmt->arg, "fraction-digits");
        return LY_EVALID;
    }
    *fracdig = num;

    for (child = stmt->child; child; child = child->next) {
        struct ly_in *in;
        LY_CHECK_RET(ly_in_new_memory(child->stmt, &in));
        enum ly_stmt kw = lysp_match_kw(in, NULL);
        ly_in_free(in, 0);

        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LYEXT_SUBSTMT_FRACDIGITS, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "fraction-digits");
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the require-instance statement. Substatement of type statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] reqinst Value to write to.
 * @param[in,out] flags Flags to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_type_reqinstance(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, uint8_t *reqinst, uint16_t *flags,
        struct lysp_ext_instance **exts)
{
    size_t arg_len;
    struct lysp_stmt *child;

    if (*flags & LYS_SET_REQINST) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "require-instance");
        return LY_EVALID;
    }
    *flags |= LYS_SET_REQINST;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    arg_len = strlen(stmt->arg);
    if ((arg_len == ly_strlen_const("true")) && !strncmp(stmt->arg, "true", arg_len)) {
        *reqinst = 1;
    } else if ((arg_len != ly_strlen_const("false")) || strncmp(stmt->arg, "false", arg_len)) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "require-instance");
        return LY_EVALID;
    }

    for (child = stmt->child; child; child = child->next) {
        struct ly_in *in;
        LY_CHECK_RET(ly_in_new_memory(child->stmt, &in));
        enum ly_stmt kw = lysp_match_kw(in, NULL);
        ly_in_free(in, 0);

        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LYEXT_SUBSTMT_REQINSTANCE, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "require-instance");
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the modifier statement. Substatement of type pattern statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] pat Value to write to.
 * @param[in,out] exts Extension instances to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_type_pattern_modifier(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, const char **pat, struct lysp_ext_instance **exts)
{
    size_t arg_len;
    char *buf;
    const struct lysp_stmt *child;

    if ((*pat)[0] == LYSP_RESTR_PATTERN_NACK) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "modifier");
        return LY_EVALID;
    }

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    arg_len = strlen(stmt->arg);
    if ((arg_len != ly_strlen_const("invert-match")) || strncmp(stmt->arg, "invert-match", arg_len)) {
        LOGVAL_PARSER(ctx, LY_VCODE_INVAL, arg_len, stmt->arg, "modifier");
        return LY_EVALID;
    }

    /* replace the value in the dictionary */
    buf = malloc(strlen(*pat) + 1);
    LY_CHECK_ERR_RET(!buf, LOGMEM(PARSER_CTX(ctx)), LY_EMEM);
    strcpy(buf, *pat);
    lydict_remove(PARSER_CTX(ctx), *pat);

    assert(buf[0] == LYSP_RESTR_PATTERN_ACK);
    buf[0] = LYSP_RESTR_PATTERN_NACK;
    LY_CHECK_RET(lydict_insert_zc(PARSER_CTX(ctx), buf, pat));

    for (child = stmt->child; child; child = child->next) {
        struct ly_in *in;
        LY_CHECK_RET(ly_in_new_memory(child->stmt, &in));
        enum ly_stmt kw = lysp_match_kw(in, NULL);
        ly_in_free(in, 0);

        switch (kw) {
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LYEXT_SUBSTMT_MODIFIER, 0, exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "modifier");
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the pattern statement. Substatement of type statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] patterns Restrictions to add to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_type_pattern(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_restr **patterns)
{
    char *buf;
    size_t arg_len;
    const struct lysp_stmt *child;
    struct lysp_restr *restr;

    LY_CHECK_RET(lysp_stmt_validate_value(ctx, Y_STR_ARG, stmt->arg));
    LY_ARRAY_NEW_RET(PARSER_CTX(ctx), *patterns, restr, LY_EMEM);
    arg_len = strlen(stmt->arg);

    /* add special meaning first byte */
    buf = malloc(arg_len + 2);
    LY_CHECK_ERR_RET(!buf, LOGMEM(PARSER_CTX(ctx)), LY_EMEM);
    memmove(buf + 1, stmt->arg, arg_len);
    buf[0] = LYSP_RESTR_PATTERN_ACK; /* pattern's default regular-match flag */
    buf[arg_len + 1] = '\0'; /* terminating NULL byte */
    LY_CHECK_RET(lydict_insert_zc(PARSER_CTX(ctx), buf, &restr->arg.str));
    restr->arg.mod = ctx->parsed_mod;

    for (child = stmt->child; child; child = child->next) {
        struct ly_in *in;
        LY_CHECK_RET(ly_in_new_memory(child->stmt, &in));
        enum ly_stmt kw = lysp_match_kw(in, NULL);
        ly_in_free(in, 0);

        switch (kw) {
        case LY_STMT_DESCRIPTION:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, LYEXT_SUBSTMT_DESCRIPTION, 0, &restr->dsc, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_REFERENCE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, LYEXT_SUBSTMT_REFERENCE, 0, &restr->ref, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_ERROR_APP_TAG:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, LYEXT_SUBSTMT_ERRTAG, 0, &restr->eapptag, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_ERROR_MESSAGE:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, LYEXT_SUBSTMT_ERRMSG, 0, &restr->emsg, Y_STR_ARG, &restr->exts));
            break;
        case LY_STMT_MODIFIER:
            PARSER_CHECK_STMTVER2_RET(ctx, "modifier", "pattern");
            LY_CHECK_RET(lysp_stmt_type_pattern_modifier(ctx, child, &restr->arg.str, &restr->exts));
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LYEXT_SUBSTMT_SELF, 0, &restr->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "pattern");
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

/**
 * @brief Parse the type statement.
 *
 * @param[in] ctx yang parser context for logging.
 * @param[in,out] data Data to read from, always moved to currently handled character.
 * @param[in,out] type Type to wrote to.
 *
 * @return LY_ERR values.
 */
static LY_ERR
lysp_stmt_type(struct lys_parser_ctx *ctx, const struct lysp_stmt *stmt, struct lysp_type *type)
{
    struct lysp_type *nest_type;
    const struct lysp_stmt *child;
    const char *str_path = NULL;
    LY_ERR ret;

    if (type->name) {
        LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, "type");
        return LY_EVALID;
    }
    LY_CHECK_RET(lydict_insert(PARSER_CTX(ctx), stmt->arg, 0, &type->name));
    type->pmod = ctx->parsed_mod;

    for (child = stmt->child; child; child = child->next) {
        struct ly_in *in;
        LY_CHECK_RET(ly_in_new_memory(child->stmt, &in));
        enum ly_stmt kw = lysp_match_kw(in, NULL);
        ly_in_free(in, 0);

        switch (kw) {
        case LY_STMT_BASE:
            LY_CHECK_RET(lysp_stmt_text_fields(ctx, child, LYEXT_SUBSTMT_BASE, &type->bases, Y_PREF_IDENTIF_ARG, &type->exts));
            type->flags |= LYS_SET_BASE;
            break;
        case LY_STMT_BIT:
            LY_CHECK_RET(lysp_stmt_type_enum(ctx, child, kw, &type->bits));
            type->flags |= LYS_SET_BIT;
            break;
        case LY_STMT_ENUM:
            LY_CHECK_RET(lysp_stmt_type_enum(ctx, child, kw, &type->enums));
            type->flags |= LYS_SET_ENUM;
            break;
        case LY_STMT_FRACTION_DIGITS:
            LY_CHECK_RET(lysp_stmt_type_fracdigits(ctx, child, &type->fraction_digits, &type->exts));
            type->flags |= LYS_SET_FRDIGITS;
            break;
        case LY_STMT_LENGTH:
            if (type->length) {
                LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(kw));
                return LY_EVALID;
            }
            type->length = calloc(1, sizeof *type->length);
            LY_CHECK_ERR_RET(!type->length, LOGMEM(PARSER_CTX(ctx)), LY_EMEM);

            LY_CHECK_RET(lysp_stmt_restr(ctx, child, kw, type->length));
            type->flags |= LYS_SET_LENGTH;
            break;
        case LY_STMT_PATH:
            LY_CHECK_RET(lysp_stmt_text_field(ctx, child, LYEXT_SUBSTMT_PATH, 0, &str_path, Y_STR_ARG, &type->exts));
            ret = ly_path_parse(PARSER_CTX(ctx), NULL, str_path, 0, LY_PATH_BEGIN_EITHER, LY_PATH_LREF_TRUE,
                    LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_LEAFREF, &type->path);
            lydict_remove(PARSER_CTX(ctx), str_path);
            LY_CHECK_RET(ret);
            type->flags |= LYS_SET_PATH;
            break;
        case LY_STMT_PATTERN:
            LY_CHECK_RET(lysp_stmt_type_pattern(ctx, child, &type->patterns));
            type->flags |= LYS_SET_PATTERN;
            break;
        case LY_STMT_RANGE:
            if (type->range) {
                LOGVAL_PARSER(ctx, LY_VCODE_DUPSTMT, ly_stmt2str(kw));
                return LY_EVALID;
            }
            type->range = calloc(1, sizeof *type->range);
            LY_CHECK_ERR_RET(!type->range, LOGMEM(PARSER_CTX(ctx)), LY_EMEM);

            LY_CHECK_RET(lysp_stmt_restr(ctx, child, kw, type->range));
            type->flags |= LYS_SET_RANGE;
            break;
        case LY_STMT_REQUIRE_INSTANCE:
            LY_CHECK_RET(lysp_stmt_type_reqinstance(ctx, child, &type->require_instance, &type->flags, &type->exts));
            /* LYS_SET_REQINST checked and set inside parse_type_reqinstance() */
            break;
        case LY_STMT_TYPE:
            LY_ARRAY_NEW_RET(PARSER_CTX(ctx), type->types, nest_type, LY_EMEM);
            LY_CHECK_RET(lysp_stmt_type(ctx, child, nest_type));
            type->flags |= LYS_SET_TYPE;
            break;
        case LY_STMT_EXTENSION_INSTANCE:
            LY_CHECK_RET(lysp_stmt_ext(ctx, child, LYEXT_SUBSTMT_SELF, 0, &type->exts));
            break;
        default:
            LOGVAL_PARSER(ctx, LY_VCODE_INCHILDSTMT, ly_stmt2str(kw), "type");
            return LY_EVALID;
        }
    }
    return LY_SUCCESS;
}

LY_ERR
lysp_stmt_parse(struct lysc_ctx *ctx, const struct lysp_stmt *stmt, enum ly_stmt kw, void **result, struct lysp_ext_instance **exts)
{
    LY_ERR ret = LY_SUCCESS;
    struct lys_yang_parser_ctx pctx = {0};

    pctx.format = LYS_IN_YANG;
    pctx.parsed_mod = ctx->pmod;
    pctx.pos_type = LY_VLOG_STR;
    pctx.path = ctx->path;

    switch (kw) {
    case LY_STMT_STATUS:
        ret = lysp_stmt_status((struct lys_parser_ctx *)&pctx, stmt, *(uint16_t **)result, exts);
        break;
    case LY_STMT_TYPE: {
        struct lysp_type *type;
        type = calloc(1, sizeof *type);

        ret = lysp_stmt_type((struct lys_parser_ctx *)&pctx, stmt, type);
        (*result) = type;
        break;
    }
    default:
        LOGINT(ctx->ctx);
        return LY_EINT;
    }

    return ret;
}
