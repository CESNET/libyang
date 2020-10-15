/**
 * @file printer_yang.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief YANG printer
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "log.h"
#include "plugins_types.h"
#include "printer.h"
#include "printer_internal.h"
#include "printer_schema.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xpath.h"

/**
 * @brief Types of the YANG printers
 */
enum schema_type {
    YPR_PARSED,   /**< YANG printer of the parsed schema */
    YPR_COMPILED  /**< YANG printer of the compiled schema */
};

/**
 * @brief YANG printer context.
 */
struct ypr_ctx {
    struct ly_out *out;              /**< output specification */
    uint16_t level;                  /**< current indentation level: 0 - no formatting, >= 1 indentation levels */
    uint32_t options;                /**< Schema output options (see @ref schemaprinterflags). */
    const struct lys_module *module; /**< schema to print */
    enum schema_type schema;         /**< type of the schema to print */
};

/**
 * @brief Print the given text as content of a double quoted YANG string,
 * including encoding characters that have special meanings. The quotation marks
 * are not printed.
 *
 * Follows RFC 7950, section 6.1.3.
 *
 * @param[in] out Output specification.
 * @param[in] text String to be printed.
 * @param[in] len Length of the string from @p text to be printed. In case of -1,
 * the @p text is printed completely as a NULL-terminated string.
 */
static void
ypr_encode(struct ly_out *out, const char *text, ssize_t len)
{
    size_t i, start_len;
    const char *start;
    char special = 0;

    if (!len) {
        return;
    }

    if (len < 0) {
        len = strlen(text);
    }

    start = text;
    start_len = 0;
    for (i = 0; i < (size_t)len; ++i) {
        switch (text[i]) {
        case '\n':
        case '\t':
        case '\"':
        case '\\':
            special = text[i];
            break;
        default:
            ++start_len;
            break;
        }

        if (special) {
            ly_write_(out, start, start_len);
            switch (special) {
            case '\n':
                ly_write_(out, "\\n", 2);
                break;
            case '\t':
                ly_write_(out, "\\t", 2);
                break;
            case '\"':
                ly_write_(out, "\\\"", 2);
                break;
            case '\\':
                ly_write_(out, "\\\\", 2);
                break;
            }

            start += start_len + 1;
            start_len = 0;

            special = 0;
        }
    }

    ly_write_(out, start, start_len);
}

static void
ypr_open(struct ly_out *out, ly_bool *flag)
{
    if (flag && !*flag) {
        *flag = 1;
        ly_print_(out, " {\n");
    }
}

static void
ypr_close(struct ypr_ctx *ctx, ly_bool flag)
{
    if (flag) {
        ly_print_(ctx->out, "%*s}\n", INDENT);
    } else {
        ly_print_(ctx->out, ";\n");
    }
}

static void
ypr_text(struct ypr_ctx *ctx, const char *name, const char *text, ly_bool singleline, ly_bool closed)
{
    const char *s, *t;

    if (singleline) {
        ly_print_(ctx->out, "%*s%s \"", INDENT, name);
    } else {
        ly_print_(ctx->out, "%*s%s\n", INDENT, name);
        LEVEL++;

        ly_print_(ctx->out, "%*s\"", INDENT);
    }
    t = text;
    while ((s = strchr(t, '\n'))) {
        ypr_encode(ctx->out, t, s - t);
        ly_print_(ctx->out, "\n");
        t = s + 1;
        if (*t != '\n') {
            ly_print_(ctx->out, "%*s ", INDENT);
        }
    }

    ypr_encode(ctx->out, t, strlen(t));
    if (closed) {
        ly_print_(ctx->out, "\";\n");
    } else {
        ly_print_(ctx->out, "\"");
    }
    if (!singleline) {
        LEVEL--;
    }
}

static void
yprp_stmt(struct ypr_ctx *ctx, struct lysp_stmt *stmt)
{
    struct lysp_stmt *childstmt;
    const char *s, *t;

    if (stmt->arg) {
        if (stmt->flags) {
            ly_print_(ctx->out, "%*s%s\n", INDENT, stmt->stmt);
            LEVEL++;
            ly_print_(ctx->out, "%*s%c", INDENT, (stmt->flags & LYS_DOUBLEQUOTED) ? '\"' : '\'');
            t = stmt->arg;
            while ((s = strchr(t, '\n'))) {
                ypr_encode(ctx->out, t, s - t);
                ly_print_(ctx->out, "\n");
                t = s + 1;
                if (*t != '\n') {
                    ly_print_(ctx->out, "%*s ", INDENT);
                }
            }
            LEVEL--;
            ypr_encode(ctx->out, t, strlen(t));
            ly_print_(ctx->out, "%c%s", (stmt->flags & LYS_DOUBLEQUOTED) ? '\"' : '\'', stmt->child ? " {\n" : ";\n");
        } else {
            ly_print_(ctx->out, "%*s%s %s%s", INDENT, stmt->stmt, stmt->arg, stmt->child ? " {\n" : ";\n");
        }
    } else {
        ly_print_(ctx->out, "%*s%s%s", INDENT, stmt->stmt, stmt->child ? " {\n" : ";\n");
    }

    if (stmt->child) {
        LEVEL++;
        LY_LIST_FOR(stmt->child, childstmt) {
            yprp_stmt(ctx, childstmt);
        }
        LEVEL--;
        ly_print_(ctx->out, "%*s}\n", INDENT);
    }
}

/**
 * @param[in] count Number of extensions to print, 0 to print them all.
 */
static void
yprp_extension_instances(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index,
        struct lysp_ext_instance *ext, ly_bool *flag, LY_ARRAY_COUNT_TYPE count)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_stmt *stmt;
    ly_bool child_presence;
    const char *argument;

    if (!count && ext) {
        count = LY_ARRAY_COUNT(ext);
    }
    LY_ARRAY_FOR(ext, u) {
        if (!count) {
            break;
        }

        count--;
        if ((ext->insubstmt != substmt) || (ext->insubstmt_index != substmt_index)) {
            continue;
        }

        if (!ext->compiled && ext->yin) {
            ly_print_(ctx->out, "%*s%s; // Model comes from different input format, extensions must be resolved first.\n", INDENT, ext[u].name);
            continue;
        }

        ypr_open(ctx->out, flag);
        argument = NULL;
        if (ext[u].compiled) {
            argument = ext[u].compiled->argument;
        } else {
            argument = ext[u].argument;
        }
        if (argument) {
            ly_print_(ctx->out, "%*s%s \"", INDENT, ext[u].name);
            ypr_encode(ctx->out, argument, -1);
            ly_print_(ctx->out, "\"");
        } else {
            ly_print_(ctx->out, "%*s%s", INDENT, ext[u].name);
        }

        child_presence = 0;
        LEVEL++;
        LY_LIST_FOR(ext[u].child, stmt) {
            if (stmt->flags & (LYS_YIN_ATTR | LYS_YIN_ARGUMENT)) {
                continue;
            }
            if (!child_presence) {
                ly_print_(ctx->out, " {\n");
                child_presence = 1;
            }
            yprp_stmt(ctx, stmt);
        }
        LEVEL--;
        if (child_presence) {
            ly_print_(ctx->out, "%*s}\n", INDENT);
        } else {
            ly_print_(ctx->out, ";\n");
        }
    }
}

/**
 * @param[in] count Number of extensions to print, 0 to print them all.
 */
static void
yprc_extension_instances(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index,
        struct lysc_ext_instance *ext, ly_bool *flag, LY_ARRAY_COUNT_TYPE count)
{
    LY_ARRAY_COUNT_TYPE u;

    if (!count && ext) {
        count = LY_ARRAY_COUNT(ext);
    }
    LY_ARRAY_FOR(ext, u) {
        if (!count) {
            break;
        }
        /* TODO compiled extensions */
        (void) ctx;
        (void) substmt;
        (void) substmt_index;
        (void) flag;

        count--;
    }
}

static void
ypr_substmt(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index, const char *text, void *ext)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool extflag = 0;

    if (!text) {
        /* nothing to print */
        return;
    }

    if (ext_substmt_info[substmt].flags & SUBST_FLAG_ID) {
        ly_print_(ctx->out, "%*s%s %s", INDENT, ext_substmt_info[substmt].name, text);
    } else {
        ypr_text(ctx, ext_substmt_info[substmt].name, text,
                (ext_substmt_info[substmt].flags & SUBST_FLAG_YIN) ? 0 : 1, 0);
    }

    LEVEL++;
    LY_ARRAY_FOR(ext, u) {
        if ((((struct lysp_ext_instance *)ext)[u].insubstmt != substmt) || (((struct lysp_ext_instance *)ext)[u].insubstmt_index != substmt_index)) {
            continue;
        }
        if (ctx->schema == YPR_PARSED) {
            yprp_extension_instances(ctx, substmt, substmt_index, &((struct lysp_ext_instance *)ext)[u], &extflag, 1);
        } else {
            yprc_extension_instances(ctx, substmt, substmt_index, &((struct lysc_ext_instance *)ext)[u], &extflag, 1);
        }
    }
    LEVEL--;
    ypr_close(ctx, extflag);
}

static void
ypr_unsigned(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index, void *exts, unsigned long int attr_value, ly_bool *flag)
{
    char *str;

    if (asprintf(&str, "%lu", attr_value) == -1) {
        LOGMEM(ctx->module->ctx);
        return;
    }
    ypr_open(ctx->out, flag);
    ypr_substmt(ctx, substmt, substmt_index, str, exts);
    free(str);
}

static void
ypr_signed(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index, void *exts, signed long int attr_value, ly_bool *flag)
{
    char *str;

    if (asprintf(&str, "%ld", attr_value) == -1) {
        LOGMEM(ctx->module->ctx);
        return;
    }
    ypr_open(ctx->out, flag);
    ypr_substmt(ctx, substmt, substmt_index, str, exts);
    free(str);
}

static void
yprp_revision(struct ypr_ctx *ctx, const struct lysp_revision *rev)
{
    if (rev->dsc || rev->ref || rev->exts) {
        ly_print_(ctx->out, "%*srevision %s {\n", INDENT, rev->date);
        LEVEL++;
        yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, rev->exts, NULL, 0);
        ypr_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, rev->dsc, rev->exts);
        ypr_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, rev->ref, rev->exts);
        LEVEL--;
        ly_print_(ctx->out, "%*s}\n", INDENT);
    } else {
        ly_print_(ctx->out, "%*srevision %s;\n", INDENT, rev->date);
    }
}

static void
ypr_mandatory(struct ypr_ctx *ctx, uint16_t flags, void *exts, ly_bool *flag)
{
    if (flags & LYS_MAND_MASK) {
        ypr_open(ctx->out, flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_MANDATORY, 0, (flags & LYS_MAND_TRUE) ? "true" : "false", exts);
    }
}

static void
ypr_config(struct ypr_ctx *ctx, uint16_t flags, void *exts, ly_bool *flag)
{
    if (flags & LYS_CONFIG_MASK) {
        ypr_open(ctx->out, flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_CONFIG, 0, (flags & LYS_CONFIG_W) ? "true" : "false", exts);
    }
}

static void
ypr_status(struct ypr_ctx *ctx, uint16_t flags, void *exts, ly_bool *flag)
{
    const char *status = NULL;

    if (flags & LYS_STATUS_CURR) {
        ypr_open(ctx->out, flag);
        status = "current";
    } else if (flags & LYS_STATUS_DEPRC) {
        ypr_open(ctx->out, flag);
        status = "deprecated";
    } else if (flags & LYS_STATUS_OBSLT) {
        ypr_open(ctx->out, flag);
        status = "obsolete";
    }

    ypr_substmt(ctx, LYEXT_SUBSTMT_STATUS, 0, status, exts);
}

static void
ypr_description(struct ypr_ctx *ctx, const char *dsc, void *exts, ly_bool *flag)
{
    if (dsc) {
        ypr_open(ctx->out, flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, dsc, exts);
    }
}

static void
ypr_reference(struct ypr_ctx *ctx, const char *ref, void *exts, ly_bool *flag)
{
    if (ref) {
        ypr_open(ctx->out, flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, ref, exts);
    }
}

static void
yprp_iffeatures(struct ypr_ctx *ctx, struct lysp_qname *iffs, struct lysp_ext_instance *exts, ly_bool *flag)
{
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool extflag;

    LY_ARRAY_FOR(iffs, u) {
        ypr_open(ctx->out, flag);
        extflag = 0;

        ly_print_(ctx->out, "%*sif-feature \"%s\"", INDENT, iffs[u].str);

        /* extensions */
        LEVEL++;
        LY_ARRAY_FOR(exts, v) {
            yprp_extension_instances(ctx, LYEXT_SUBSTMT_IFFEATURE, u, &exts[v], &extflag, 1);
        }
        LEVEL--;
        ypr_close(ctx, extflag);
    }
}

static void
yprc_iffeature(struct ypr_ctx *ctx, struct lysc_iffeature *feat, size_t *index_e, size_t *index_f)
{
    ly_bool brackets_flag = *index_e ? 1 : 0;
    uint8_t op;

    op = lysc_iff_getop(feat->expr, *index_e);
    (*index_e)++;

    switch (op) {
    case LYS_IFF_F:
        if (ctx->module == feat->features[*index_f]->module) {
            ly_print_(ctx->out, "%s", feat->features[*index_f]->name);
        } else {
            ly_print_(ctx->out, "%s:%s", feat->features[*index_f]->module->prefix, feat->features[*index_f]->name);
        }
        (*index_f)++;
        break;
    case LYS_IFF_NOT:
        ly_print_(ctx->out, "not ");
        yprc_iffeature(ctx, feat, index_e, index_f);
        break;
    case LYS_IFF_AND:
        if (brackets_flag) {
            /* AND need brackets only if previous op was not */
            if ((*index_e < 2) || (lysc_iff_getop(feat->expr, *index_e - 2) != LYS_IFF_NOT)) {
                brackets_flag = 0;
            }
        }
    /* falls through */
    case LYS_IFF_OR:
        if (brackets_flag) {
            ly_print_(ctx->out, "(");
        }
        yprc_iffeature(ctx, feat, index_e, index_f);
        ly_print_(ctx->out, " %s ", op == LYS_IFF_OR ? "or" : "and");
        yprc_iffeature(ctx, feat, index_e, index_f);
        if (brackets_flag) {
            ly_print_(ctx->out, ")");
        }
    }
}

static void
yprc_iffeatures(struct ypr_ctx *ctx, struct lysc_iffeature *iff, struct lysc_ext_instance *exts, ly_bool *flag)
{
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool extflag;

    LY_ARRAY_FOR(iff, u) {
        size_t index_e = 0, index_f = 0;

        ypr_open(ctx->out, flag);
        extflag = 0;

        ly_print_(ctx->out, "%*sif-feature \"", INDENT);
        yprc_iffeature(ctx, iff, &index_e, &index_f);
        ly_print_(ctx->out, "\"");

        /* extensions */
        LEVEL++;
        LY_ARRAY_FOR(exts, v) {
            if ((exts[v].insubstmt != LYEXT_SUBSTMT_IFFEATURE) || (exts[v].insubstmt_index != u)) {
                continue;
            }
            yprc_extension_instances(ctx, LYEXT_SUBSTMT_IFFEATURE, u, &exts[v], &extflag, 1);
        }
        LEVEL--;
        ypr_close(ctx, extflag);
    }
}

static void
yprp_extension(struct ypr_ctx *ctx, const struct lysp_ext *ext)
{
    ly_bool flag = 0, flag2 = 0;
    LY_ARRAY_COUNT_TYPE u;

    ly_print_(ctx->out, "%*sextension %s", INDENT, ext->name);
    LEVEL++;

    if (ext->exts) {
        yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, ext->exts, &flag, 0);
    }

    if (ext->argument) {
        ypr_open(ctx->out, &flag);
        ly_print_(ctx->out, "%*sargument %s", INDENT, ext->argument);
        LEVEL++;
        if (ext->exts) {
            u = -1;
            while ((u = lysp_ext_instance_iter(ext->exts, u + 1, LYEXT_SUBSTMT_ARGUMENT)) != LY_ARRAY_COUNT(ext->exts)) {
                yprp_extension_instances(ctx, LYEXT_SUBSTMT_ARGUMENT, 0, &ext->exts[u], &flag2, 1);
            }
        }
        if ((ext->flags & LYS_YINELEM_MASK) ||
                (ext->exts && (lysp_ext_instance_iter(ext->exts, 0, LYEXT_SUBSTMT_YINELEM) != LY_ARRAY_COUNT(ext->exts)))) {
            ypr_open(ctx->out, &flag2);
            ypr_substmt(ctx, LYEXT_SUBSTMT_YINELEM, 0, (ext->flags & LYS_YINELEM_TRUE) ? "true" : "false", ext->exts);
        }
        LEVEL--;
        ypr_close(ctx, flag2);
    }

    ypr_status(ctx, ext->flags, ext->exts, &flag);
    ypr_description(ctx, ext->dsc, ext->exts, &flag);
    ypr_reference(ctx, ext->ref, ext->exts, &flag);

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprp_feature(struct ypr_ctx *ctx, const struct lysp_feature *feat)
{
    ly_bool flag = 0;

    ly_print_(ctx->out, "\n%*sfeature %s", INDENT, feat->name);
    LEVEL++;
    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, feat->exts, &flag, 0);
    yprp_iffeatures(ctx, feat->iffeatures, feat->exts, &flag);
    ypr_status(ctx, feat->flags, feat->exts, &flag);
    ypr_description(ctx, feat->dsc, feat->exts, &flag);
    ypr_reference(ctx, feat->ref, feat->exts, &flag);
    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprc_feature(struct ypr_ctx *ctx, const struct lysc_feature *feat)
{
    ly_bool flag = 0;

    ly_print_(ctx->out, "\n%*sfeature %s", INDENT, feat->name);
    LEVEL++;
    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, feat->exts, &flag, 0);
    yprc_iffeatures(ctx, feat->iffeatures, feat->exts, &flag);
    ypr_status(ctx, feat->flags, feat->exts, &flag);
    ypr_description(ctx, feat->dsc, feat->exts, &flag);
    ypr_reference(ctx, feat->ref, feat->exts, &flag);
    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprp_identity(struct ypr_ctx *ctx, const struct lysp_ident *ident)
{
    ly_bool flag = 0;
    LY_ARRAY_COUNT_TYPE u;

    ly_print_(ctx->out, "\n%*sidentity %s", INDENT, ident->name);
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, ident->exts, &flag, 0);
    yprp_iffeatures(ctx, ident->iffeatures, ident->exts, &flag);

    LY_ARRAY_FOR(ident->bases, u) {
        ypr_open(ctx->out, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_BASE, u, ident->bases[u], ident->exts);
    }

    ypr_status(ctx, ident->flags, ident->exts, &flag);
    ypr_description(ctx, ident->dsc, ident->exts, &flag);
    ypr_reference(ctx, ident->ref, ident->exts, &flag);

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprc_identity(struct ypr_ctx *ctx, const struct lysc_ident *ident)
{
    ly_bool flag = 0;
    LY_ARRAY_COUNT_TYPE u;

    ly_print_(ctx->out, "\n%*sidentity %s", INDENT, ident->name);
    LEVEL++;

    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, ident->exts, &flag, 0);
    yprc_iffeatures(ctx, ident->iffeatures, ident->exts, &flag);

    LY_ARRAY_FOR(ident->derived, u) {
        ypr_open(ctx->out, &flag);
        if (ctx->module != ident->derived[u]->module) {
            ly_print_(ctx->out, "%*sderived %s:%s;\n", INDENT, ident->derived[u]->module->prefix, ident->derived[u]->name);
        } else {
            ly_print_(ctx->out, "%*sderived %s;\n", INDENT, ident->derived[u]->name);
        }
    }

    ypr_status(ctx, ident->flags, ident->exts, &flag);
    ypr_description(ctx, ident->dsc, ident->exts, &flag);
    ypr_reference(ctx, ident->ref, ident->exts, &flag);

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprp_restr(struct ypr_ctx *ctx, const struct lysp_restr *restr, const char *name, ly_bool *flag)
{
    ly_bool inner_flag = 0;

    if (!restr) {
        return;
    }

    ypr_open(ctx->out, flag);
    ly_print_(ctx->out, "%*s%s \"", INDENT, name);
    ypr_encode(ctx->out, (restr->arg.str[0] != 0x15 && restr->arg.str[0] != 0x06) ? restr->arg.str : &restr->arg.str[1], -1);
    ly_print_(ctx->out, "\"");

    LEVEL++;
    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, restr->exts, &inner_flag, 0);
    if (restr->arg.str[0] == 0x15) {
        /* special byte value in pattern's expression: 0x15 - invert-match, 0x06 - match */
        ypr_open(ctx->out, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_MODIFIER, 0, "invert-match", restr->exts);
    }
    if (restr->emsg) {
        ypr_open(ctx->out, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRMSG, 0, restr->emsg, restr->exts);
    }
    if (restr->eapptag) {
        ypr_open(ctx->out, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRTAG, 0, restr->eapptag, restr->exts);
    }
    ypr_description(ctx, restr->dsc, restr->exts, &inner_flag);
    ypr_reference(ctx, restr->ref, restr->exts, &inner_flag);

    LEVEL--;
    ypr_close(ctx, inner_flag);
}

static void
yprc_must(struct ypr_ctx *ctx, const struct lysc_must *must, ly_bool *flag)
{
    ly_bool inner_flag = 0;

    ypr_open(ctx->out, flag);
    ly_print_(ctx->out, "%*smust \"", INDENT);
    ypr_encode(ctx->out, must->cond->expr, -1);
    ly_print_(ctx->out, "\"");

    LEVEL++;
    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, must->exts, &inner_flag, 0);
    if (must->emsg) {
        ypr_open(ctx->out, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRMSG, 0, must->emsg, must->exts);
    }
    if (must->eapptag) {
        ypr_open(ctx->out, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRTAG, 0, must->eapptag, must->exts);
    }
    ypr_description(ctx, must->dsc, must->exts, &inner_flag);
    ypr_reference(ctx, must->ref, must->exts, &inner_flag);

    LEVEL--;
    ypr_close(ctx, inner_flag);
}

static void
yprc_range(struct ypr_ctx *ctx, const struct lysc_range *range, LY_DATA_TYPE basetype, ly_bool *flag)
{
    ly_bool inner_flag = 0;
    LY_ARRAY_COUNT_TYPE u;

    if (!range) {
        return;
    }

    ypr_open(ctx->out, flag);
    ly_print_(ctx->out, "%*s%s \"", INDENT, (basetype == LY_TYPE_STRING || basetype == LY_TYPE_BINARY) ? "length" : "range");
    LY_ARRAY_FOR(range->parts, u) {
        if (u > 0) {
            ly_print_(ctx->out, " | ");
        }
        if (range->parts[u].max_64 == range->parts[u].min_64) {
            if (basetype <= LY_TYPE_STRING) { /* unsigned values */
                ly_print_(ctx->out, "%" PRIu64, range->parts[u].max_u64);
            } else { /* signed values */
                ly_print_(ctx->out, "%" PRId64, range->parts[u].max_64);
            }
        } else {
            if (basetype <= LY_TYPE_STRING) { /* unsigned values */
                ly_print_(ctx->out, "%" PRIu64 "..%" PRIu64, range->parts[u].min_u64, range->parts[u].max_u64);
            } else { /* signed values */
                ly_print_(ctx->out, "%" PRId64 "..%" PRId64, range->parts[u].min_64, range->parts[u].max_64);
            }
        }
    }
    ly_print_(ctx->out, "\"");

    LEVEL++;
    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, range->exts, &inner_flag, 0);
    if (range->emsg) {
        ypr_open(ctx->out, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRMSG, 0, range->emsg, range->exts);
    }
    if (range->eapptag) {
        ypr_open(ctx->out, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRTAG, 0, range->eapptag, range->exts);
    }
    ypr_description(ctx, range->dsc, range->exts, &inner_flag);
    ypr_reference(ctx, range->ref, range->exts, &inner_flag);

    LEVEL--;
    ypr_close(ctx, inner_flag);
}

static void
yprc_pattern(struct ypr_ctx *ctx, const struct lysc_pattern *pattern, ly_bool *flag)
{
    ly_bool inner_flag = 0;

    ypr_open(ctx->out, flag);
    ly_print_(ctx->out, "%*spattern \"", INDENT);
    ypr_encode(ctx->out, pattern->expr, -1);
    ly_print_(ctx->out, "\"");

    LEVEL++;
    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, pattern->exts, &inner_flag, 0);
    if (pattern->inverted) {
        /* special byte value in pattern's expression: 0x15 - invert-match, 0x06 - match */
        ypr_open(ctx->out, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_MODIFIER, 0, "invert-match", pattern->exts);
    }
    if (pattern->emsg) {
        ypr_open(ctx->out, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRMSG, 0, pattern->emsg, pattern->exts);
    }
    if (pattern->eapptag) {
        ypr_open(ctx->out, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRTAG, 0, pattern->eapptag, pattern->exts);
    }
    ypr_description(ctx, pattern->dsc, pattern->exts, &inner_flag);
    ypr_reference(ctx, pattern->ref, pattern->exts, &inner_flag);

    LEVEL--;
    ypr_close(ctx, inner_flag);
}

static void
yprp_when(struct ypr_ctx *ctx, struct lysp_when *when, ly_bool *flag)
{
    ly_bool inner_flag = 0;

    if (!when) {
        return;
    }
    ypr_open(ctx->out, flag);

    ly_print_(ctx->out, "%*swhen \"", INDENT);
    ypr_encode(ctx->out, when->cond, -1);
    ly_print_(ctx->out, "\"");

    LEVEL++;
    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, when->exts, &inner_flag, 0);
    ypr_description(ctx, when->dsc, when->exts, &inner_flag);
    ypr_reference(ctx, when->ref, when->exts, &inner_flag);
    LEVEL--;
    ypr_close(ctx, inner_flag);
}

static void
yprc_when(struct ypr_ctx *ctx, struct lysc_when *when, ly_bool *flag)
{
    ly_bool inner_flag = 0;

    if (!when) {
        return;
    }
    ypr_open(ctx->out, flag);

    ly_print_(ctx->out, "%*swhen \"", INDENT);
    ypr_encode(ctx->out, when->cond->expr, -1);
    ly_print_(ctx->out, "\"");

    LEVEL++;
    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, when->exts, &inner_flag, 0);
    ypr_description(ctx, when->dsc, when->exts, &inner_flag);
    ypr_reference(ctx, when->ref, when->exts, &inner_flag);
    LEVEL--;
    ypr_close(ctx, inner_flag);
}

static void
yprp_enum(struct ypr_ctx *ctx, const struct lysp_type_enum *items, LY_DATA_TYPE type, ly_bool *flag)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool inner_flag;

    LY_ARRAY_FOR(items, u) {
        ypr_open(ctx->out, flag);
        if (type == LY_TYPE_BITS) {
            ly_print_(ctx->out, "%*sbit %s", INDENT, items[u].name);
        } else { /* LY_TYPE_ENUM */
            ly_print_(ctx->out, "%*senum \"", INDENT);
            ypr_encode(ctx->out, items[u].name, -1);
            ly_print_(ctx->out, "\"");
        }
        inner_flag = 0;
        LEVEL++;
        yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, items[u].exts, &inner_flag, 0);
        yprp_iffeatures(ctx, items[u].iffeatures, items[u].exts, &inner_flag);
        if (items[u].flags & LYS_SET_VALUE) {
            if (type == LY_TYPE_BITS) {
                ypr_unsigned(ctx, LYEXT_SUBSTMT_POSITION, 0, items[u].exts, items[u].value, &inner_flag);
            } else { /* LY_TYPE_ENUM */
                ypr_signed(ctx, LYEXT_SUBSTMT_VALUE, 0, items[u].exts, items[u].value, &inner_flag);
            }
        }
        ypr_status(ctx, items[u].flags, items[u].exts, &inner_flag);
        ypr_description(ctx, items[u].dsc, items[u].exts, &inner_flag);
        ypr_reference(ctx, items[u].ref, items[u].exts, &inner_flag);
        LEVEL--;
        ypr_close(ctx, inner_flag);
    }
}

static void
yprp_type(struct ypr_ctx *ctx, const struct lysp_type *type)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;

    ly_print_(ctx->out, "%*stype %s", INDENT, type->name);
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, type->exts, &flag, 0);

    yprp_restr(ctx, type->range, "range", &flag);
    yprp_restr(ctx, type->length, "length", &flag);
    LY_ARRAY_FOR(type->patterns, u) {
        yprp_restr(ctx, &type->patterns[u], "pattern", &flag);
    }
    yprp_enum(ctx, type->bits, LY_TYPE_BITS, &flag);
    yprp_enum(ctx, type->enums, LY_TYPE_ENUM, &flag);

    if (type->path) {
        ypr_open(ctx->out, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_PATH, 0, type->path->expr, type->exts);
    }
    if (type->flags & LYS_SET_REQINST) {
        ypr_open(ctx->out, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_REQINSTANCE, 0, type->require_instance ? "true" : "false", type->exts);
    }
    if (type->flags & LYS_SET_FRDIGITS) {
        ypr_unsigned(ctx, LYEXT_SUBSTMT_FRACDIGITS, 0, type->exts, type->fraction_digits, &flag);
    }
    LY_ARRAY_FOR(type->bases, u) {
        ypr_open(ctx->out, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_BASE, u, type->bases[u], type->exts);
    }
    LY_ARRAY_FOR(type->types, u) {
        ypr_open(ctx->out, &flag);
        yprp_type(ctx, &type->types[u]);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprc_dflt_value(struct ypr_ctx *ctx, const struct lyd_value *value, struct lysc_ext_instance *exts)
{
    ly_bool dynamic;
    const char *str;

    str = value->realtype->plugin->print(value, LY_PREF_JSON, NULL, &dynamic);
    ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, str, exts);
    if (dynamic) {
        free((void *)str);
    }
}

static void
yprc_type(struct ypr_ctx *ctx, const struct lysc_type *type)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;

    ly_print_(ctx->out, "%*stype %s", INDENT, lys_datatype2str(type->basetype));
    LEVEL++;

    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, type->exts, &flag, 0);

    switch (type->basetype) {
    case LY_TYPE_BINARY: {
        struct lysc_type_bin *bin = (struct lysc_type_bin *)type;
        yprc_range(ctx, bin->length, type->basetype, &flag);
        break;
    }
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64: {
        struct lysc_type_num *num = (struct lysc_type_num *)type;
        yprc_range(ctx, num->range, type->basetype, &flag);
        break;
    }
    case LY_TYPE_STRING: {
        struct lysc_type_str *str = (struct lysc_type_str *)type;
        yprc_range(ctx, str->length, type->basetype, &flag);
        LY_ARRAY_FOR(str->patterns, u) {
            yprc_pattern(ctx, str->patterns[u], &flag);
        }
        break;
    }
    case LY_TYPE_BITS:
    case LY_TYPE_ENUM: {
        /* bits and enums structures are compatible */
        struct lysc_type_bits *bits = (struct lysc_type_bits *)type;
        LY_ARRAY_FOR(bits->bits, u) {
            struct lysc_type_bitenum_item *item = &bits->bits[u];
            ly_bool inner_flag = 0;

            ypr_open(ctx->out, &flag);
            ly_print_(ctx->out, "%*s%s \"", INDENT, type->basetype == LY_TYPE_BITS ? "bit" : "enum");
            ypr_encode(ctx->out, item->name, -1);
            ly_print_(ctx->out, "\"");
            LEVEL++;
            yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, item->exts, &inner_flag, 0);
            yprc_iffeatures(ctx, item->iffeatures, item->exts, &inner_flag);
            if (type->basetype == LY_TYPE_BITS) {
                ypr_unsigned(ctx, LYEXT_SUBSTMT_POSITION, 0, item->exts, item->position, &inner_flag);
            } else { /* LY_TYPE_ENUM */
                ypr_signed(ctx, LYEXT_SUBSTMT_VALUE, 0, item->exts, item->value, &inner_flag);
            }
            ypr_status(ctx, item->flags, item->exts, &inner_flag);
            ypr_description(ctx, item->dsc, item->exts, &inner_flag);
            ypr_reference(ctx, item->ref, item->exts, &inner_flag);
            LEVEL--;
            ypr_close(ctx, inner_flag);
        }
        break;
    }
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
        /* nothing to do */
        break;
    case LY_TYPE_DEC64: {
        struct lysc_type_dec *dec = (struct lysc_type_dec *)type;
        ypr_open(ctx->out, &flag);
        ypr_unsigned(ctx, LYEXT_SUBSTMT_FRACDIGITS, 0, type->exts, dec->fraction_digits, &flag);
        yprc_range(ctx, dec->range, dec->basetype, &flag);
        break;
    }
    case LY_TYPE_IDENT: {
        struct lysc_type_identityref *ident = (struct lysc_type_identityref *)type;
        LY_ARRAY_FOR(ident->bases, u) {
            ypr_open(ctx->out, &flag);
            ypr_substmt(ctx, LYEXT_SUBSTMT_BASE, u, ident->bases[u]->name, type->exts);
        }
        break;
    }
    case LY_TYPE_INST: {
        struct lysc_type_instanceid *inst = (struct lysc_type_instanceid *)type;
        ypr_open(ctx->out, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_REQINSTANCE, 0, inst->require_instance ? "true" : "false", inst->exts);
        break;
    }
    case LY_TYPE_LEAFREF: {
        struct lysc_type_leafref *lr = (struct lysc_type_leafref *)type;
        ypr_open(ctx->out, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_PATH, 0, lr->path->expr, lr->exts);
        ypr_substmt(ctx, LYEXT_SUBSTMT_REQINSTANCE, 0, lr->require_instance ? "true" : "false", lr->exts);
        yprc_type(ctx, lr->realtype);
        break;
    }
    case LY_TYPE_UNION: {
        struct lysc_type_union *un = (struct lysc_type_union *)type;
        LY_ARRAY_FOR(un->types, u) {
            ypr_open(ctx->out, &flag);
            yprc_type(ctx, un->types[u]);
        }
        break;
    }
    default:
        LOGINT(ctx->module->ctx);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprp_typedef(struct ypr_ctx *ctx, const struct lysp_tpdf *tpdf)
{
    ly_print_(ctx->out, "\n%*stypedef %s {\n", INDENT, tpdf->name);
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, tpdf->exts, NULL, 0);

    yprp_type(ctx, &tpdf->type);

    if (tpdf->units) {
        ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, tpdf->units, tpdf->exts);
    }
    if (tpdf->dflt.str) {
        ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, tpdf->dflt.str, tpdf->exts);
    }

    ypr_status(ctx, tpdf->flags, tpdf->exts, NULL);
    ypr_description(ctx, tpdf->dsc, tpdf->exts, NULL);
    ypr_reference(ctx, tpdf->ref, tpdf->exts, NULL);

    LEVEL--;
    ly_print_(ctx->out, "%*s}\n", INDENT);
}

static void yprp_node(struct ypr_ctx *ctx, const struct lysp_node *node);
static void yprc_node(struct ypr_ctx *ctx, const struct lysc_node *node);
static void yprp_action(struct ypr_ctx *ctx, const struct lysp_action *action);

static void
yprp_grouping(struct ypr_ctx *ctx, const struct lysp_grp *grp)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysp_node *data;

    ly_print_(ctx->out, "\n%*sgrouping %s", INDENT, grp->name);
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, grp->exts, &flag, 0);
    ypr_status(ctx, grp->flags, grp->exts, &flag);
    ypr_description(ctx, grp->dsc, grp->exts, &flag);
    ypr_reference(ctx, grp->ref, grp->exts, &flag);

    LY_ARRAY_FOR(grp->typedefs, u) {
        ypr_open(ctx->out, &flag);
        yprp_typedef(ctx, &grp->typedefs[u]);
    }

    LY_ARRAY_FOR(grp->groupings, u) {
        ypr_open(ctx->out, &flag);
        yprp_grouping(ctx, &grp->groupings[u]);
    }

    LY_LIST_FOR(grp->data, data) {
        ypr_open(ctx->out, &flag);
        yprp_node(ctx, data);
    }

    LY_ARRAY_FOR(grp->actions, u) {
        yprp_action(ctx, &grp->actions[u]);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprp_inout(struct ypr_ctx *ctx, const struct lysp_action_inout *inout, ly_bool *flag)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node *data;

    if (!inout->data) {
        /* no children */
        return;
    }
    ypr_open(ctx->out, flag);

    ly_print_(ctx->out, "\n%*s%s {\n", INDENT, (inout->nodetype == LYS_INPUT ? "input" : "output"));
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, inout->exts, NULL, 0);
    LY_ARRAY_FOR(inout->musts, u) {
        yprp_restr(ctx, &inout->musts[u], "must", NULL);
    }
    LY_ARRAY_FOR(inout->typedefs, u) {
        yprp_typedef(ctx, &inout->typedefs[u]);
    }
    LY_ARRAY_FOR(inout->groupings, u) {
        yprp_grouping(ctx, &inout->groupings[u]);
    }

    LY_LIST_FOR(inout->data, data) {
        yprp_node(ctx, data);
    }

    LEVEL--;
    ypr_close(ctx, 1);
}

static void
yprc_inout(struct ypr_ctx *ctx, const struct lysc_action *action, const struct lysc_action_inout *inout, ly_bool *flag)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_node *data;

    if (!inout->data) {
        /* input/output is empty */
        return;
    }
    ypr_open(ctx->out, flag);

    ly_print_(ctx->out, "\n%*s%s {\n", INDENT, (&action->input == inout) ? "input" : "output");
    LEVEL++;

    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, (&action->input == inout) ? action->input_exts : action->output_exts, NULL, 0);
    LY_ARRAY_FOR(inout->musts, u) {
        yprc_must(ctx, &inout->musts[u], NULL);
    }

    if (!(ctx->options & LYS_PRINT_NO_SUBSTMT)) {
        LY_LIST_FOR(inout->data, data) {
            yprc_node(ctx, data);
        }
    }

    LEVEL--;
    ypr_close(ctx, 1);
}

static void
yprp_notification(struct ypr_ctx *ctx, const struct lysp_notif *notif)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysp_node *data;

    ly_print_(ctx->out, "%*snotification %s", INDENT, notif->name);

    LEVEL++;
    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, notif->exts, &flag, 0);
    yprp_iffeatures(ctx, notif->iffeatures, notif->exts, &flag);

    LY_ARRAY_FOR(notif->musts, u) {
        yprp_restr(ctx, &notif->musts[u], "must", &flag);
    }
    ypr_status(ctx, notif->flags, notif->exts, &flag);
    ypr_description(ctx, notif->dsc, notif->exts, &flag);
    ypr_reference(ctx, notif->ref, notif->exts, &flag);

    LY_ARRAY_FOR(notif->typedefs, u) {
        ypr_open(ctx->out, &flag);
        yprp_typedef(ctx, &notif->typedefs[u]);
    }

    LY_ARRAY_FOR(notif->groupings, u) {
        ypr_open(ctx->out, &flag);
        yprp_grouping(ctx, &notif->groupings[u]);
    }

    LY_LIST_FOR(notif->data, data) {
        ypr_open(ctx->out, &flag);
        yprp_node(ctx, data);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprc_notification(struct ypr_ctx *ctx, const struct lysc_notif *notif)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysc_node *data;

    ly_print_(ctx->out, "%*snotification %s", INDENT, notif->name);

    LEVEL++;
    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, notif->exts, &flag, 0);
    yprc_iffeatures(ctx, notif->iffeatures, notif->exts, &flag);

    LY_ARRAY_FOR(notif->musts, u) {
        yprc_must(ctx, &notif->musts[u], &flag);
    }
    ypr_status(ctx, notif->flags, notif->exts, &flag);
    ypr_description(ctx, notif->dsc, notif->exts, &flag);
    ypr_reference(ctx, notif->ref, notif->exts, &flag);

    if (!(ctx->options & LYS_PRINT_NO_SUBSTMT)) {
        LY_LIST_FOR(notif->data, data) {
            ypr_open(ctx->out, &flag);
            yprc_node(ctx, data);
        }
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprp_action(struct ypr_ctx *ctx, const struct lysp_action *action)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;

    ly_print_(ctx->out, "%*s%s %s", INDENT, action->parent ? "action" : "rpc", action->name);

    LEVEL++;
    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, action->exts, &flag, 0);
    yprp_iffeatures(ctx, action->iffeatures, action->exts, &flag);
    ypr_status(ctx, action->flags, action->exts, &flag);
    ypr_description(ctx, action->dsc, action->exts, &flag);
    ypr_reference(ctx, action->ref, action->exts, &flag);

    LY_ARRAY_FOR(action->typedefs, u) {
        ypr_open(ctx->out, &flag);
        yprp_typedef(ctx, &action->typedefs[u]);
    }

    LY_ARRAY_FOR(action->groupings, u) {
        ypr_open(ctx->out, &flag);
        yprp_grouping(ctx, &action->groupings[u]);
    }

    yprp_inout(ctx, &action->input, &flag);
    yprp_inout(ctx, &action->output, &flag);

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprc_action(struct ypr_ctx *ctx, const struct lysc_action *action)
{
    ly_bool flag = 0;

    ly_print_(ctx->out, "%*s%s %s", INDENT, action->parent ? "action" : "rpc", action->name);

    LEVEL++;
    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, action->exts, &flag, 0);
    yprc_iffeatures(ctx, action->iffeatures, action->exts, &flag);
    ypr_status(ctx, action->flags, action->exts, &flag);
    ypr_description(ctx, action->dsc, action->exts, &flag);
    ypr_reference(ctx, action->ref, action->exts, &flag);

    yprc_inout(ctx, action, &action->input, &flag);
    yprc_inout(ctx, action, &action->output, &flag);

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprp_node_common1(struct ypr_ctx *ctx, const struct lysp_node *node, ly_bool *flag)
{
    ly_print_(ctx->out, "%*s%s %s%s", INDENT, lys_nodetype2str(node->nodetype), node->name, flag ? "" : " {\n");
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, node->exts, flag, 0);
    yprp_when(ctx, node->when, flag);
    yprp_iffeatures(ctx, node->iffeatures, node->exts, flag);
}

static void
yprc_node_common1(struct ypr_ctx *ctx, const struct lysc_node *node, ly_bool *flag)
{
    LY_ARRAY_COUNT_TYPE u;

    ly_print_(ctx->out, "%*s%s %s%s", INDENT, lys_nodetype2str(node->nodetype), node->name, flag ? "" : " {\n");
    LEVEL++;

    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, node->exts, flag, 0);
    LY_ARRAY_FOR(node->when, u) {
        yprc_when(ctx, node->when[u], flag);
    }
    yprc_iffeatures(ctx, node->iffeatures, node->exts, flag);
}

/* macr oto unify the code */
#define YPR_NODE_COMMON2 \
    ypr_config(ctx, node->flags, node->exts, flag); \
    if (node->nodetype & (LYS_CHOICE | LYS_LEAF | LYS_ANYDATA)) { \
        ypr_mandatory(ctx, node->flags, node->exts, flag); \
    } \
    ypr_status(ctx, node->flags, node->exts, flag); \
    ypr_description(ctx, node->dsc, node->exts, flag); \
    ypr_reference(ctx, node->ref, node->exts, flag)

static void
yprp_node_common2(struct ypr_ctx *ctx, const struct lysp_node *node, ly_bool *flag)
{
    YPR_NODE_COMMON2;
}

static void
yprc_node_common2(struct ypr_ctx *ctx, const struct lysc_node *node, ly_bool *flag)
{
    YPR_NODE_COMMON2;
}

#undef YPR_NODE_COMMON2

static void
yprp_container(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysp_node *child;
    struct lysp_node_container *cont = (struct lysp_node_container *)node;

    yprp_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(cont->musts, u) {
        yprp_restr(ctx, &cont->musts[u], "must", &flag);
    }
    if (cont->presence) {
        ypr_open(ctx->out, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_PRESENCE, 0, cont->presence, cont->exts);
    }

    yprp_node_common2(ctx, node, &flag);

    LY_ARRAY_FOR(cont->typedefs, u) {
        ypr_open(ctx->out, &flag);
        yprp_typedef(ctx, &cont->typedefs[u]);
    }

    LY_ARRAY_FOR(cont->groupings, u) {
        ypr_open(ctx->out, &flag);
        yprp_grouping(ctx, &cont->groupings[u]);
    }

    LY_LIST_FOR(cont->child, child) {
        ypr_open(ctx->out, &flag);
        yprp_node(ctx, child);
    }

    LY_ARRAY_FOR(cont->actions, u) {
        ypr_open(ctx->out, &flag);
        yprp_action(ctx, &cont->actions[u]);
    }

    LY_ARRAY_FOR(cont->notifs, u) {
        ypr_open(ctx->out, &flag);
        yprp_notification(ctx, &cont->notifs[u]);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprc_container(struct ypr_ctx *ctx, const struct lysc_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysc_node *child;
    struct lysc_node_container *cont = (struct lysc_node_container *)node;

    yprc_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(cont->musts, u) {
        yprc_must(ctx, &cont->musts[u], &flag);
    }
    if (cont->flags & LYS_PRESENCE) {
        ypr_open(ctx->out, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_PRESENCE, 0, "true", cont->exts);
    }

    yprc_node_common2(ctx, node, &flag);

    if (!(ctx->options & LYS_PRINT_NO_SUBSTMT)) {
        LY_LIST_FOR(cont->child, child) {
            ypr_open(ctx->out, &flag);
            yprc_node(ctx, child);
        }

        LY_ARRAY_FOR(cont->actions, u) {
            ypr_open(ctx->out, &flag);
            yprc_action(ctx, &cont->actions[u]);
        }

        LY_ARRAY_FOR(cont->notifs, u) {
            ypr_open(ctx->out, &flag);
            yprc_notification(ctx, &cont->notifs[u]);
        }
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprp_case(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    ly_bool flag = 0;
    struct lysp_node *child;
    struct lysp_node_case *cas = (struct lysp_node_case *)node;

    yprp_node_common1(ctx, node, &flag);
    yprp_node_common2(ctx, node, &flag);

    LY_LIST_FOR(cas->child, child) {
        ypr_open(ctx->out, &flag);
        yprp_node(ctx, child);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprc_case(struct ypr_ctx *ctx, const struct lysc_node_case *cs)
{
    ly_bool flag = 0;
    struct lysc_node *child;

    yprc_node_common1(ctx, (struct lysc_node *)cs, &flag);
    yprc_node_common2(ctx, (struct lysc_node *)cs, &flag);

    if (!(ctx->options & LYS_PRINT_NO_SUBSTMT)) {
        for (child = cs->child; child && child->parent == (struct lysc_node *)cs; child = child->next) {
            ypr_open(ctx->out, &flag);
            yprc_node(ctx, child);
        }
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprp_choice(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    ly_bool flag = 0;
    struct lysp_node *child;
    struct lysp_node_choice *choice = (struct lysp_node_choice *)node;

    yprp_node_common1(ctx, node, &flag);

    if (choice->dflt.str) {
        ypr_open(ctx->out, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, choice->dflt.str, choice->exts);
    }

    yprp_node_common2(ctx, node, &flag);

    LY_LIST_FOR(choice->child, child) {
        ypr_open(ctx->out, &flag);
        yprp_node(ctx, child);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprc_choice(struct ypr_ctx *ctx, const struct lysc_node *node)
{
    ly_bool flag = 0;
    struct lysc_node_case *cs;
    struct lysc_node_choice *choice = (struct lysc_node_choice *)node;

    yprc_node_common1(ctx, node, &flag);

    if (choice->dflt) {
        ypr_open(ctx->out, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, choice->dflt->name, choice->exts);
    }

    yprc_node_common2(ctx, node, &flag);

    for (cs = choice->cases; cs; cs = (struct lysc_node_case *)cs->next) {
        ypr_open(ctx->out, &flag);
        yprc_case(ctx, cs);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprp_leaf(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node_leaf *leaf = (struct lysp_node_leaf *)node;

    yprp_node_common1(ctx, node, NULL);

    yprp_type(ctx, &leaf->type);
    ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, leaf->units, leaf->exts);
    LY_ARRAY_FOR(leaf->musts, u) {
        yprp_restr(ctx, &leaf->musts[u], "must", NULL);
    }
    ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, leaf->dflt.str, leaf->exts);

    yprp_node_common2(ctx, node, NULL);

    LEVEL--;
    ly_print_(ctx->out, "%*s}\n", INDENT);
}

static void
yprc_leaf(struct ypr_ctx *ctx, const struct lysc_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_node_leaf *leaf = (struct lysc_node_leaf *)node;

    yprc_node_common1(ctx, node, NULL);

    yprc_type(ctx, leaf->type);
    ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, leaf->units, leaf->exts);
    LY_ARRAY_FOR(leaf->musts, u) {
        yprc_must(ctx, &leaf->musts[u], NULL);
    }

    if (leaf->dflt) {
        yprc_dflt_value(ctx, leaf->dflt, leaf->exts);
    }

    yprc_node_common2(ctx, node, NULL);

    LEVEL--;
    ly_print_(ctx->out, "%*s}\n", INDENT);
}

static void
yprp_leaflist(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node_leaflist *llist = (struct lysp_node_leaflist *)node;

    yprp_node_common1(ctx, node, NULL);

    yprp_type(ctx, &llist->type);
    ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, llist->units, llist->exts);
    LY_ARRAY_FOR(llist->musts, u) {
        yprp_restr(ctx, &llist->musts[u], "must", NULL);
    }
    LY_ARRAY_FOR(llist->dflts, u) {
        ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, u, llist->dflts[u].str, llist->exts);
    }

    ypr_config(ctx, node->flags, node->exts, NULL);

    if (llist->flags & LYS_SET_MIN) {
        ypr_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, llist->exts, llist->min, NULL);
    }
    if (llist->flags & LYS_SET_MAX) {
        if (llist->max) {
            ypr_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, llist->exts, llist->max, NULL);
        } else {
            ypr_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", llist->exts);
        }
    }

    if (llist->flags & LYS_ORDBY_MASK) {
        ypr_substmt(ctx, LYEXT_SUBSTMT_ORDEREDBY, 0, (llist->flags & LYS_ORDBY_USER) ? "user" : "system", llist->exts);
    }

    ypr_status(ctx, node->flags, node->exts, NULL);
    ypr_description(ctx, node->dsc, node->exts, NULL);
    ypr_reference(ctx, node->ref, node->exts, NULL);

    LEVEL--;
    ly_print_(ctx->out, "%*s}\n", INDENT);
}

static void
yprc_leaflist(struct ypr_ctx *ctx, const struct lysc_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_node_leaflist *llist = (struct lysc_node_leaflist *)node;

    yprc_node_common1(ctx, node, NULL);

    yprc_type(ctx, llist->type);
    ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, llist->units, llist->exts);
    LY_ARRAY_FOR(llist->musts, u) {
        yprc_must(ctx, &llist->musts[u], NULL);
    }
    LY_ARRAY_FOR(llist->dflts, u) {
        yprc_dflt_value(ctx, llist->dflts[u], llist->exts);
    }

    ypr_config(ctx, node->flags, node->exts, NULL);

    ypr_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, llist->exts, llist->min, NULL);
    if (llist->max) {
        ypr_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, llist->exts, llist->max, NULL);
    } else {
        ypr_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", llist->exts);
    }

    ypr_substmt(ctx, LYEXT_SUBSTMT_ORDEREDBY, 0, (llist->flags & LYS_ORDBY_USER) ? "user" : "system", llist->exts);

    ypr_status(ctx, node->flags, node->exts, NULL);
    ypr_description(ctx, node->dsc, node->exts, NULL);
    ypr_reference(ctx, node->ref, node->exts, NULL);

    LEVEL--;
    ly_print_(ctx->out, "%*s}\n", INDENT);
}

static void
yprp_list(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysp_node *child;
    struct lysp_node_list *list = (struct lysp_node_list *)node;

    yprp_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(list->musts, u) {
        yprp_restr(ctx, &list->musts[u], "must", NULL);
    }
    if (list->key) {
        ypr_open(ctx->out, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_KEY, 0, list->key, list->exts);
    }
    LY_ARRAY_FOR(list->uniques, u) {
        ypr_open(ctx->out, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_UNIQUE, u, list->uniques[u].str, list->exts);
    }

    ypr_config(ctx, node->flags, node->exts, NULL);

    if (list->flags & LYS_SET_MIN) {
        ypr_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, list->exts, list->min, NULL);
    }
    if (list->flags & LYS_SET_MAX) {
        if (list->max) {
            ypr_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, list->exts, list->max, NULL);
        } else {
            ypr_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", list->exts);
        }
    }

    if (list->flags & LYS_ORDBY_MASK) {
        ypr_substmt(ctx, LYEXT_SUBSTMT_ORDEREDBY, 0, (list->flags & LYS_ORDBY_USER) ? "user" : "system", list->exts);
    }

    ypr_status(ctx, node->flags, node->exts, NULL);
    ypr_description(ctx, node->dsc, node->exts, NULL);
    ypr_reference(ctx, node->ref, node->exts, NULL);

    LY_ARRAY_FOR(list->typedefs, u) {
        ypr_open(ctx->out, &flag);
        yprp_typedef(ctx, &list->typedefs[u]);
    }

    LY_ARRAY_FOR(list->groupings, u) {
        ypr_open(ctx->out, &flag);
        yprp_grouping(ctx, &list->groupings[u]);
    }

    LY_LIST_FOR(list->child, child) {
        ypr_open(ctx->out, &flag);
        yprp_node(ctx, child);
    }

    LY_ARRAY_FOR(list->actions, u) {
        ypr_open(ctx->out, &flag);
        yprp_action(ctx, &list->actions[u]);
    }

    LY_ARRAY_FOR(list->notifs, u) {
        ypr_open(ctx->out, &flag);
        yprp_notification(ctx, &list->notifs[u]);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprc_list(struct ypr_ctx *ctx, const struct lysc_node *node)
{
    LY_ARRAY_COUNT_TYPE u, v;
    ly_bool flag = 0;
    struct lysc_node *child;
    struct lysc_node_list *list = (struct lysc_node_list *)node;

    yprc_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(list->musts, u) {
        yprc_must(ctx, &list->musts[u], NULL);
    }
    if (!(list->flags & LYS_KEYLESS)) {
        ypr_open(ctx->out, &flag);
        ly_print_(ctx->out, "%*skey \"", INDENT);
        for (struct lysc_node *key = list->child; key && key->nodetype == LYS_LEAF && (key->flags & LYS_KEY); key = key->next) {
            ly_print_(ctx->out, "%s%s", u > 0 ? ", " : "", key->name);
        }
        ly_print_(ctx->out, "\";\n");
    }
    LY_ARRAY_FOR(list->uniques, u) {
        ypr_open(ctx->out, &flag);
        ly_print_(ctx->out, "%*sunique \"", INDENT);
        LY_ARRAY_FOR(list->uniques[u], v) {
            ly_print_(ctx->out, "%s%s", v > 0 ? ", " : "", list->uniques[u][v]->name);
        }
        ypr_close(ctx, 0);
    }

    ypr_config(ctx, node->flags, node->exts, NULL);

    ypr_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, list->exts, list->min, NULL);
    if (list->max) {
        ypr_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, list->exts, list->max, NULL);
    } else {
        ypr_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", list->exts);
    }

    ypr_substmt(ctx, LYEXT_SUBSTMT_ORDEREDBY, 0, (list->flags & LYS_ORDBY_USER) ? "user" : "system", list->exts);

    ypr_status(ctx, node->flags, node->exts, NULL);
    ypr_description(ctx, node->dsc, node->exts, NULL);
    ypr_reference(ctx, node->ref, node->exts, NULL);

    if (!(ctx->options & LYS_PRINT_NO_SUBSTMT)) {
        LY_LIST_FOR(list->child, child) {
            ypr_open(ctx->out, &flag);
            yprc_node(ctx, child);
        }

        LY_ARRAY_FOR(list->actions, u) {
            ypr_open(ctx->out, &flag);
            yprc_action(ctx, &list->actions[u]);
        }

        LY_ARRAY_FOR(list->notifs, u) {
            ypr_open(ctx->out, &flag);
            yprc_notification(ctx, &list->notifs[u]);
        }
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprp_refine(struct ypr_ctx *ctx, struct lysp_refine *refine)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;

    ly_print_(ctx->out, "%*srefine \"%s\"", INDENT, refine->nodeid);
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, refine->exts, &flag, 0);
    yprp_iffeatures(ctx, refine->iffeatures, refine->exts, &flag);

    LY_ARRAY_FOR(refine->musts, u) {
        ypr_open(ctx->out, &flag);
        yprp_restr(ctx, &refine->musts[u], "must", NULL);
    }

    if (refine->presence) {
        ypr_open(ctx->out, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_PRESENCE, 0, refine->presence, refine->exts);
    }

    LY_ARRAY_FOR(refine->dflts, u) {
        ypr_open(ctx->out, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, u, refine->dflts[u].str, refine->exts);
    }

    ypr_config(ctx, refine->flags, refine->exts, &flag);
    ypr_mandatory(ctx, refine->flags, refine->exts, &flag);

    if (refine->flags & LYS_SET_MIN) {
        ypr_open(ctx->out, &flag);
        ypr_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, refine->exts, refine->min, NULL);
    }
    if (refine->flags & LYS_SET_MAX) {
        ypr_open(ctx->out, &flag);
        if (refine->max) {
            ypr_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, refine->exts, refine->max, NULL);
        } else {
            ypr_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", refine->exts);
        }
    }

    ypr_description(ctx, refine->dsc, refine->exts, &flag);
    ypr_reference(ctx, refine->ref, refine->exts, &flag);

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprp_augment(struct ypr_ctx *ctx, const struct lysp_augment *aug)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node *child;

    ly_print_(ctx->out, "%*saugment \"%s\" {\n", INDENT, aug->nodeid);
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, aug->exts, NULL, 0);
    yprp_when(ctx, aug->when, NULL);
    yprp_iffeatures(ctx, aug->iffeatures, aug->exts, NULL);
    ypr_status(ctx, aug->flags, aug->exts, NULL);
    ypr_description(ctx, aug->dsc, aug->exts, NULL);
    ypr_reference(ctx, aug->ref, aug->exts, NULL);

    LY_LIST_FOR(aug->child, child) {
        yprp_node(ctx, child);
    }

    LY_ARRAY_FOR(aug->actions, u) {
        yprp_action(ctx, &aug->actions[u]);
    }

    LY_ARRAY_FOR(aug->notifs, u) {
        yprp_notification(ctx, &aug->notifs[u]);
    }

    LEVEL--;
    ypr_close(ctx, 1);
}

static void
yprp_uses(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysp_node_uses *uses = (struct lysp_node_uses *)node;

    yprp_node_common1(ctx, node, &flag);
    yprp_node_common2(ctx, node, &flag);

    LY_ARRAY_FOR(uses->refines, u) {
        ypr_open(ctx->out, &flag);
        yprp_refine(ctx, &uses->refines[u]);
    }

    LY_ARRAY_FOR(uses->augments, u) {
        ypr_open(ctx->out, &flag);
        yprp_augment(ctx, &uses->augments[u]);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprp_anydata(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysp_node_anydata *any = (struct lysp_node_anydata *)node;

    yprp_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(any->musts, u) {
        ypr_open(ctx->out, &flag);
        yprp_restr(ctx, &any->musts[u], "must", NULL);
    }

    yprp_node_common2(ctx, node, &flag);

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprc_anydata(struct ypr_ctx *ctx, const struct lysc_node *node)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool flag = 0;
    struct lysc_node_anydata *any = (struct lysc_node_anydata *)node;

    yprc_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(any->musts, u) {
        ypr_open(ctx->out, &flag);
        yprc_must(ctx, &any->musts[u], NULL);
    }

    yprc_node_common2(ctx, node, &flag);

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
yprp_node(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    switch (node->nodetype) {
    case LYS_CONTAINER:
        yprp_container(ctx, node);
        break;
    case LYS_CHOICE:
        yprp_choice(ctx, node);
        break;
    case LYS_LEAF:
        yprp_leaf(ctx, node);
        break;
    case LYS_LEAFLIST:
        yprp_leaflist(ctx, node);
        break;
    case LYS_LIST:
        yprp_list(ctx, node);
        break;
    case LYS_USES:
        yprp_uses(ctx, node);
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        yprp_anydata(ctx, node);
        break;
    case LYS_CASE:
        yprp_case(ctx, node);
        break;
    default:
        break;
    }
}

static void
yprc_node(struct ypr_ctx *ctx, const struct lysc_node *node)
{
    switch (node->nodetype) {
    case LYS_CONTAINER:
        yprc_container(ctx, node);
        break;
    case LYS_CHOICE:
        yprc_choice(ctx, node);
        break;
    case LYS_LEAF:
        yprc_leaf(ctx, node);
        break;
    case LYS_LEAFLIST:
        yprc_leaflist(ctx, node);
        break;
    case LYS_LIST:
        yprc_list(ctx, node);
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        yprc_anydata(ctx, node);
        break;
    default:
        break;
    }
}

static void
yprp_deviation(struct ypr_ctx *ctx, const struct lysp_deviation *deviation)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_deviate_add *add;
    struct lysp_deviate_rpl *rpl;
    struct lysp_deviate_del *del;
    struct lysp_deviate *elem;

    ly_print_(ctx->out, "%*sdeviation \"%s\" {\n", INDENT, deviation->nodeid);
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, deviation->exts, NULL, 0);
    ypr_description(ctx, deviation->dsc, deviation->exts, NULL);
    ypr_reference(ctx, deviation->ref, deviation->exts, NULL);

    LY_LIST_FOR(deviation->deviates, elem) {
        ly_print_(ctx->out, "%*sdeviate ", INDENT);
        if (elem->mod == LYS_DEV_NOT_SUPPORTED) {
            if (elem->exts) {
                ly_print_(ctx->out, "not-supported {\n");
                LEVEL++;

                yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, elem->exts, NULL, 0);
            } else {
                ly_print_(ctx->out, "not-supported;\n");
                continue;
            }
        } else if (elem->mod == LYS_DEV_ADD) {
            add = (struct lysp_deviate_add *)elem;
            ly_print_(ctx->out, "add {\n");
            LEVEL++;

            yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, add->exts, NULL, 0);
            ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, add->units, add->exts);
            LY_ARRAY_FOR(add->musts, u) {
                yprp_restr(ctx, &add->musts[u], "must", NULL);
            }
            LY_ARRAY_FOR(add->uniques, u) {
                ypr_substmt(ctx, LYEXT_SUBSTMT_UNIQUE, u, add->uniques[u].str, add->exts);
            }
            LY_ARRAY_FOR(add->dflts, u) {
                ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, u, add->dflts[u].str, add->exts);
            }
            ypr_config(ctx, add->flags, add->exts, NULL);
            ypr_mandatory(ctx, add->flags, add->exts, NULL);
            if (add->flags & LYS_SET_MIN) {
                ypr_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, add->exts, add->min, NULL);
            }
            if (add->flags & LYS_SET_MAX) {
                if (add->max) {
                    ypr_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, add->exts, add->max, NULL);
                } else {
                    ypr_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", add->exts);
                }
            }
        } else if (elem->mod == LYS_DEV_REPLACE) {
            rpl = (struct lysp_deviate_rpl *)elem;
            ly_print_(ctx->out, "replace {\n");
            LEVEL++;

            yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, rpl->exts, NULL, 0);
            if (rpl->type) {
                yprp_type(ctx, rpl->type);
            }
            ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, rpl->units, rpl->exts);
            ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, rpl->dflt.str, rpl->exts);
            ypr_config(ctx, rpl->flags, rpl->exts, NULL);
            ypr_mandatory(ctx, rpl->flags, rpl->exts, NULL);
            if (rpl->flags & LYS_SET_MIN) {
                ypr_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, rpl->exts, rpl->min, NULL);
            }
            if (rpl->flags & LYS_SET_MAX) {
                if (rpl->max) {
                    ypr_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, rpl->exts, rpl->max, NULL);
                } else {
                    ypr_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", rpl->exts);
                }
            }
        } else if (elem->mod == LYS_DEV_DELETE) {
            del = (struct lysp_deviate_del *)elem;
            ly_print_(ctx->out, "delete {\n");
            LEVEL++;

            yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, del->exts, NULL, 0);
            ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, del->units, del->exts);
            LY_ARRAY_FOR(del->musts, u) {
                yprp_restr(ctx, &del->musts[u], "must", NULL);
            }
            LY_ARRAY_FOR(del->uniques, u) {
                ypr_substmt(ctx, LYEXT_SUBSTMT_UNIQUE, u, del->uniques[u].str, del->exts);
            }
            LY_ARRAY_FOR(del->dflts, u) {
                ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, u, del->dflts[u].str, del->exts);
            }
        }

        LEVEL--;
        ypr_close(ctx, 1);
    }

    LEVEL--;
    ypr_close(ctx, 1);
}

static void
yang_print_parsed_linkage(struct ypr_ctx *ctx, const struct lysp_module *modp)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(modp->imports, u) {
        ly_print_(ctx->out, "%s%*simport %s {\n", u ? "" : "\n", INDENT, modp->imports[u].name);
        LEVEL++;
        yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, modp->imports[u].exts, NULL, 0);
        ypr_substmt(ctx, LYEXT_SUBSTMT_PREFIX, 0, modp->imports[u].prefix, modp->imports[u].exts);
        if (modp->imports[u].rev[0]) {
            ypr_substmt(ctx, LYEXT_SUBSTMT_REVISIONDATE, 0, modp->imports[u].rev, modp->imports[u].exts);
        }
        ypr_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, modp->imports[u].dsc, modp->imports[u].exts);
        ypr_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, modp->imports[u].ref, modp->imports[u].exts);
        LEVEL--;
        ly_print_(ctx->out, "%*s}\n", INDENT);
    }
    LY_ARRAY_FOR(modp->includes, u) {
        if (modp->includes[u].rev[0] || modp->includes[u].dsc || modp->includes[u].ref || modp->includes[u].exts) {
            ly_print_(ctx->out, "%s%*sinclude %s {\n", u ? "" : "\n",  INDENT, modp->includes[u].name);
            LEVEL++;
            yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, modp->includes[u].exts, NULL, 0);
            if (modp->includes[u].rev[0]) {
                ypr_substmt(ctx, LYEXT_SUBSTMT_REVISIONDATE, 0, modp->includes[u].rev, modp->includes[u].exts);
            }
            ypr_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, modp->includes[u].dsc, modp->includes[u].exts);
            ypr_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, modp->includes[u].ref, modp->includes[u].exts);
            LEVEL--;
            ly_print_(ctx->out, "%*s}\n", INDENT);
        } else {
            ly_print_(ctx->out, "\n%*sinclude \"%s\";\n", INDENT, modp->includes[u].name);
        }
    }
}

static void
yang_print_parsed_body(struct ypr_ctx *ctx, const struct lysp_module *modp)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_node *data;

    LY_ARRAY_FOR(modp->extensions, u) {
        ly_print_(ctx->out, "\n");
        yprp_extension(ctx, &modp->extensions[u]);
    }
    if (modp->exts) {
        ly_print_(ctx->out, "\n");
        yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, modp->exts, NULL, 0);
    }

    LY_ARRAY_FOR(modp->features, u) {
        yprp_feature(ctx, &modp->features[u]);
    }

    LY_ARRAY_FOR(modp->identities, u) {
        yprp_identity(ctx, &modp->identities[u]);
    }

    LY_ARRAY_FOR(modp->typedefs, u) {
        yprp_typedef(ctx, &modp->typedefs[u]);
    }

    LY_ARRAY_FOR(modp->groupings, u) {
        yprp_grouping(ctx, &modp->groupings[u]);
    }

    LY_LIST_FOR(modp->data, data) {
        yprp_node(ctx, data);
    }

    LY_ARRAY_FOR(modp->augments, u) {
        yprp_augment(ctx, &modp->augments[u]);
    }

    LY_ARRAY_FOR(modp->rpcs, u) {
        yprp_action(ctx, &modp->rpcs[u]);
    }

    LY_ARRAY_FOR(modp->notifs, u) {
        yprp_notification(ctx, &modp->notifs[u]);
    }

    LY_ARRAY_FOR(modp->deviations, u) {
        yprp_deviation(ctx, &modp->deviations[u]);
    }
}

LY_ERR
yang_print_parsed_module(struct ly_out *out, const struct lys_module *module, const struct lysp_module *modp, uint32_t options)
{
    LY_ARRAY_COUNT_TYPE u;
    struct ypr_ctx ctx_ = {.out = out, .level = 0, .module = module, .schema = YPR_PARSED, .options = options}, *ctx = &ctx_;

    ly_print_(ctx->out, "%*smodule %s {\n", INDENT, module->name);
    LEVEL++;

    /* module-header-stmts */
    if (modp->version) {
        ypr_substmt(ctx, LYEXT_SUBSTMT_VERSION, 0, modp->version == LYS_VERSION_1_1 ? "1.1" : "1", modp->exts);
    }

    ypr_substmt(ctx, LYEXT_SUBSTMT_NAMESPACE, 0, module->ns, modp->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_PREFIX, 0, module->prefix, modp->exts);

    /* linkage-stmts (import/include) */
    yang_print_parsed_linkage(ctx, modp);

    /* meta-stmts */
    if (module->org || module->contact || module->dsc || module->ref) {
        ly_print_(out, "\n");
    }
    ypr_substmt(ctx, LYEXT_SUBSTMT_ORGANIZATION, 0, module->org, modp->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_CONTACT, 0, module->contact, modp->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, module->dsc, modp->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, module->ref, modp->exts);

    /* revision-stmts */
    if (modp->revs) {
        ly_print_(out, "\n");
    }
    LY_ARRAY_FOR(modp->revs, u) {
        yprp_revision(ctx, &modp->revs[u]);
    }
    /* body-stmts */
    yang_print_parsed_body(ctx, modp);

    LEVEL--;
    ly_print_(out, "%*s}\n", INDENT);
    ly_print_flush(out);

    return LY_SUCCESS;
}

static void
yprp_belongsto(struct ypr_ctx *ctx, const struct lysp_submodule *submodp)
{
    ly_print_(ctx->out, "%*sbelongs-to %s {\n", INDENT, submodp->mod->name);
    LEVEL++;
    yprp_extension_instances(ctx, LYEXT_SUBSTMT_BELONGSTO, 0, submodp->exts, NULL, 0);
    ypr_substmt(ctx, LYEXT_SUBSTMT_PREFIX, 0, submodp->prefix, submodp->exts);
    LEVEL--;
    ly_print_(ctx->out, "%*s}\n", INDENT);
}

LY_ERR
yang_print_parsed_submodule(struct ly_out *out, const struct lys_module *module, const struct lysp_submodule *submodp, uint32_t options)
{
    LY_ARRAY_COUNT_TYPE u;
    struct ypr_ctx ctx_ = {.out = out, .level = 0, .module = module, .schema = YPR_PARSED, .options = options}, *ctx = &ctx_;

    ly_print_(ctx->out, "%*ssubmodule %s {\n", INDENT, submodp->name);
    LEVEL++;

    /* submodule-header-stmts */
    if (submodp->version) {
        ypr_substmt(ctx, LYEXT_SUBSTMT_VERSION, 0, submodp->version == LYS_VERSION_1_1 ? "1.1" : "1", submodp->exts);
    }

    yprp_belongsto(ctx, submodp);

    /* linkage-stmts (import/include) */
    yang_print_parsed_linkage(ctx, (struct lysp_module *)submodp);

    /* meta-stmts */
    if (submodp->org || submodp->contact || submodp->dsc || submodp->ref) {
        ly_print_(out, "\n");
    }
    ypr_substmt(ctx, LYEXT_SUBSTMT_ORGANIZATION, 0, submodp->org, submodp->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_CONTACT, 0, submodp->contact, submodp->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, submodp->dsc, submodp->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, submodp->ref, submodp->exts);

    /* revision-stmts */
    if (submodp->revs) {
        ly_print_(out, "\n");
    }
    LY_ARRAY_FOR(submodp->revs, u) {
        yprp_revision(ctx, &submodp->revs[u]);
    }
    /* body-stmts */
    yang_print_parsed_body(ctx, (struct lysp_module *)submodp);

    LEVEL--;
    ly_print_(out, "%*s}\n", INDENT);
    ly_print_flush(out);

    return LY_SUCCESS;
}

LY_ERR
yang_print_compiled_node(struct ly_out *out, const struct lysc_node *node, uint32_t options)
{
    struct ypr_ctx ctx_ = {.out = out, .level = 0, .module = node->module, .options = options}, *ctx = &ctx_;

    yprc_node(ctx, node);

    ly_print_flush(out);
    return LY_SUCCESS;
}

LY_ERR
yang_print_compiled(struct ly_out *out, const struct lys_module *module, uint32_t options)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_node *data;
    struct lysc_module *modc = module->compiled;
    struct ypr_ctx ctx_ = {.out = out, .level = 0, .module = module, .options = options}, *ctx = &ctx_;

    ly_print_(ctx->out, "%*smodule %s {\n", INDENT, module->name);
    LEVEL++;

    /* module-header-stmts */
    ypr_substmt(ctx, LYEXT_SUBSTMT_NAMESPACE, 0, module->ns, modc->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_PREFIX, 0, module->prefix, modc->exts);

    /* no linkage-stmts */

    /* meta-stmts */
    if (module->org || module->contact || module->dsc || module->ref) {
        ly_print_(out, "\n");
    }
    ypr_substmt(ctx, LYEXT_SUBSTMT_ORGANIZATION, 0, module->org, modc->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_CONTACT, 0, module->contact, modc->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, module->dsc, modc->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, module->ref, modc->exts);

    /* revision-stmts */
    if (module->revision) {
        ly_print_(ctx->out, "\n%*srevision %s;\n", INDENT, module->revision);
    }

    /* body-stmts */
    if (modc->exts) {
        ly_print_(out, "\n");
        yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, module->compiled->exts, NULL, 0);
    }

    LY_ARRAY_FOR(module->features, u) {
        yprc_feature(ctx, &module->features[u]);
    }

    LY_ARRAY_FOR(module->identities, u) {
        yprc_identity(ctx, &module->identities[u]);
    }

    if (!(ctx->options & LYS_PRINT_NO_SUBSTMT)) {
        LY_LIST_FOR(modc->data, data) {
            yprc_node(ctx, data);
        }

        LY_ARRAY_FOR(modc->rpcs, u) {
            yprc_action(ctx, &modc->rpcs[u]);
        }

        LY_ARRAY_FOR(modc->notifs, u) {
            yprc_notification(ctx, &modc->notifs[u]);
        }
    }

    LEVEL--;
    ly_print_(out, "%*s}\n", INDENT);
    ly_print_flush(out);

    return LY_SUCCESS;
}
