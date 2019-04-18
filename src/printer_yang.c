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

#include "common.h"

#include "printer_internal.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

#define LEVEL ctx->level
#define INDENT (LEVEL)*2,""

struct ypr_ctx {
    struct lyout *out;
    unsigned int level;
    const struct lys_module *module;
};

static void
ypr_encode(struct lyout *out, const char *text, int len)
{
    int i, start_len;
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
    for (i = 0; i < len; ++i) {
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
            ly_write(out, start, start_len);
            switch (special) {
            case '\n':
                ly_write(out, "\\n", 2);
                break;
            case '\t':
                ly_write(out, "\\t", 2);
                break;
            case '\"':
                ly_write(out, "\\\"", 2);
                break;
            case '\\':
                ly_write(out, "\\\\", 2);
                break;
            }

            start += start_len + 1;
            start_len = 0;

            special = 0;
        }
    }

    ly_write(out, start, start_len);
}

static void
ypr_open(struct lyout *out, int *flag)
{
    if (flag && !*flag) {
        *flag = 1;
        ly_print(out, " {\n");
    }
}

static void
ypr_close(struct ypr_ctx *ctx, int flag)
{
    if (flag) {
        ly_print(ctx->out, "%*s}\n", INDENT);
    } else {
        ly_print(ctx->out, ";\n");
    }
}

static void
ypr_text(struct ypr_ctx *ctx, const char *name, const char *text, int singleline, int closed)
{
    const char *s, *t;

    if (singleline) {
        ly_print(ctx->out, "%*s%s \"", INDENT, name);
    } else {
        ly_print(ctx->out, "%*s%s\n", INDENT, name);
        LEVEL++;

        ly_print(ctx->out, "%*s\"", INDENT);
    }
    t = text;
    while ((s = strchr(t, '\n'))) {
        ypr_encode(ctx->out, t, s - t);
        ly_print(ctx->out, "\n");
        t = s + 1;
        if (*t != '\n') {
            ly_print(ctx->out, "%*s ", INDENT);
        }
    }

    ypr_encode(ctx->out, t, strlen(t));
    if (closed) {
        ly_print(ctx->out, "\";\n");
    } else {
        ly_print(ctx->out, "\"");
    }
    if (!singleline) {
        LEVEL--;
    }
}

static void
ypr_parsed_stmt(struct ypr_ctx *ctx, struct lysp_stmt *stmt)
{
    struct lysp_stmt *childstmt;
    const char *s, *t;

    if (stmt->arg) {
        if (stmt->flags) {
            ly_print(ctx->out, "%*s%s\n", INDENT, stmt->stmt);
            LEVEL++;
            ly_print(ctx->out, "%*s%c", INDENT, (stmt->flags & LYS_DOUBLEQUOTED) ? '\"' : '\'');
            t = stmt->arg;
            while ((s = strchr(t, '\n'))) {
                ypr_encode(ctx->out, t, s - t);
                ly_print(ctx->out, "\n");
                t = s + 1;
                if (*t != '\n') {
                    ly_print(ctx->out, "%*s ", INDENT);
                }
            }
            LEVEL--;
            ypr_encode(ctx->out, t, strlen(t));
            ly_print(ctx->out, "%c%s", (stmt->flags & LYS_DOUBLEQUOTED) ? '\"' : '\'', stmt->child ? " {\n" : ";\n");
        } else {
            ly_print(ctx->out, "%*s%s %s%s", INDENT, stmt->stmt, stmt->arg, stmt->child ? " {\n" : ";\n");
        }
    } else {
        ly_print(ctx->out, "%*s%s%s", INDENT, stmt->stmt, stmt->child ? " {\n" : ";\n");
    }

    if (stmt->child) {
        LEVEL++;
        LY_LIST_FOR(stmt->child, childstmt) {
            ypr_parsed_stmt(ctx, childstmt);
        }
        LEVEL--;
        ly_print(ctx->out, "%*s}\n", INDENT);
    }
}

/**
 * @param[in] count Number of extensions to print, 0 to print them all.
 */
static void
ypr_parsed_extension_instances(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index,
                               struct lysp_ext_instance *ext, int *flag, unsigned int count)
{
    unsigned int u;
    struct lysp_stmt *stmt;

    if (!count && ext) {
        count = LY_ARRAY_SIZE(ext);
    }
    LY_ARRAY_FOR(ext, u) {
        if (!count) {
            break;
        }
        if (ext->insubstmt == substmt && ext->insubstmt_index == substmt_index) {
            ypr_open(ctx->out, flag);
            if (ext[u].argument) {
                ly_print(ctx->out, "%*s%s %s%s", INDENT, ext[u].name, ext[u].argument, ext[u].child ? " {\n" : ";\n");
            } else {
                ly_print(ctx->out, "%*s%s%s", INDENT, ext[u].name, ext[u].child ? " {\n" : ";\n");
            }

            if (ext[u].child) {
                LEVEL++;
                LY_LIST_FOR(ext[u].child, stmt) {
                    ypr_parsed_stmt(ctx, stmt);
                }
                LEVEL--;
                ly_print(ctx->out, "%*s}\n", INDENT);
            }
        }
        count--;
    }
}

static void
ypr_parsed_substmt(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index, const char *text, struct lysp_ext_instance *ext)
{
    unsigned int u;
    int extflag = 0;

    if (!text) {
        /* nothing to print */
        return;
    }

    if (ext_substmt_info[substmt].flags & SUBST_FLAG_ID) {
        ly_print(ctx->out, "%*s%s %s", INDENT, ext_substmt_info[substmt].name, text);
    } else {
        ypr_text(ctx, ext_substmt_info[substmt].name, text,
                 (ext_substmt_info[substmt].flags & SUBST_FLAG_YIN) ? 0 : 1, 0);
    }

    LEVEL++;
    LY_ARRAY_FOR(ext, u) {
        if (ext[u].insubstmt != substmt || ext[u].insubstmt_index != substmt_index) {
            continue;
        }
        ypr_parsed_extension_instances(ctx, substmt, substmt_index, &ext[u], &extflag, 1);
    }
    LEVEL--;
    ypr_close(ctx, extflag);
}

static void
ypr_parsed_unsigned(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index, struct lysp_ext_instance *exts,
                    unsigned int attr_value, int *flag)
{
    char *str;

    if (asprintf(&str, "%u", attr_value) == -1) {
        LOGMEM(ctx->module->ctx);
        return;
    }
    ypr_open(ctx->out, flag);
    ypr_parsed_substmt(ctx, substmt, substmt_index, str, exts);
    free(str);
}

static void
ypr_parsed_signed(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index, struct lysp_ext_instance *exts,
                  signed int attr_value, int *flag)
{
    char *str;

    if (asprintf(&str, "%d", attr_value) == -1) {
        LOGMEM(ctx->module->ctx);
        return;
    }
    ypr_open(ctx->out, flag);
    ypr_parsed_substmt(ctx, substmt, substmt_index, str, exts);
    free(str);
}

static void
ypr_parsed_revision(struct ypr_ctx *ctx, const struct lysp_revision *rev)
{
    if (rev->dsc || rev->ref || rev->exts) {
        ly_print(ctx->out, "%*srevision %s {\n", INDENT, rev->date);
        LEVEL++;
        ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, rev->exts, NULL, 0);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, rev->dsc, rev->exts);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, rev->ref, rev->exts);
        LEVEL--;
        ly_print(ctx->out, "%*s}\n", INDENT);
    } else {
        ly_print(ctx->out, "%*srevision %s;\n", INDENT, rev->date);
    }
}

static void
ypr_parsed_mandatory(struct ypr_ctx *ctx, uint16_t flags, struct lysp_ext_instance *exts, int *flag)
{
    if (flags & LYS_MAND_MASK) {
        ypr_open(ctx->out, flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_MANDATORY, 0, (flags & LYS_MAND_TRUE) ? "true" : "false", exts);
    }
}

static void
ypr_parsed_config(struct ypr_ctx *ctx, uint16_t flags, struct lysp_ext_instance *exts, int *flag)
{
    if (flags & LYS_CONFIG_MASK) {
        ypr_open(ctx->out, flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_CONFIG, 0, (flags & LYS_CONFIG_W) ? "true" : "false", exts);
    }
}

static void
ypr_parsed_status(struct ypr_ctx *ctx, uint16_t flags, struct lysp_ext_instance *exts, int *flag)
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
    ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_STATUS, 0, status, exts);
}

static void
ypr_description(struct ypr_ctx *ctx, const char *dsc, struct lysp_ext_instance *exts, int *flag)
{
    if (dsc) {
        ypr_open(ctx->out, flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, dsc, exts);
    }
}

static void
ypr_reference(struct ypr_ctx *ctx, const char *ref, struct lysp_ext_instance *exts, int *flag)
{
    if (ref) {
        ypr_open(ctx->out, flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, ref, exts);
    }
}

static void
ypr_parsed_iffeatures(struct ypr_ctx *ctx, const char **iff, struct lysp_ext_instance *exts, int *flag)
{
    unsigned int u;
    int extflag;

    LY_ARRAY_FOR(iff, u) {
        ypr_open(ctx->out, flag);
        extflag = 0;

        ly_print(ctx->out, "%*sif-feature \"%s\"", INDENT, iff[u]);

        /* extensions */
        LEVEL++;
        LY_ARRAY_FOR(exts, u) {
            if (exts[u].insubstmt != LYEXT_SUBSTMT_IFFEATURE || exts[u].insubstmt_index != u) {
                continue;
            }
            ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_IFFEATURE, u, &exts[u], &extflag, 1);
        }
        LEVEL--;
        ypr_close(ctx, extflag);
    }
}

static void
ypr_parsed_extension(struct ypr_ctx *ctx, const struct lysp_ext *ext)
{
    int flag = 0, flag2 = 0;
    unsigned int i;

    ly_print(ctx->out, "%*sextension %s", INDENT, ext->name);
    LEVEL++;

    if (ext->exts) {
        ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, ext->exts, &flag, 0);
    }

    if (ext->argument) {
        ypr_open(ctx->out, &flag);
        ly_print(ctx->out, "%*sargument %s", INDENT, ext->argument);
        if (ext->exts) {
            LEVEL++;
            i = -1;
            while ((i = lysp_ext_instance_iter(ext->exts, i + 1, LYEXT_SUBSTMT_ARGUMENT)) != LY_ARRAY_SIZE(ext->exts)) {
                ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_ARGUMENT, 0, &ext->exts[i], &flag2, 1);
            }
            LEVEL--;
        }
        if ((ext->flags & LYS_YINELEM_MASK) ||
                (ext->exts && lysp_ext_instance_iter(ext->exts, 0, LYEXT_SUBSTMT_YINELEM) != LY_ARRAY_SIZE(ext->exts))) {
            ypr_open(ctx->out, &flag2);
            ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_YINELEM, 0, (ext->flags & LYS_YINELEM_TRUE) ? "true" : "false", ext->exts);
        }
        ypr_close(ctx, flag2);
    }

    ypr_parsed_status(ctx, ext->flags, ext->exts, &flag);
    ypr_description(ctx, ext->dsc, ext->exts, &flag);
    ypr_reference(ctx, ext->ref, ext->exts, &flag);

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
ypr_parsed_feature(struct ypr_ctx *ctx, const struct lysp_feature *feat)
{
    int flag = 0;

    ly_print(ctx->out, "\n%*sfeature %s", INDENT, feat->name);
    LEVEL++;
    ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, feat->exts, &flag, 0);
    ypr_parsed_iffeatures(ctx, feat->iffeatures, feat->exts, &flag);
    ypr_parsed_status(ctx, feat->flags, feat->exts, &flag);
    ypr_description(ctx, feat->dsc, feat->exts, &flag);
    ypr_reference(ctx, feat->ref, feat->exts, &flag);
    LEVEL--;
    ypr_close(ctx, flag);
}

static void
ypr_parsed_identity(struct ypr_ctx *ctx, const struct lysp_ident *ident)
{
    int flag = 0;
    unsigned int u;

    ly_print(ctx->out, "\n%*sidentity %s", INDENT, ident->name);
    LEVEL++;

    ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, ident->exts, &flag, 0);
    ypr_parsed_iffeatures(ctx, ident->iffeatures, ident->exts, &flag);

    LY_ARRAY_FOR(ident->bases, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_BASE, u, ident->bases[u], ident->exts);
    }

    ypr_parsed_status(ctx, ident->flags, ident->exts, &flag);
    ypr_description(ctx, ident->dsc, ident->exts, &flag);
    ypr_reference(ctx, ident->ref, ident->exts, &flag);

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
ypr_parsed_restr(struct ypr_ctx *ctx, const struct lysp_restr *restr, const char *name, int *flag)
{
    int inner_flag = 0;

    if (!restr) {
        return;
    }

    ypr_open(ctx->out, flag);
    ly_print(ctx->out, "%*s%s \"", INDENT, name);
    ypr_encode(ctx->out, (restr->arg[0] != 0x15 && restr->arg[0] != 0x06) ? restr->arg : &restr->arg[1], -1);
    ly_print(ctx->out, "\"");

    LEVEL++;
    ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, restr->exts, &inner_flag, 0);
    if (restr->arg[0] == 0x15) {
        /* special byte value in pattern's expression: 0x15 - invert-match, 0x06 - match */
        ypr_open(ctx->out, &inner_flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_MODIFIER, 0, "invert-match", restr->exts);
    }
    if (restr->emsg) {
        ypr_open(ctx->out, &inner_flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_ERRMSG, 0, restr->emsg, restr->exts);
    }
    if (restr->eapptag) {
        ypr_open(ctx->out, &inner_flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_ERRTAG, 0, restr->eapptag, restr->exts);
    }
    if (restr->dsc != NULL) {
        ypr_open(ctx->out, &inner_flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, restr->dsc,restr->exts);
    }
    if (restr->ref != NULL) {
        ypr_open(ctx->out, &inner_flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, restr->ref, restr->exts);
    }
    LEVEL--;
    ypr_close(ctx, inner_flag);
}

static void
ypr_parsed_when(struct ypr_ctx *ctx, struct lysp_when *when, int *flag)
{
    int inner_flag = 0;

    if (!when) {
        return;
    }
    ypr_open(ctx->out, flag);

    ly_print(ctx->out, "%*swhen \"", INDENT);
    ypr_encode(ctx->out, when->cond, -1);
    ly_print(ctx->out, "\"");

    LEVEL++;
    ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, when->exts, &inner_flag, 0);
    ypr_description(ctx, when->dsc, when->exts, &inner_flag);
    ypr_reference(ctx, when->ref, when->exts, &inner_flag);
    LEVEL--;
    ypr_close(ctx, inner_flag);
}

static void
ypr_parsed_enum(struct ypr_ctx *ctx, const struct lysp_type_enum *items, LY_DATA_TYPE type, int *flag)
{
    unsigned int u;
    int inner_flag;

    LY_ARRAY_FOR(items, u) {
        ypr_open(ctx->out, flag);
        ly_print(ctx->out, "%*s%s \"", INDENT, type == LY_TYPE_BITS ? "bit" : "enum");
        ypr_encode(ctx->out, items[u].name, -1);
        ly_print(ctx->out, "\"");
        inner_flag = 0;
        LEVEL++;
        ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, items[u].exts, &inner_flag, 0);
        ypr_parsed_iffeatures(ctx, items[u].iffeatures, items[u].exts, &inner_flag);
        if (items[u].flags & LYS_SET_VALUE) {
            if (type == LY_TYPE_BITS) {
                ypr_parsed_unsigned(ctx, LYEXT_SUBSTMT_POSITION, 0, items[u].exts, items[u].value, &inner_flag);
            } else { /* LY_TYPE_ENUM */
                ypr_parsed_signed(ctx, LYEXT_SUBSTMT_VALUE, 0, items[u].exts, items[u].value, &inner_flag);
            }
        }
        ypr_parsed_status(ctx, items[u].flags, items[u].exts, &inner_flag);
        ypr_description(ctx, items[u].dsc, items[u].exts, &inner_flag);
        ypr_reference(ctx, items[u].ref, items[u].exts, &inner_flag);
        LEVEL--;
        ypr_close(ctx, inner_flag);
    }
}

static void
ypr_parsed_type(struct ypr_ctx *ctx, const struct lysp_type *type)
{
    unsigned int u;
    int flag = 0;

    ly_print(ctx->out, "%*stype %s", INDENT, type->name);
    LEVEL++;

    ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, type->exts, &flag, 0);

    ypr_parsed_restr(ctx, type->range, "range", &flag);
    ypr_parsed_restr(ctx, type->length, "length", &flag);
    LY_ARRAY_FOR(type->patterns, u) {
        ypr_parsed_restr(ctx, &type->patterns[u], "pattern", &flag);
    }
    ypr_parsed_enum(ctx, type->bits, LY_TYPE_BITS, &flag);
    ypr_parsed_enum(ctx, type->enums, LY_TYPE_ENUM, &flag);

    if (type->path) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_PATH, 0, type->path, type->exts);
    }
    if (type->flags & LYS_SET_REQINST) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_REQINSTANCE, 0, type->require_instance ? "true" : "false", type->exts);
    }
    if (type->flags & LYS_SET_FRDIGITS) {
        ypr_parsed_unsigned(ctx, LYEXT_SUBSTMT_FRACDIGITS, 0, type->exts, type->fraction_digits, &flag);
    }
    LY_ARRAY_FOR(type->bases, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_BASE, u, type->bases[u], type->exts);
    }
    LY_ARRAY_FOR(type->types, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_type(ctx, &type->types[u]);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
ypr_parsed_typedef(struct ypr_ctx *ctx, const struct lysp_tpdf *tpdf)
{
    ly_print(ctx->out, "\n%*stypedef %s {\n", INDENT, tpdf->name);
    LEVEL++;

    ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, tpdf->exts, NULL, 0);

    ypr_parsed_type(ctx, &tpdf->type);

    if (tpdf->units) {
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, tpdf->units, tpdf->exts);
    }
    if (tpdf->dflt) {
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, tpdf->dflt, tpdf->exts);
    }

    ypr_parsed_status(ctx, tpdf->flags, tpdf->exts, NULL);
    ypr_description(ctx, tpdf->dsc, tpdf->exts, NULL);
    ypr_reference(ctx, tpdf->ref, tpdf->exts, NULL);

    LEVEL--;
    ly_print(ctx->out, "%*s}\n", INDENT);
}

static void ypr_parsed_node(struct ypr_ctx *ctx, const struct lysp_node *node);
static void ypr_parsed_action(struct ypr_ctx *ctx, const struct lysp_action *action);

static void
ypr_parsed_grouping(struct ypr_ctx *ctx, const struct lysp_grp *grp)
{
    unsigned int u;
    int flag = 0;
    struct lysp_node *data;

    ly_print(ctx->out, "\n%*sgrouping %s", INDENT, grp->name);
    LEVEL++;

    ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, grp->exts, &flag, 0);
    ypr_parsed_status(ctx, grp->flags, grp->exts, &flag);
    ypr_description(ctx, grp->dsc, grp->exts, &flag);
    ypr_reference(ctx, grp->ref, grp->exts, &flag);

    LY_ARRAY_FOR(grp->typedefs, u) {
        ypr_parsed_typedef(ctx, &grp->typedefs[u]);
    }

    LY_ARRAY_FOR(grp->groupings, u) {
        ypr_parsed_grouping(ctx, &grp->groupings[u]);
    }

    LY_LIST_FOR(grp->data, data) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_node(ctx, data);
    }

    LY_ARRAY_FOR(grp->actions, u) {
        ypr_parsed_action(ctx, &grp->actions[u]);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
ypr_parsed_inout(struct ypr_ctx *ctx, const struct lysp_action_inout *inout, int *flag)
{
    unsigned int u;
    struct lysp_node *data;

    if (!inout->nodetype) {
        /* nodetype not set -> input/output is empty */
        return;
    }
    ypr_open(ctx->out, flag);

    ly_print(ctx->out, "\n%*s%s {\n", INDENT, (inout->nodetype == LYS_INPUT ? "input" : "output"));
    LEVEL++;

    ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, inout->exts, NULL, 0);
    LY_ARRAY_FOR(inout->musts, u) {
        ypr_parsed_restr(ctx, &inout->musts[u], "must", NULL);
    }
    LY_ARRAY_FOR(inout->typedefs, u) {
        ypr_parsed_typedef(ctx, &inout->typedefs[u]);
    }
    LY_ARRAY_FOR(inout->groupings, u) {
        ypr_parsed_grouping(ctx, &inout->groupings[u]);
    }

    LY_LIST_FOR(inout->data, data) {
        ypr_parsed_node(ctx, data);
    }

    LEVEL--;
    ypr_close(ctx, 1);
}

static void
ypr_parsed_notification(struct ypr_ctx *ctx, const struct lysp_notif *notif)
{
    unsigned int u;
    int flag = 0;
    struct lysp_node *data;

    ly_print(ctx->out, "%*snotification %s", INDENT, notif->name);

    LEVEL++;
    ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, notif->exts, &flag, 0);
    ypr_parsed_iffeatures(ctx, notif->iffeatures, notif->exts, &flag);

    LY_ARRAY_FOR(notif->musts, u) {
        ypr_parsed_restr(ctx, &notif->musts[u], "must", &flag);
    }
    ypr_parsed_status(ctx, notif->flags, notif->exts, &flag);
    ypr_description(ctx, notif->dsc, notif->exts, &flag);
    ypr_reference(ctx, notif->ref, notif->exts, &flag);

    LY_ARRAY_FOR(notif->typedefs, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_typedef(ctx, &notif->typedefs[u]);
    }

    LY_ARRAY_FOR(notif->groupings, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_grouping(ctx, &notif->groupings[u]);
    }

    LY_LIST_FOR(notif->data, data) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_node(ctx, data);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
ypr_parsed_action(struct ypr_ctx *ctx, const struct lysp_action *action)
{
    unsigned int u;
    int flag = 0;

    ly_print(ctx->out, "%*s%s %s", INDENT, action->parent ? "action" : "rpc", action->name);

    LEVEL++;
    ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, action->exts, &flag, 0);
    ypr_parsed_iffeatures(ctx, action->iffeatures, action->exts, &flag);
    ypr_parsed_status(ctx, action->flags, action->exts, &flag);
    ypr_description(ctx, action->dsc, action->exts, &flag);
    ypr_reference(ctx, action->ref, action->exts, &flag);

    LY_ARRAY_FOR(action->typedefs, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_typedef(ctx, &action->typedefs[u]);
    }

    LY_ARRAY_FOR(action->groupings, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_grouping(ctx, &action->groupings[u]);
    }

    ypr_parsed_inout(ctx, &action->input, &flag);
    ypr_parsed_inout(ctx, &action->output, &flag);

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
ypr_parsed_node_common1(struct ypr_ctx *ctx, const struct lysp_node *node, int *flag)
{
    ly_print(ctx->out, "\n%*s%s %s%s", INDENT, lys_nodetype2str(node->nodetype), node->name, flag ? "" : " {\n");
    LEVEL++;

    ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, node->exts, flag, 0);
    ypr_parsed_when(ctx, node->when, flag);
    ypr_parsed_iffeatures(ctx, node->iffeatures, node->exts, flag);
}

static void
ypr_parsed_node_common2(struct ypr_ctx *ctx, const struct lysp_node *node, int *flag)
{
    ypr_parsed_config(ctx, node->flags, node->exts, flag);
    if (node->nodetype & (LYS_CHOICE | LYS_LEAF | LYS_ANYDATA)) {
        ypr_parsed_mandatory(ctx, node->flags, node->exts, flag);
    }
    ypr_parsed_status(ctx, node->flags, node->exts, flag);
    ypr_description(ctx, node->dsc, node->exts, flag);
    ypr_reference(ctx, node->ref, node->exts, flag);

}

static void
ypr_parsed_container(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    unsigned int u;
    int flag = 0;
    struct lysp_node *child;
    struct lysp_node_container *cont = (struct lysp_node_container *)node;

    ypr_parsed_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(cont->musts, u) {
        ypr_parsed_restr(ctx, &cont->musts[u], "must", &flag);
    }
    if (cont->presence) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_PRESENCE, 0, cont->presence, cont->exts);
    }

    ypr_parsed_node_common2(ctx, node, &flag);

    LY_ARRAY_FOR(cont->typedefs, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_typedef(ctx, &cont->typedefs[u]);
    }

    LY_ARRAY_FOR(cont->groupings, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_grouping(ctx, &cont->groupings[u]);
    }

    LY_LIST_FOR(cont->child, child) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_node(ctx, child);
    }

    LY_ARRAY_FOR(cont->actions, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_action(ctx, &cont->actions[u]);
    }

    LY_ARRAY_FOR(cont->notifs, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_notification(ctx, &cont->notifs[u]);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
ypr_parsed_choice(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    int flag = 0;
    struct lysp_node *child;
    struct lysp_node_choice *choice = (struct lysp_node_choice *)node;

    ypr_parsed_node_common1(ctx, node, &flag);

    if (choice->dflt) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, choice->dflt, choice->exts);
    }

    ypr_parsed_node_common2(ctx, node, &flag);

    LY_LIST_FOR(choice->child, child) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_node(ctx, child);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
ypr_parsed_leaf(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    unsigned int u;
    struct lysp_node_leaf *leaf = (struct lysp_node_leaf *)node;

    ypr_parsed_node_common1(ctx, node, NULL);

    ypr_parsed_type(ctx, &leaf->type);
    ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, leaf->units, leaf->exts);
    LY_ARRAY_FOR(leaf->musts, u) {
        ypr_parsed_restr(ctx, &leaf->musts[u], "must", NULL);
    }
    ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, leaf->dflt, leaf->exts);

    ypr_parsed_node_common2(ctx, node, NULL);

    LEVEL--;
    ly_print(ctx->out, "%*s}\n", INDENT);
}

static void
ypr_parsed_leaflist(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    unsigned int u;
    struct lysp_node_leaflist *llist = (struct lysp_node_leaflist *)node;

    ypr_parsed_node_common1(ctx, node, NULL);

    ypr_parsed_type(ctx, &llist->type);
    ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, llist->units, llist->exts);
    LY_ARRAY_FOR(llist->musts, u) {
        ypr_parsed_restr(ctx, &llist->musts[u], "must", NULL);
    }
    LY_ARRAY_FOR(llist->dflts, u) {
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, u, llist->dflts[u], llist->exts);
    }

    ypr_parsed_config(ctx, node->flags, node->exts, NULL);

    if (llist->flags & LYS_SET_MIN) {
        ypr_parsed_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, llist->exts, llist->min, NULL);
    }
    if (llist->flags & LYS_SET_MAX) {
        if (llist->max) {
            ypr_parsed_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, llist->exts, llist->max, NULL);
        } else {
            ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", llist->exts);
        }
    }

    if (llist->flags & LYS_ORDBY_MASK) {
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_ORDEREDBY, 0, (llist->flags & LYS_ORDBY_USER) ? "user" : "system", llist->exts);
    }

    ypr_parsed_status(ctx, node->flags, node->exts, NULL);
    ypr_description(ctx, node->dsc, node->exts, NULL);
    ypr_reference(ctx, node->ref, node->exts, NULL);

    LEVEL--;
    ly_print(ctx->out, "%*s}\n", INDENT);
}

static void
ypr_parsed_list(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    unsigned int u;
    int flag = 0;
    struct lysp_node *child;
    struct lysp_node_list *list = (struct lysp_node_list *)node;

    ypr_parsed_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(list->musts, u) {
        ypr_parsed_restr(ctx, &list->musts[u], "must", NULL);
    }
    if (list->key) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_KEY, 0, list->key, list->exts);
    }
    LY_ARRAY_FOR(list->uniques, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_UNIQUE, u, list->uniques[u], list->exts);
    }

    ypr_parsed_config(ctx, node->flags, node->exts, NULL);

    if (list->flags & LYS_SET_MIN) {
        ypr_parsed_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, list->exts, list->min, NULL);
    }
    if (list->flags & LYS_SET_MAX) {
        if (list->max) {
            ypr_parsed_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, list->exts, list->max, NULL);
        } else {
            ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", list->exts);
        }
    }

    if (list->flags & LYS_ORDBY_MASK) {
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_ORDEREDBY, 0, (list->flags & LYS_ORDBY_USER) ? "user" : "system", list->exts);
    }

    ypr_parsed_status(ctx, node->flags, node->exts, NULL);
    ypr_description(ctx, node->dsc, node->exts, NULL);
    ypr_reference(ctx, node->ref, node->exts, NULL);

    LY_ARRAY_FOR(list->typedefs, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_typedef(ctx, &list->typedefs[u]);
    }

    LY_ARRAY_FOR(list->groupings, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_grouping(ctx, &list->groupings[u]);
    }

    LY_LIST_FOR(list->child, child) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_node(ctx, child);
    }

    LY_ARRAY_FOR(list->actions, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_action(ctx, &list->actions[u]);
    }

    LY_ARRAY_FOR(list->notifs, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_notification(ctx, &list->notifs[u]);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
ypr_parsed_refine(struct ypr_ctx *ctx, struct lysp_refine *refine)
{
    unsigned int u;
    int flag = 0;

    ly_print(ctx->out, "%*srefine \"%s\"", INDENT, refine->nodeid);
    LEVEL++;

    ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, refine->exts, &flag, 0);
    ypr_parsed_iffeatures(ctx, refine->iffeatures, refine->exts, &flag);

    LY_ARRAY_FOR(refine->musts, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_restr(ctx, &refine->musts[u], "must", NULL);
    }

    if (refine->presence) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_PRESENCE, 0, refine->presence, refine->exts);
    }

    LY_ARRAY_FOR(refine->dflts, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, u, refine->dflts[u], refine->exts);
    }

    ypr_parsed_config(ctx, refine->flags, refine->exts, &flag);
    ypr_parsed_mandatory(ctx, refine->flags, refine->exts, &flag);

    if (refine->flags & LYS_SET_MIN) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, refine->exts, refine->min, NULL);
    }
    if (refine->flags & LYS_SET_MAX) {
        ypr_open(ctx->out, &flag);
        if (refine->max) {
            ypr_parsed_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, refine->exts, refine->max, NULL);
        } else {
            ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", refine->exts);
        }
    }

    ypr_description(ctx, refine->dsc, refine->exts, &flag);
    ypr_reference(ctx, refine->ref, refine->exts, &flag);

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
ypr_parsed_augment(struct ypr_ctx *ctx, const struct lysp_augment *aug)
{
    unsigned int u;
    struct lysp_node *child;

    ly_print(ctx->out, "%*saugment \"%s\" {\n", INDENT, aug->nodeid);
    LEVEL++;

    ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, aug->exts, NULL, 0);
    ypr_parsed_when(ctx, aug->when, NULL);
    ypr_parsed_iffeatures(ctx, aug->iffeatures, aug->exts, NULL);
    ypr_parsed_status(ctx, aug->flags, aug->exts, NULL);
    ypr_description(ctx, aug->dsc, aug->exts, NULL);
    ypr_reference(ctx, aug->ref, aug->exts, NULL);

    LY_LIST_FOR(aug->child, child) {
        ypr_parsed_node(ctx, child);
    }

    LY_ARRAY_FOR(aug->actions, u) {
        ypr_parsed_action(ctx, &aug->actions[u]);
    }

    LY_ARRAY_FOR(aug->notifs, u) {
        ypr_parsed_notification(ctx, &aug->notifs[u]);
    }

    LEVEL--;
    ypr_close(ctx, 1);
}


static void
ypr_parsed_uses(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    unsigned int u;
    int flag = 0;
    struct lysp_node_uses *uses = (struct lysp_node_uses *)node;

    ypr_parsed_node_common1(ctx, node, &flag);
    ypr_parsed_node_common2(ctx, node, &flag);

    LY_ARRAY_FOR(uses->refines, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_refine(ctx, &uses->refines[u]);
    }

    LY_ARRAY_FOR(uses->augments, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_augment(ctx, &uses->augments[u]);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
ypr_parsed_anydata(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    unsigned int u;
    int flag = 0;
    struct lysp_node_anydata *any = (struct lysp_node_anydata *)node;

    ypr_parsed_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(any->musts, u) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_restr(ctx, &any->musts[u], "must", NULL);
    }

    ypr_parsed_node_common2(ctx, node, &flag);

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
ypr_parsed_case(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    int flag = 0;
    struct lysp_node *child;
    struct lysp_node_case *cas = (struct lysp_node_case *)node;

    ypr_parsed_node_common1(ctx, node, &flag);
    ypr_parsed_node_common2(ctx, node, &flag);

    LY_LIST_FOR(cas->child, child) {
        ypr_open(ctx->out, &flag);
        ypr_parsed_node(ctx, child);
    }

    LEVEL--;
    ypr_close(ctx, flag);
}

static void
ypr_parsed_node(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    switch (node->nodetype) {
    case LYS_CONTAINER:
        ypr_parsed_container(ctx, node);
        break;
    case LYS_CHOICE:
        ypr_parsed_choice(ctx, node);
        break;
    case LYS_LEAF:
        ypr_parsed_leaf(ctx, node);
        break;
    case LYS_LEAFLIST:
        ypr_parsed_leaflist(ctx, node);
        break;
    case LYS_LIST:
        ypr_parsed_list(ctx, node);
        break;
    case LYS_USES:
        ypr_parsed_uses(ctx, node);
        break;
    case LYS_ANYXML:
    case LYS_ANYDATA:
        ypr_parsed_anydata(ctx, node);
        break;
    case LYS_CASE:
        ypr_parsed_case(ctx, node);
        break;
    default:
        break;
    }
}

static void
ypr_parsed_deviation(struct ypr_ctx *ctx, const struct lysp_deviation *deviation)
{
    unsigned int u, v;
    struct lysp_deviate_add *add;
    struct lysp_deviate_rpl *rpl;
    struct lysp_deviate_del *del;

    ly_print(ctx->out, "%*sdeviation \"%s\" {\n", INDENT, deviation->nodeid);
    LEVEL++;

    ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, deviation->exts, NULL, 0);
    ypr_description(ctx, deviation->dsc, deviation->exts, NULL);
    ypr_reference(ctx, deviation->ref, deviation->exts, NULL);

    LY_ARRAY_FOR(deviation->deviates, u) {
        ly_print(ctx->out, "%*sdeviate ", INDENT);
        if (deviation->deviates[u].mod == LYS_DEV_NOT_SUPPORTED) {
            if (deviation->deviates[u].exts) {
                ly_print(ctx->out, "not-supported {\n");
                LEVEL++;

                ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, deviation->deviates[u].exts, NULL, 0);
            } else {
                ly_print(ctx->out, "not-supported;\n");
                continue;
            }
        } else if (deviation->deviates[u].mod == LYS_DEV_ADD) {
            add = (struct lysp_deviate_add*)&deviation->deviates[u];
            ly_print(ctx->out, "add {\n");
            LEVEL++;

            ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, add->exts, NULL, 0);
            ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, add->units, add->exts);
            LY_ARRAY_FOR(add->musts, v) {
                ypr_parsed_restr(ctx, &add->musts[v], "must", NULL);
            }
            LY_ARRAY_FOR(add->uniques, v) {
                ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_UNIQUE, v, add->uniques[v], add->exts);
            }
            LY_ARRAY_FOR(add->dflts, v) {
                ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, v, add->dflts[v], add->exts);
            }
            ypr_parsed_config(ctx, add->flags, add->exts, NULL);
            ypr_parsed_mandatory(ctx, add->flags, add->exts, NULL);
            if (add->flags & LYS_SET_MIN) {
                ypr_parsed_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, add->exts, add->min, NULL);
            }
            if (add->flags & LYS_SET_MAX) {
                if (add->max) {
                    ypr_parsed_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, add->exts, add->max, NULL);
                } else {
                    ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", add->exts);
                }
            }
        } else if (deviation->deviates[u].mod == LYS_DEV_REPLACE) {
            rpl = (struct lysp_deviate_rpl*)&deviation->deviates[u];
            ly_print(ctx->out, "replace {\n");
            LEVEL++;

            ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, rpl->exts, NULL, 0);
            if (rpl->type) {
                ypr_parsed_type(ctx, rpl->type);
            }
            ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, rpl->units, rpl->exts);
            ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, rpl->dflt, rpl->exts);
            ypr_parsed_config(ctx, rpl->flags, rpl->exts, NULL);
            ypr_parsed_mandatory(ctx, rpl->flags, rpl->exts, NULL);
            if (rpl->flags & LYS_SET_MIN) {
                ypr_parsed_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, rpl->exts, rpl->min, NULL);
            }
            if (rpl->flags & LYS_SET_MAX) {
                if (rpl->max) {
                    ypr_parsed_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, rpl->exts, rpl->max, NULL);
                } else {
                    ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", rpl->exts);
                }
            }
        } else if (deviation->deviates[u].mod == LYS_DEV_DELETE) {
            del = (struct lysp_deviate_del*)&deviation->deviates[u];
            ly_print(ctx->out, "delete {\n");
            LEVEL++;

            ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, del->exts, NULL, 0);
            ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, del->units, del->exts);
            LY_ARRAY_FOR(del->musts, v) {
                ypr_parsed_restr(ctx, &del->musts[v], "must", NULL);
            }
            LY_ARRAY_FOR(del->uniques, v) {
                ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_UNIQUE, v, del->uniques[v], del->exts);
            }
            LY_ARRAY_FOR(del->dflts, v) {
                ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, v, del->dflts[v], del->exts);
            }
        }

        LEVEL--;
        ypr_close(ctx, 1);
    }

    LEVEL--;
    ypr_close(ctx, 1);
}

LY_ERR
yang_print_parsed(struct lyout *out, const struct lys_module *module)
{
    unsigned int u;
    struct lysp_node *data;
    struct lysp_module *modp = module->parsed;
    struct ypr_ctx ctx_ = {.out = out, .level = 0, .module = module}, *ctx = &ctx_;

    ly_print(ctx->out, "%*smodule %s {\n", INDENT, module->name);
    LEVEL++;

    /* module-header-stmts */
    if (module->version) {
        if (module->version) {
            ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_VERSION, 0, module->version == LYS_VERSION_1_1 ? "1.1" : "1", modp->exts);
        }
    }
    ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_NAMESPACE, 0, module->ns, modp->exts);
    ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_PREFIX, 0, module->prefix, modp->exts);

    /* linkage-stmts */
    LY_ARRAY_FOR(modp->imports, u) {
        ly_print(out, "\n%*simport %s {\n", INDENT, modp->imports[u].module->name);
        LEVEL++;
        ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, modp->imports[u].exts, NULL, 0);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_PREFIX, 0, modp->imports[u].prefix, modp->imports[u].exts);
        if (modp->imports[u].rev[0]) {
            ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_REVISIONDATE, 0, modp->imports[u].rev, modp->imports[u].exts);
        }
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, modp->imports[u].dsc, modp->imports[u].exts);
        ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, modp->imports[u].ref, modp->imports[u].exts);
        LEVEL--;
        ly_print(out, "%*s}\n", INDENT);
    }
    LY_ARRAY_FOR(modp->includes, u) {
        if (modp->includes[u].rev[0] || modp->includes[u].dsc || modp->includes[u].ref || modp->includes[u].exts) {
            ly_print(out, "\n%*sinclude %s {\n", INDENT, modp->includes[u].submodule->name);
            LEVEL++;
            ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, modp->includes[u].exts, NULL, 0);
            if (modp->includes[u].rev[0]) {
                ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_REVISIONDATE, 0, modp->includes[u].rev, modp->includes[u].exts);
            }
            ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, modp->includes[u].dsc, modp->includes[u].exts);
            ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, modp->includes[u].ref, modp->includes[u].exts);
            LEVEL--;
            ly_print(out, "%*s}\n", INDENT);
        } else {
            ly_print(out, "\n%*sinclude \"%s\";\n", INDENT, modp->includes[u].submodule->name);
        }
    }

    /* meta-stmts */
    if (module->org || module->contact || module->dsc || module->ref) {
        ly_print(out, "\n");
    }
    ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_ORGANIZATION, 0, module->org, modp->exts);
    ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_CONTACT, 0, module->contact, modp->exts);
    ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, module->dsc, modp->exts);
    ypr_parsed_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, module->ref, modp->exts);

    /* revision-stmts */
    if (modp->revs) {
        ly_print(out, "\n");
    }
    LY_ARRAY_FOR(modp->revs, u) {
        ypr_parsed_revision(ctx, &modp->revs[u]);
    }
    /* body-stmts */
    LY_ARRAY_FOR(modp->extensions, u) {
        ly_print(out, "\n");
        ypr_parsed_extension(ctx, &modp->extensions[u]);
    }
    if (modp->exts) {
        ly_print(out, "\n");
        ypr_parsed_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, module->parsed->exts, NULL, 0);
    }

    LY_ARRAY_FOR(modp->features, u) {
        ypr_parsed_feature(ctx, &modp->features[u]);
    }

    LY_ARRAY_FOR(modp->identities, u) {
        ypr_parsed_identity(ctx, &modp->identities[u]);
    }

    LY_ARRAY_FOR(modp->typedefs, u) {
        ypr_parsed_typedef(ctx, &modp->typedefs[u]);
    }

    LY_ARRAY_FOR(modp->groupings, u) {
        ypr_parsed_grouping(ctx, &modp->groupings[u]);
    }

    LY_LIST_FOR(modp->data, data) {
        ypr_parsed_node(ctx, data);
    }

    LY_ARRAY_FOR(modp->augments, u) {
        ypr_parsed_augment(ctx, &modp->augments[u]);
    }

    LY_ARRAY_FOR(modp->rpcs, u) {
        ypr_parsed_action(ctx, &modp->rpcs[u]);
    }

    LY_ARRAY_FOR(modp->notifs, u) {
        ypr_parsed_notification(ctx, &modp->notifs[u]);
    }

    LY_ARRAY_FOR(modp->deviations, u) {
        ypr_parsed_deviation(ctx, &modp->deviations[u]);
    }

    LEVEL--;
    ly_print(out, "%*s}\n", INDENT);
    ly_print_flush(out);

    return LY_SUCCESS;
}

LY_ERR
yang_print_compiled(struct lyout *out, const struct lys_module *module)
{
    (void) out;
    (void) module;

    return LY_SUCCESS;
}
