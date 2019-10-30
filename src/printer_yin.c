/**
 * @file printer_yin.c
 * @author Fred Gan <ganshaolong@vip.qq.com>
 * @brief YIN printer
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

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "plugins_exts.h"
#include "printer_internal.h"
#include "tree.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "plugins_types.h"
#include "xpath.h"
#include "xml.h"


#define YIN_NS_URI "urn:ietf:params:xml:ns:yang:yin:1"

/**
 * @brief YIN printer context.
 */
struct ypr_ctx {
    struct lyout *out;               /**< output specification */
    unsigned int level;              /**< current indentation level: 0 - no formatting, >= 1 indentation levels */
    const struct lys_module *module; /**< schema to print */
    enum schema_type schema;         /**< type of the schema to print */
};

#define LEVEL ctx->level             /**< current level */
#define INDENT (LEVEL)*2,""          /**< indentation parameters for printer functions */

static void yprp_extension_instances(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index,
                               struct lysp_ext_instance *ext, int *flag, unsigned int count);
static void yprc_extension_instances(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index,
                                 struct lysc_ext_instance *ext, int *flag, unsigned int count);

static void
ypr_open(struct ypr_ctx *ctx, const char *elem_name, const char *attr_name, const char *attr_value,  int flag)
{
    ly_print(ctx->out, "%*s<%s", INDENT, elem_name);

    if (attr_name) {
        ly_print(ctx->out, " %s=\"", attr_name);
        lyxml_dump_text(ctx->out, attr_value, 1);
        ly_print(ctx->out, "\"%s", flag == -1 ? "/>\n" : flag == 1 ? ">\n" : "");
    } else if (flag) {
        ly_print(ctx->out, flag == -1 ? "/>\n" : ">\n");
    }
}

static void
ypr_close(struct ypr_ctx *ctx, const char *elem_name, int flag)
{
    if (flag) {
        ly_print(ctx->out, "%*s</%s>\n", INDENT, elem_name);
    } else {
        ly_print(ctx->out, "/>\n");
    }
}

/*
 * par_close_flag
 * 0 - parent not yet closed, printing >\n, setting flag to 1
 * 1 or NULL - parent already closed, do nothing
 */
static void
ypr_close_parent(struct ypr_ctx *ctx, int *par_close_flag)
{
    if (par_close_flag && !(*par_close_flag)) {
        (*par_close_flag) = 1;
        ly_print(ctx->out, ">\n");
    }
}

static void
ypr_yin_arg(struct ypr_ctx *ctx, const char *arg, const char *text)
{
    ly_print(ctx->out, "%*s<%s>", INDENT, arg);
    lyxml_dump_text(ctx->out, text, 0);
    ly_print(ctx->out, "</%s>\n", arg);
}



static void
ypr_substmt(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index, const char *text, void *ext)
{
    unsigned int u;
    int extflag = 0;

    if (!text) {
        /* nothing to print */
        return;
    }

    if (ext_substmt_info[substmt].flags & SUBST_FLAG_YIN) {
        extflag = 1;
        ypr_open(ctx, ext_substmt_info[substmt].name, NULL, NULL, extflag);
    } else {
        ypr_open(ctx, ext_substmt_info[substmt].name, ext_substmt_info[substmt].arg, text, extflag);
    }

    LEVEL++;
    LY_ARRAY_FOR(ext, u) {
        if (((struct lysp_ext_instance*)ext)[u].insubstmt != substmt || ((struct lysp_ext_instance*)ext)[u].insubstmt_index != substmt_index) {
            continue;
        }
        if (ctx->schema == YPR_PARSED) {
            yprp_extension_instances(ctx, substmt, substmt_index, &((struct lysp_ext_instance*)ext)[u], &extflag, 1);
        } else {
            yprc_extension_instances(ctx, substmt, substmt_index, &((struct lysc_ext_instance*)ext)[u], &extflag, 1);
        }
    }

    /* argument as yin-element */
    if (ext_substmt_info[substmt].flags & SUBST_FLAG_YIN) {
        ypr_yin_arg(ctx, ext_substmt_info[substmt].arg, text);
    }

    LEVEL--;
    ypr_close(ctx, ext_substmt_info[substmt].name, extflag);
}

static void
ypr_unsigned(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index, void *exts, unsigned int attr_value)
{
    char *str;
    if (asprintf(&str, "%u", attr_value) == -1) {
        LOGMEM(ctx->module->ctx);
        ctx->out->status = LY_EMEM;
        return;
    }
    ypr_substmt(ctx, substmt, substmt_index, str, exts);
    free(str);
}

static void
ypr_signed(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index, void *exts, signed int attr_value)
{
    char *str;

    if (asprintf(&str, "%d", attr_value) == -1) {
        LOGMEM(ctx->module->ctx);
        ctx->out->status = LY_EMEM;
        return;
    }
    ypr_substmt(ctx, substmt, substmt_index, str, exts);
    free(str);
}

static void
yprp_revision(struct ypr_ctx *ctx, const struct lysp_revision *rev)
{
    if (rev->dsc || rev->ref || rev->exts) {
        ypr_open(ctx, "revision", "date", rev->date, 1);
        LEVEL++;
        yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, rev->exts, NULL, 0);
        ypr_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, rev->dsc, rev->exts);
        ypr_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, rev->ref, rev->exts);
        LEVEL--;
        ypr_close(ctx, "revision", 1);
    } else {
        ypr_open(ctx, "revision", "date", rev->date, -1);
    }
}

static void
ypr_mandatory(struct ypr_ctx *ctx, uint16_t flags, void *exts, int *flag)
{
    if (flags & LYS_MAND_MASK) {
        ypr_close_parent(ctx, flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_MANDATORY, 0, (flags & LYS_MAND_TRUE) ? "true" : "false", exts);
    }
}

static void
ypr_config(struct ypr_ctx *ctx, uint16_t flags, void *exts, int *flag)
{
    if (flags & LYS_CONFIG_MASK) {
        ypr_close_parent(ctx, flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_CONFIG, 0, (flags & LYS_CONFIG_W) ? "true" : "false", exts);
    }
}

static void
ypr_status(struct ypr_ctx *ctx, uint16_t flags, void *exts, int *flag)
{
    const char *status = NULL;

    if (flags & LYS_STATUS_CURR) {
        ypr_close_parent(ctx, flag);
        status = "current";
    } else if (flags & LYS_STATUS_DEPRC) {
        ypr_close_parent(ctx, flag);
        status = "deprecated";
    } else if (flags & LYS_STATUS_OBSLT) {
        ypr_close_parent(ctx, flag);
        status = "obsolete";
    }

    ypr_substmt(ctx, LYEXT_SUBSTMT_STATUS, 0, status, exts);
}

static void
ypr_description(struct ypr_ctx *ctx, const char *dsc, void *exts, int *flag)
{
    if (dsc) {
        ypr_close_parent(ctx, flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, dsc, exts);
    }
}

static void
ypr_reference(struct ypr_ctx *ctx, const char *ref, void *exts, int *flag)
{
    if (ref) {
        ypr_close_parent(ctx, flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, ref, exts);
    }
}

static void
yprp_iffeatures(struct ypr_ctx *ctx, const char **iff, struct lysp_ext_instance *exts, int *flag)
{
    unsigned int u;
    int extflag;

    LY_ARRAY_FOR(iff, u) {
        ypr_close_parent(ctx, flag);
        extflag = 0;

        ly_print(ctx->out, "%*s<if-feature name=\"%s",  INDENT, iff[u]);

        /* extensions */
        LEVEL++;
        LY_ARRAY_FOR(exts, u) {
            if (exts[u].insubstmt != LYEXT_SUBSTMT_IFFEATURE || exts[u].insubstmt_index != u) {
                continue;
            }
            yprp_extension_instances(ctx, LYEXT_SUBSTMT_IFFEATURE, u, &exts[u], &extflag, 1);
        }
        LEVEL--;
        ly_print(ctx->out, "\"/>\n");
    }
}

static void
yprc_iffeature(struct ypr_ctx *ctx, struct lysc_iffeature *feat, int *index_e, int *index_f)
{
    int brackets_flag = *index_e;
    uint8_t op;

    op = lysc_iff_getop(feat->expr, *index_e);
    (*index_e)++;

    switch (op) {
    case LYS_IFF_F:
        if (ctx->module == feat->features[*index_f]->module) {
            ly_print(ctx->out, "%s", feat->features[*index_f]->name);
        } else {
            ly_print(ctx->out, "%s:%s", feat->features[*index_f]->module->prefix, feat->features[*index_f]->name);
        }
        (*index_f)++;
        break;
    case LYS_IFF_NOT:
        ly_print(ctx->out, "not ");
        yprc_iffeature(ctx, feat, index_e, index_f);
        break;
    case LYS_IFF_AND:
        if (brackets_flag) {
            /* AND need brackets only if previous op was not */
            if (*index_e < 2 || lysc_iff_getop(feat->expr, *index_e - 2) != LYS_IFF_NOT) {
                brackets_flag = 0;
            }
        }
        /* falls through */
    case LYS_IFF_OR:
        if (brackets_flag) {
            ly_print(ctx->out, "(");
        }
        yprc_iffeature(ctx, feat, index_e, index_f);
        ly_print(ctx->out, " %s ", op == LYS_IFF_OR ? "or" : "and");
        yprc_iffeature(ctx, feat, index_e, index_f);
        if (brackets_flag) {
            ly_print(ctx->out, ")");
        }
    }
}

static void
yprc_iffeatures(struct ypr_ctx *ctx, struct lysc_iffeature *iff, struct lysc_ext_instance *exts, int *flag)
{
    unsigned int u, v;
    int extflag;

    LY_ARRAY_FOR(iff, u) {
        int index_e = 0, index_f = 0;

        ypr_close_parent(ctx, flag);
        extflag = 0;

        ly_print(ctx->out, "%*s<if-feature name=\"", INDENT);
        yprc_iffeature(ctx, iff, &index_e, &index_f);
        ly_print(ctx->out, "\"");

        /* extensions */
        LEVEL++;
        LY_ARRAY_FOR(exts, v) {
            if (exts[v].insubstmt != LYEXT_SUBSTMT_IFFEATURE || exts[v].insubstmt_index != u) {
                continue;
            }
            yprc_extension_instances(ctx, LYEXT_SUBSTMT_IFFEATURE, u, &exts[v], &extflag, 1);
        }
        LEVEL--;
        ypr_close(ctx, "if-feature", extflag);
    }
}

static void
yprp_extension(struct ypr_ctx *ctx, const struct lysp_ext *ext)
{
    int flag = 0, flag2 = 0;
    unsigned int i;

    ypr_open(ctx, "extension", "name", ext->name, flag);
    LEVEL++;

    if (ext->exts) {
        ypr_close_parent(ctx, &flag);
        yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, ext->exts, &flag, 0);
    }

    if (ext->argument) {
        ypr_close_parent(ctx, &flag);
        ypr_open(ctx, "argument", "name", ext->argument, flag2);

        LEVEL++;
        if (ext->exts) {
            i = -1;
            while ((i = lysp_ext_instance_iter(ext->exts, i + 1, LYEXT_SUBSTMT_ARGUMENT)) != LY_ARRAY_SIZE(ext->exts)) {
                ypr_close_parent(ctx, &flag2);
                yprp_extension_instances(ctx, LYEXT_SUBSTMT_ARGUMENT, 0, &ext->exts[i], &flag2, 1);
            }
        }
        if ((ext->flags & LYS_YINELEM_MASK) ||
                (ext->exts && lysp_ext_instance_iter(ext->exts, 0, LYEXT_SUBSTMT_YINELEM) != LY_ARRAY_SIZE(ext->exts))) {
            ypr_close_parent(ctx, &flag2);
            ypr_substmt(ctx, LYEXT_SUBSTMT_YINELEM, 0, (ext->flags & LYS_YINELEM_TRUE) ? "true" : "false", ext->exts);
        }
        LEVEL--;
        ypr_close(ctx, "argument", flag2);
    }

    ypr_status(ctx, ext->flags, ext->exts, &flag);
    ypr_description(ctx, ext->dsc, ext->exts, &flag);
    ypr_reference(ctx, ext->ref, ext->exts, &flag);

    LEVEL--;
    ypr_close(ctx, "extension", flag);
}

static void
yprp_feature(struct ypr_ctx *ctx, const struct lysp_feature *feat)
{
    int flag = 0;

    ypr_open(ctx, "feature", "name", feat->name, flag);
    LEVEL++;
    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, feat->exts, &flag, 0);
    yprp_iffeatures(ctx, feat->iffeatures, feat->exts, &flag);
    ypr_status(ctx, feat->flags, feat->exts, &flag);
    ypr_description(ctx, feat->dsc, feat->exts, &flag);
    ypr_reference(ctx, feat->ref, feat->exts, &flag);
    LEVEL--;
    ypr_close(ctx, "feature", flag);
}

static void
yprc_feature(struct ypr_ctx *ctx, const struct lysc_feature *feat)
{
    int flag = 0;

    ypr_open(ctx, "feature", "name", feat->name, flag);
    LEVEL++;
    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, feat->exts, &flag, 0);
    yprc_iffeatures(ctx, feat->iffeatures, feat->exts, &flag);
    ypr_status(ctx, feat->flags, feat->exts, &flag);
    ypr_description(ctx, feat->dsc, feat->exts, &flag);
    ypr_reference(ctx, feat->ref, feat->exts, &flag);
    LEVEL--;
    ypr_close(ctx, "feature", flag);
}

static void
yprp_identity(struct ypr_ctx *ctx, const struct lysp_ident *ident)
{
    int flag = 0;
    unsigned int u;

    ypr_open(ctx, "identity", "name", ident->name, flag);
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, ident->exts, &flag, 0);
    yprp_iffeatures(ctx, ident->iffeatures, ident->exts, &flag);

    LY_ARRAY_FOR(ident->bases, u) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_BASE, u, ident->bases[u], ident->exts);
    }

    ypr_status(ctx, ident->flags, ident->exts, &flag);
    ypr_description(ctx, ident->dsc, ident->exts, &flag);
    ypr_reference(ctx, ident->ref, ident->exts, &flag);

    LEVEL--;
    ypr_close(ctx, "identity", flag);
}

static void
yprc_identity(struct ypr_ctx *ctx, const struct lysc_ident *ident)
{
    int flag = 0;
    unsigned int u;

    ypr_open(ctx, "identity", "name", ident->name, flag);
    LEVEL++;

    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, ident->exts, &flag, 0);
    yprc_iffeatures(ctx, ident->iffeatures, ident->exts, &flag);

    LY_ARRAY_FOR(ident->derived, u) {
        ypr_close_parent(ctx, &flag);
        if (ctx->module != ident->derived[u]->module) {
            ly_print(ctx->out, "%*s<derived name=\"%s:%s\"/>\n", INDENT, ident->derived[u]->module->prefix, ident->derived[u]->name);
        } else {
            ly_print(ctx->out, "%*s<derived name=\"%s\"/>\n", INDENT, ident->derived[u]->name);
        }
    }

    ypr_status(ctx, ident->flags, ident->exts, &flag);
    ypr_description(ctx, ident->dsc, ident->exts, &flag);
    ypr_reference(ctx, ident->ref, ident->exts, &flag);

    LEVEL--;
    ypr_close(ctx, "identity", flag);
}

static void
yprp_restr(struct ypr_ctx *ctx, const struct lysp_restr *restr, const char *name, const char *attr, int *flag)
{
    (void)flag;
    int inner_flag = 0;

    if (!restr) {
        return;
    }

    ly_print(ctx->out, "%*s<%s %s=\"", INDENT, name, attr);
    lyxml_dump_text(ctx->out, (restr->arg[0] != 0x15 && restr->arg[0] != 0x06) ? restr->arg : &restr->arg[1], 1);
    ly_print(ctx->out, "\"");

    LEVEL++;
    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, restr->exts, &inner_flag, 0);
    if (restr->arg[0] == 0x15) {
        ypr_close_parent(ctx, &inner_flag);
        /* special byte value in pattern's expression: 0x15 - invert-match, 0x06 - match */
        ypr_substmt(ctx, LYEXT_SUBSTMT_MODIFIER, 0, "invert-match", restr->exts);
    }
    if (restr->emsg) {
        ypr_close_parent(ctx, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRMSG, 0, restr->emsg, restr->exts);
    }
    if (restr->eapptag) {
        ypr_close_parent(ctx, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRTAG, 0, restr->eapptag, restr->exts);
    }
    ypr_description(ctx, restr->dsc, restr->exts, &inner_flag);
    ypr_reference(ctx, restr->ref, restr->exts, &inner_flag);

    LEVEL--;
    ypr_close(ctx, name, inner_flag);
}

static void
yprc_must(struct ypr_ctx *ctx, const struct lysc_must *must, int *flag)
{
    (void)flag;
    int inner_flag = 0;

    ly_print(ctx->out, "%*s<must condition=\"", INDENT);
    lyxml_dump_text(ctx->out, must->cond->expr, 1);
    ly_print(ctx->out, "\"");

    LEVEL++;
    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, must->exts, &inner_flag, 0);
    if (must->emsg) {
        ypr_close_parent(ctx, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRMSG, 0, must->emsg, must->exts);
    }
    if (must->eapptag) {
        ypr_close_parent(ctx, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRTAG, 0, must->eapptag, must->exts);
    }
    ypr_description(ctx, must->dsc, must->exts, &inner_flag);
    ypr_reference(ctx, must->ref, must->exts, &inner_flag);

    LEVEL--;
    ypr_close(ctx, "must", inner_flag);
}

static void
yprc_range(struct ypr_ctx *ctx, const struct lysc_range *range, LY_DATA_TYPE basetype, int *flag)
{
    int inner_flag = 0;
    unsigned int u;

    if (!range) {
        return;
    }

    ypr_close_parent(ctx, flag);
    ly_print(ctx->out, "%*s<%s value=\"", INDENT, (basetype == LY_TYPE_STRING || basetype == LY_TYPE_BINARY) ? "length" : "range");
    LY_ARRAY_FOR(range->parts, u) {
        if (u > 0) {
            ly_print(ctx->out, " | ");
        }
        if (range->parts[u].max_64 == range->parts[u].min_64) {
            if (basetype <= LY_TYPE_STRING) { /* unsigned values */
                ly_print(ctx->out, "%"PRIu64, range->parts[u].max_u64);
            } else { /* signed values */
                ly_print(ctx->out, "%"PRId64, range->parts[u].max_64);
            }
        } else {
            if (basetype <= LY_TYPE_STRING) { /* unsigned values */
                ly_print(ctx->out, "%"PRIu64"..%"PRIu64, range->parts[u].min_u64, range->parts[u].max_u64);
            } else { /* signed values */
                ly_print(ctx->out, "%"PRId64"..%"PRId64, range->parts[u].min_64, range->parts[u].max_64);
            }
        }
    }
    ly_print(ctx->out, "\"");

    LEVEL++;
    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, range->exts, &inner_flag, 0);
    if (range->emsg) {
        ypr_close_parent(ctx, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRMSG, 0, range->emsg, range->exts);
    }
    if (range->eapptag) {
        ypr_close_parent(ctx, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRTAG, 0, range->eapptag, range->exts);
    }
    ypr_description(ctx, range->dsc, range->exts, &inner_flag);
    ypr_reference(ctx, range->ref, range->exts, &inner_flag);

    LEVEL--;
    ypr_close(ctx, (basetype == LY_TYPE_STRING || basetype == LY_TYPE_BINARY) ? "length" : "range", inner_flag);
}

static void
yprc_pattern(struct ypr_ctx *ctx, const struct lysc_pattern *pattern, int *flag)
{
    (void)flag;
    int inner_flag = 0;

    ly_print(ctx->out, "%*s<pattern value=\"", INDENT);
    lyxml_dump_text(ctx->out, pattern->expr, 1);
    ly_print(ctx->out, "\"");

    LEVEL++;
    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, pattern->exts, &inner_flag, 0);
    if (pattern->inverted) {
        /* special byte value in pattern's expression: 0x15 - invert-match, 0x06 - match */
        ypr_close_parent(ctx, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_MODIFIER, 0, "invert-match", pattern->exts);
    }
    if (pattern->emsg) {
        ypr_close_parent(ctx, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRMSG, 0, pattern->emsg, pattern->exts);
    }
    if (pattern->eapptag) {
        ypr_close_parent(ctx, &inner_flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ERRTAG, 0, pattern->eapptag, pattern->exts);
    }
    ypr_description(ctx, pattern->dsc, pattern->exts, &inner_flag);
    ypr_reference(ctx, pattern->ref, pattern->exts, &inner_flag);

    LEVEL--;
    ypr_close(ctx, "pattern", inner_flag);
}

static void
yprp_when(struct ypr_ctx *ctx, struct lysp_when *when, int *flag)
{
    int inner_flag = 0;
    (void)flag;

    if (!when) {
        return;
    }

    ly_print(ctx->out, "%*s<when condition=\"", INDENT);
    lyxml_dump_text(ctx->out, when->cond, 1);
    ly_print(ctx->out, "\"");

    LEVEL++;
    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, when->exts, &inner_flag, 0);
    ypr_description(ctx, when->dsc, when->exts, &inner_flag);
    ypr_reference(ctx, when->ref, when->exts, &inner_flag);
    LEVEL--;
    ypr_close(ctx, "when", inner_flag);
}

static void
yprc_when(struct ypr_ctx *ctx, struct lysc_when *when, int *flag)
{
    int inner_flag = 0;
    (void)flag;

    if (!when) {
        return;
    }

    ly_print(ctx->out, "%*s<when condition=\"", INDENT);
    lyxml_dump_text(ctx->out, when->cond->expr, 1);
    ly_print(ctx->out, "\"");

    LEVEL++;
    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, when->exts, &inner_flag, 0);
    ypr_description(ctx, when->dsc, when->exts, &inner_flag);
    ypr_reference(ctx, when->ref, when->exts, &inner_flag);
    LEVEL--;
    ypr_close(ctx, "when", inner_flag);
}

static void
yprp_enum(struct ypr_ctx *ctx, const struct lysp_type_enum *items, LY_DATA_TYPE type, int *flag)
{
    unsigned int u;
    int inner_flag;
    (void)flag;

    LY_ARRAY_FOR(items, u) {
        if (type == LY_TYPE_BITS) {
            ly_print(ctx->out, "%*s<bit name=\"", INDENT);
            lyxml_dump_text(ctx->out, items[u].name, 1);
            ly_print(ctx->out, "\"");
        } else { /* LY_TYPE_ENUM */
            ly_print(ctx->out, "%*s<enum name=\"", INDENT);
            lyxml_dump_text(ctx->out, items[u].name, 1);
            ly_print(ctx->out, "\"");
        }
        inner_flag = 0;
        LEVEL++;
        yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, items[u].exts, &inner_flag, 0);
        yprp_iffeatures(ctx, items[u].iffeatures, items[u].exts, &inner_flag);
        if (items[u].flags & LYS_SET_VALUE) {
            if (type == LY_TYPE_BITS) {
                ypr_close_parent(ctx, &inner_flag);
                ypr_unsigned(ctx, LYEXT_SUBSTMT_POSITION, 0, items[u].exts, items[u].value);
            } else { /* LY_TYPE_ENUM */
                ypr_close_parent(ctx, &inner_flag);
                ypr_signed(ctx, LYEXT_SUBSTMT_VALUE, 0, items[u].exts, items[u].value);
            }
        }
        ypr_status(ctx, items[u].flags, items[u].exts, &inner_flag);
        ypr_description(ctx, items[u].dsc, items[u].exts, &inner_flag);
        ypr_reference(ctx, items[u].ref, items[u].exts, &inner_flag);
        LEVEL--;
        ypr_close(ctx, type == LY_TYPE_BITS ? "bit" : "enum", inner_flag);
    }
}

static void
yprp_type(struct ypr_ctx *ctx, const struct lysp_type *type)
{
    if (!ctx || !type) {
        return;
    }

    unsigned int u;
    int flag = 0;

    ypr_open(ctx, "type", "name", type->name, flag);
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, type->exts, &flag, 0);

    if (type->range || type->length || type->patterns || type->bits || type->enums) {
        ypr_close_parent(ctx, &flag);
    }
    yprp_restr(ctx, type->range, "range", "value", &flag);
    yprp_restr(ctx, type->length, "length", "value", &flag);
    LY_ARRAY_FOR(type->patterns, u) {
        yprp_restr(ctx, &type->patterns[u], "pattern", "value", &flag);
    }
    yprp_enum(ctx, type->bits, LY_TYPE_BITS, &flag);
    yprp_enum(ctx, type->enums, LY_TYPE_ENUM, &flag);

    if (type->path) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_PATH, 0, type->path, type->exts);
    }
    if (type->flags & LYS_SET_REQINST) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_REQINSTANCE, 0, type->require_instance ? "true" : "false", type->exts);
    }
    if (type->flags & LYS_SET_FRDIGITS) {
        ypr_close_parent(ctx, &flag);
        ypr_unsigned(ctx, LYEXT_SUBSTMT_FRACDIGITS, 0, type->exts, type->fraction_digits);
    }
    LY_ARRAY_FOR(type->bases, u) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_BASE, u, type->bases[u], type->exts);
    }
    LY_ARRAY_FOR(type->types, u) {
        ypr_close_parent(ctx, &flag);
        yprp_type(ctx, &type->types[u]);
    }

    LEVEL--;
    ypr_close(ctx, "type", flag);
}

static void
yprc_dflt_value(struct ypr_ctx *ctx, const struct lyd_value *value, const struct lys_module *value_mod, struct lysc_ext_instance *exts)
{
    int dynamic;
    const char *str;

    str = value->realtype->plugin->print(value, LYD_JSON, lys_get_prefix, (void*)value_mod, &dynamic);
    ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, str, exts);
    if (dynamic) {
        free((void*)str);
    }
}

static void
yprc_type(struct ypr_ctx *ctx, const struct lysc_type *type)
{
    unsigned int u;
    int flag = 0;

    ypr_open(ctx, "type", "name", lys_datatype2str(type->basetype), flag);
    LEVEL++;

    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, type->exts, &flag, 0);
    if (type->dflt) {
        ypr_close_parent(ctx, &flag);
        yprc_dflt_value(ctx, type->dflt, type->dflt_mod, type->exts);
    }

    switch(type->basetype) {
    case LY_TYPE_BINARY: {
        struct lysc_type_bin *bin = (struct lysc_type_bin*)type;
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
        struct lysc_type_num *num = (struct lysc_type_num*)type;
        yprc_range(ctx, num->range, type->basetype, &flag);
        break;
    }
    case LY_TYPE_STRING: {
        struct lysc_type_str *str = (struct lysc_type_str*)type;
        yprc_range(ctx, str->length, type->basetype, &flag);
        LY_ARRAY_FOR(str->patterns, u) {
            ypr_close_parent(ctx, &flag);
            yprc_pattern(ctx, str->patterns[u], &flag);
        }
        break;
    }
    case LY_TYPE_BITS:
    case LY_TYPE_ENUM: {
        /* bits and enums structures are compatible */
        struct lysc_type_bits *bits = (struct lysc_type_bits*)type;
        LY_ARRAY_FOR(bits->bits, u) {
            ypr_close_parent(ctx, &flag);
            struct lysc_type_bitenum_item *item = &bits->bits[u];
            int inner_flag = 0;

            ly_print(ctx->out, "%*s<%s name=\"", INDENT, type->basetype == LY_TYPE_BITS ? "bit" : "enum");
            lyxml_dump_text(ctx->out, item->name, 1);
            ly_print(ctx->out, "\"");
            LEVEL++;
            yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, item->exts, &inner_flag, 0);
            yprc_iffeatures(ctx, item->iffeatures, item->exts, &inner_flag);
            if (type->basetype == LY_TYPE_BITS) {
                ypr_close_parent(ctx, &inner_flag);
                ypr_unsigned(ctx, LYEXT_SUBSTMT_POSITION, 0, item->exts, item->position);
            } else { /* LY_TYPE_ENUM */
                ypr_close_parent(ctx, &inner_flag);
                ypr_signed(ctx, LYEXT_SUBSTMT_VALUE, 0, item->exts, item->value);
            }
            ypr_status(ctx, item->flags, item->exts, &inner_flag);
            ypr_description(ctx, item->dsc, item->exts, &inner_flag);
            ypr_reference(ctx, item->ref, item->exts, &inner_flag);
            LEVEL--;
            ypr_close(ctx, type->basetype == LY_TYPE_BITS ? "bit" : "enum", inner_flag);
        }
        break;
    }
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
        /* nothing to do */
        break;
    case LY_TYPE_DEC64: {
        struct lysc_type_dec *dec = (struct lysc_type_dec*)type;
        ypr_close_parent(ctx, &flag);
        ypr_unsigned(ctx, LYEXT_SUBSTMT_FRACDIGITS, 0, type->exts, dec->fraction_digits);
        yprc_range(ctx, dec->range, dec->basetype, &flag);
        break;
    }
    case LY_TYPE_IDENT: {
        struct lysc_type_identityref *ident = (struct lysc_type_identityref*)type;
        LY_ARRAY_FOR(ident->bases, u) {
            ypr_close_parent(ctx, &flag);
            ypr_substmt(ctx, LYEXT_SUBSTMT_BASE, u, ident->bases[u]->name, type->exts);
        }
        break;
    }
    case LY_TYPE_INST: {
        struct lysc_type_instanceid *inst = (struct lysc_type_instanceid*)type;
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_REQINSTANCE, 0, inst->require_instance ? "true" : "false", inst->exts);
        break;
    }
    case LY_TYPE_LEAFREF: {
        struct lysc_type_leafref *lr = (struct lysc_type_leafref*)type;
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_PATH, 0, lr->path, lr->exts);
        ypr_substmt(ctx, LYEXT_SUBSTMT_REQINSTANCE, 0, lr->require_instance ? "true" : "false", lr->exts);
        yprc_type(ctx, lr->realtype);
        break;
    }
    case LY_TYPE_UNION: {
        struct lysc_type_union *un = (struct lysc_type_union*)type;
        LY_ARRAY_FOR(un->types, u) {
            ypr_close_parent(ctx, &flag);
            yprc_type(ctx, un->types[u]);
        }
        break;
    }
    default:
        LOGINT(ctx->module->ctx);
        ctx->out->status = LY_EINT;
    }

    LEVEL--;
    ypr_close(ctx, "type", flag);
}

static void
yprp_typedef(struct ypr_ctx *ctx, const struct lysp_tpdf *tpdf)
{
    LYOUT_CHECK(ctx->out);

    ypr_open(ctx, "typedef", "name", tpdf->name, 1);
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, tpdf->exts, NULL, 0);

    yprp_type(ctx, &tpdf->type);

    if (tpdf->units) {
        ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, tpdf->units, tpdf->exts);
    }
    if (tpdf->dflt) {
        ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, tpdf->dflt, tpdf->exts);
    }

    ypr_status(ctx, tpdf->flags, tpdf->exts, NULL);
    ypr_description(ctx, tpdf->dsc, tpdf->exts, NULL);
    ypr_reference(ctx, tpdf->ref, tpdf->exts, NULL);

    LEVEL--;
    ypr_close(ctx, "typedef", 1);
}

static void yprp_node(struct ypr_ctx *ctx, const struct lysp_node *node);
static void yprc_node(struct ypr_ctx *ctx, const struct lysc_node *node);
static void yprp_action(struct ypr_ctx *ctx, const struct lysp_action *action);

static void
yprp_grouping(struct ypr_ctx *ctx, const struct lysp_grp *grp)
{
    unsigned int u;
    int flag = 0;
    struct lysp_node *data;

    LYOUT_CHECK(ctx->out);

    ypr_open(ctx, "grouping", "name", grp->name, flag);
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, grp->exts, &flag, 0);
    ypr_status(ctx, grp->flags, grp->exts, &flag);
    ypr_description(ctx, grp->dsc, grp->exts, &flag);
    ypr_reference(ctx, grp->ref, grp->exts, &flag);

    LY_ARRAY_FOR(grp->typedefs, u) {
        ypr_close_parent(ctx, &flag);
        yprp_typedef(ctx, &grp->typedefs[u]);
    }

    LY_ARRAY_FOR(grp->groupings, u) {
        ypr_close_parent(ctx, &flag);
        yprp_grouping(ctx, &grp->groupings[u]);
    }

    LY_LIST_FOR(grp->data, data) {
        ypr_close_parent(ctx, &flag);
        yprp_node(ctx, data);
    }

    LY_ARRAY_FOR(grp->actions, u) {
        ypr_close_parent(ctx, &flag);
        yprp_action(ctx, &grp->actions[u]);
    }

    LEVEL--;
    ypr_close(ctx, "grouping", flag);
}

static void
yprp_inout(struct ypr_ctx *ctx, const struct lysp_action_inout *inout, int *flag)
{
    unsigned int u;
    struct lysp_node *data;

    if (!inout->nodetype) {
        /* nodetype not set -> input/output is empty */
        return;
    }
    ypr_close_parent(ctx, flag);

    ypr_open(ctx, (inout->nodetype == LYS_INPUT ? "input" : "output"), NULL, NULL, *flag);
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, inout->exts, NULL, 0);
    LY_ARRAY_FOR(inout->musts, u) {
        yprp_restr(ctx, &inout->musts[u], "must", "condition", NULL);
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
    ypr_close(ctx, (inout->nodetype == LYS_INPUT ? "input" : "output"), 1);
}

static void
yprc_inout(struct ypr_ctx *ctx, const struct lysc_action *action, const struct lysc_action_inout *inout, int *flag)
{
    unsigned int u;
    struct lysc_node *data;

    if (!inout->data) {
        /* input/output is empty */
        return;
    }

    ypr_open(ctx, (&action->input == inout) ? "input" : "output", NULL, NULL, *flag);
    LEVEL++;

    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, (&action->input == inout) ? action->input_exts : action->output_exts, NULL, 0);
    LY_ARRAY_FOR(inout->musts, u) {
        yprc_must(ctx, &inout->musts[u], NULL);
    }

    LY_LIST_FOR(inout->data, data) {
        yprc_node(ctx, data);
    }

    LEVEL--;
    ypr_close(ctx, (&action->input == inout) ? "input" : "output", 1);
}

static void
yprp_notification(struct ypr_ctx *ctx, const struct lysp_notif *notif)
{
    unsigned int u;
    int flag = 0;
    struct lysp_node *data;

    LYOUT_CHECK(ctx->out);

    ypr_open(ctx, "notification", "name", notif->name, flag);

    LEVEL++;
    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, notif->exts, &flag, 0);
    yprp_iffeatures(ctx, notif->iffeatures, notif->exts, &flag);

    LY_ARRAY_FOR(notif->musts, u) {
        ypr_close_parent(ctx, &flag);
        yprp_restr(ctx, &notif->musts[u], "must", "condition", &flag);
    }
    ypr_status(ctx, notif->flags, notif->exts, &flag);
    ypr_description(ctx, notif->dsc, notif->exts, &flag);
    ypr_reference(ctx, notif->ref, notif->exts, &flag);

    LY_ARRAY_FOR(notif->typedefs, u) {
        ypr_close_parent(ctx, &flag);
        yprp_typedef(ctx, &notif->typedefs[u]);
    }

    LY_ARRAY_FOR(notif->groupings, u) {
        ypr_close_parent(ctx, &flag);
        yprp_grouping(ctx, &notif->groupings[u]);
    }

    LY_LIST_FOR(notif->data, data) {
        ypr_close_parent(ctx, &flag);
        yprp_node(ctx, data);
    }

    LEVEL--;
    ypr_close(ctx, "notification", flag);
}

static void
yprc_notification(struct ypr_ctx *ctx, const struct lysc_notif *notif)
{
    unsigned int u;
    int flag = 0;
    struct lysc_node *data;

    LYOUT_CHECK(ctx->out);

    ypr_open(ctx, "notification", "name", notif->name, flag);

    LEVEL++;
    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, notif->exts, &flag, 0);
    yprc_iffeatures(ctx, notif->iffeatures, notif->exts, &flag);

    LY_ARRAY_FOR(notif->musts, u) {
        ypr_close_parent(ctx, &flag);
        yprc_must(ctx, &notif->musts[u], &flag);
    }
    ypr_status(ctx, notif->flags, notif->exts, &flag);
    ypr_description(ctx, notif->dsc, notif->exts, &flag);
    ypr_reference(ctx, notif->ref, notif->exts, &flag);

    LY_LIST_FOR(notif->data, data) {
        ypr_close_parent(ctx, &flag);
        yprc_node(ctx, data);
    }

    LEVEL--;
    ypr_close(ctx, "notification", flag);
}

static void
yprp_action(struct ypr_ctx *ctx, const struct lysp_action *action)
{
    unsigned int u;
    int flag = 0;

    LYOUT_CHECK(ctx->out);

    ypr_open(ctx, action->parent ? "action" : "rpc", "name", action->name, flag);

    LEVEL++;
    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, action->exts, &flag, 0);
    yprp_iffeatures(ctx, action->iffeatures, action->exts, &flag);
    ypr_status(ctx, action->flags, action->exts, &flag);
    ypr_description(ctx, action->dsc, action->exts, &flag);
    ypr_reference(ctx, action->ref, action->exts, &flag);

    LY_ARRAY_FOR(action->typedefs, u) {
        ypr_close_parent(ctx, &flag);
        yprp_typedef(ctx, &action->typedefs[u]);
    }

    LY_ARRAY_FOR(action->groupings, u) {
        ypr_close_parent(ctx, &flag);
        yprp_grouping(ctx, &action->groupings[u]);
    }

    yprp_inout(ctx, &action->input, &flag);
    yprp_inout(ctx, &action->output, &flag);

    LEVEL--;
    ypr_close(ctx, action->parent ? "action" : "rpc", flag);
}

static void
yprc_action(struct ypr_ctx *ctx, const struct lysc_action *action)
{
    int flag = 0;

    LYOUT_CHECK(ctx->out);

    ypr_open(ctx, action->parent ? "action" : "rpc", "name", action->name, flag);

    LEVEL++;
    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, action->exts, &flag, 0);
    yprc_iffeatures(ctx, action->iffeatures, action->exts, &flag);
    ypr_status(ctx, action->flags, action->exts, &flag);
    ypr_description(ctx, action->dsc, action->exts, &flag);
    ypr_reference(ctx, action->ref, action->exts, &flag);

    yprc_inout(ctx, action, &action->input, &flag);
    yprc_inout(ctx, action, &action->output, &flag);

    LEVEL--;
    ypr_close(ctx, action->parent ? "action" : "rpc", flag);
}

static void
yprp_node_common1(struct ypr_ctx *ctx, const struct lysp_node *node, int *flag)
{
    ypr_open(ctx, lys_nodetype2str(node->nodetype), "name", node->name, *flag);
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, node->exts, flag, 0);
    yprp_when(ctx, node->when, flag);
    yprp_iffeatures(ctx, node->iffeatures, node->exts, flag);
}

static void
yprc_node_common1(struct ypr_ctx *ctx, const struct lysc_node *node, int *flag)
{
    unsigned int u;

    ypr_open(ctx, lys_nodetype2str(node->nodetype), "name", node->name, *flag);
    LEVEL++;

    yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, node->exts, flag, 0);
    LY_ARRAY_FOR(node->when, u) {
        ypr_close_parent(ctx, flag);
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
yprp_node_common2(struct ypr_ctx *ctx, const struct lysp_node *node, int *flag)
{
    YPR_NODE_COMMON2;
}

static void
yprc_node_common2(struct ypr_ctx *ctx, const struct lysc_node *node, int *flag)
{
    YPR_NODE_COMMON2;
}

#undef YPR_NODE_COMMON2

static void
yprp_container(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    unsigned int u;
    int flag = 0;
    struct lysp_node *child;
    struct lysp_node_container *cont = (struct lysp_node_container *)node;

    yprp_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(cont->musts, u) {
        ypr_close_parent(ctx, &flag);
        yprp_restr(ctx, &cont->musts[u], "must", "condition", &flag);
    }
    if (cont->presence) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_PRESENCE, 0, cont->presence, cont->exts);
    }

    yprp_node_common2(ctx, node, &flag);

    LY_ARRAY_FOR(cont->typedefs, u) {
        ypr_close_parent(ctx, &flag);
        yprp_typedef(ctx, &cont->typedefs[u]);
    }

    LY_ARRAY_FOR(cont->groupings, u) {
        ypr_close_parent(ctx, &flag);
        yprp_grouping(ctx, &cont->groupings[u]);
    }

    LY_LIST_FOR(cont->child, child) {
        ypr_close_parent(ctx, &flag);
        yprp_node(ctx, child);
    }

    LY_ARRAY_FOR(cont->actions, u) {
        ypr_close_parent(ctx, &flag);
        yprp_action(ctx, &cont->actions[u]);
    }

    LY_ARRAY_FOR(cont->notifs, u) {
        ypr_close_parent(ctx, &flag);
        yprp_notification(ctx, &cont->notifs[u]);
    }

    LEVEL--;
    ypr_close(ctx, "container", flag);
}

static void
yprc_container(struct ypr_ctx *ctx, const struct lysc_node *node)
{
    unsigned int u;
    int flag = 0;
    struct lysc_node *child;
    struct lysc_node_container *cont = (struct lysc_node_container *)node;

    yprc_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(cont->musts, u) {
        ypr_close_parent(ctx, &flag);
        yprc_must(ctx, &cont->musts[u], &flag);
    }
    if (cont->flags & LYS_PRESENCE) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_PRESENCE, 0, "true", cont->exts);
    }

    yprc_node_common2(ctx, node, &flag);

    LY_LIST_FOR(cont->child, child) {
        ypr_close_parent(ctx, &flag);
        yprc_node(ctx, child);
    }

    LY_ARRAY_FOR(cont->actions, u) {
        ypr_close_parent(ctx, &flag);
        yprc_action(ctx, &cont->actions[u]);
    }

    LY_ARRAY_FOR(cont->notifs, u) {
        ypr_close_parent(ctx, &flag);
        yprc_notification(ctx, &cont->notifs[u]);
    }

    LEVEL--;
    ypr_close(ctx, "container", flag);
}

static void
yprp_case(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    int flag = 0;
    struct lysp_node *child;
    struct lysp_node_case *cas = (struct lysp_node_case *)node;

    yprp_node_common1(ctx, node, &flag);
    yprp_node_common2(ctx, node, &flag);

    LY_LIST_FOR(cas->child, child) {
        ypr_close_parent(ctx, &flag);
        yprp_node(ctx, child);
    }

    LEVEL--;
    ypr_close(ctx, "case", flag);
}

static void
yprc_case(struct ypr_ctx *ctx, const struct lysc_node_case *cs)
{
    int flag = 0;
    struct lysc_node *child;

    yprc_node_common1(ctx, (struct lysc_node*)cs, &flag);
    yprc_node_common2(ctx, (struct lysc_node*)cs, &flag);

    for (child = cs->child; child && child->parent == (struct lysc_node*)cs; child = child->next) {
        ypr_close_parent(ctx, &flag);
        yprc_node(ctx, child);
    }

    LEVEL--;
    ypr_close(ctx, "case", flag);
}

static void
yprp_choice(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    int flag = 0;
    struct lysp_node *child;
    struct lysp_node_choice *choice = (struct lysp_node_choice *)node;

    yprp_node_common1(ctx, node, &flag);

    if (choice->dflt) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, choice->dflt, choice->exts);
    }

    yprp_node_common2(ctx, node, &flag);

    LY_LIST_FOR(choice->child, child) {
        ypr_close_parent(ctx, &flag);
        yprp_node(ctx, child);
    }

    LEVEL--;
    ypr_close(ctx, "choice", flag);
}

static void
yprc_choice(struct ypr_ctx *ctx, const struct lysc_node *node)
{
    int flag = 0;
    struct lysc_node_case *cs;
    struct lysc_node_choice *choice = (struct lysc_node_choice *)node;

    yprc_node_common1(ctx, node, &flag);

    if (choice->dflt) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, choice->dflt->name, choice->exts);
    }

    yprc_node_common2(ctx, node, &flag);

    for (cs = choice->cases; cs; cs = (struct lysc_node_case*)cs->next) {
        ypr_close_parent(ctx, &flag);
        yprc_case(ctx, cs);
    }

    LEVEL--;
    ypr_close(ctx, "choice", flag);
}

static void
yprp_leaf(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    unsigned int u;
    struct lysp_node_leaf *leaf = (struct lysp_node_leaf *)node;

    int flag = 1;
    yprp_node_common1(ctx, node, &flag);

    yprp_type(ctx, &leaf->type);
    ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, leaf->units, leaf->exts);
    LY_ARRAY_FOR(leaf->musts, u) {
        yprp_restr(ctx, &leaf->musts[u], "must", "condition", &flag);
    }
    ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, leaf->dflt, leaf->exts);

    yprp_node_common2(ctx, node, &flag);

    LEVEL--;
    ypr_close(ctx, "leaf", flag);
}

static void
yprc_leaf(struct ypr_ctx *ctx, const struct lysc_node *node)
{
    unsigned int u;
    struct lysc_node_leaf *leaf = (struct lysc_node_leaf *)node;

    int flag = 1;
    yprc_node_common1(ctx, node, &flag);

    yprc_type(ctx, leaf->type);
    ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, leaf->units, leaf->exts);
    LY_ARRAY_FOR(leaf->musts, u) {
        yprc_must(ctx, &leaf->musts[u], &flag);
    }

    if (leaf->dflt) {
        yprc_dflt_value(ctx, leaf->dflt, leaf->dflt_mod, leaf->exts);
    }

    yprc_node_common2(ctx, node, &flag);

    LEVEL--;
    ypr_close(ctx, "leaf", flag);
}

static void
yprp_leaflist(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    unsigned int u;
    struct lysp_node_leaflist *llist = (struct lysp_node_leaflist *)node;
    int flag = 1;

    yprp_node_common1(ctx, node, &flag);

    yprp_type(ctx, &llist->type);
    ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, llist->units, llist->exts);
    LY_ARRAY_FOR(llist->musts, u) {
        yprp_restr(ctx, &llist->musts[u], "must", "condition", NULL);
    }
    LY_ARRAY_FOR(llist->dflts, u) {
        ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, u, llist->dflts[u], llist->exts);
    }

    ypr_config(ctx, node->flags, node->exts, NULL);

    if (llist->flags & LYS_SET_MIN) {
        ypr_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, llist->exts, llist->min);
    }
    if (llist->flags & LYS_SET_MAX) {
        if (llist->max) {
            ypr_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, llist->exts, llist->max);
        } else {
            ypr_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", llist->exts);
        }
    }

    if (llist->flags & LYS_ORDBY_MASK) {
        ypr_substmt(ctx, LYEXT_SUBSTMT_ORDEREDBY, 0, (llist->flags & LYS_ORDBY_USER) ? "user" : "system", llist->exts);
    }

    ypr_status(ctx, node->flags, node->exts, &flag);
    ypr_description(ctx, node->dsc, node->exts, &flag);
    ypr_reference(ctx, node->ref, node->exts, &flag);

    LEVEL--;
    ypr_close(ctx, "leaf-list", flag);
}

static void
yprc_leaflist(struct ypr_ctx *ctx, const struct lysc_node *node)
{
    unsigned int u;
    struct lysc_node_leaflist *llist = (struct lysc_node_leaflist *)node;
    int flag = 1;

    yprc_node_common1(ctx, node, &flag);

    yprc_type(ctx, llist->type);
    ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, llist->units, llist->exts);
    LY_ARRAY_FOR(llist->musts, u) {
        yprc_must(ctx, &llist->musts[u], NULL);
    }
    LY_ARRAY_FOR(llist->dflts, u) {
        yprc_dflt_value(ctx, llist->dflts[u], llist->dflts_mods[u], llist->exts);
    }

    ypr_config(ctx, node->flags, node->exts, NULL);

    ypr_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, llist->exts, llist->min);
    if (llist->max) {
        ypr_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, llist->exts, llist->max);
    } else {
        ypr_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", llist->exts);
    }

    ypr_substmt(ctx, LYEXT_SUBSTMT_ORDEREDBY, 0, (llist->flags & LYS_ORDBY_USER) ? "user" : "system", llist->exts);

    ypr_status(ctx, node->flags, node->exts, &flag);
    ypr_description(ctx, node->dsc, node->exts, &flag);
    ypr_reference(ctx, node->ref, node->exts, &flag);

    LEVEL--;
    ypr_close(ctx, "leaf-list", flag);
}

static void
yprp_list(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    unsigned int u;
    int flag = 0;
    struct lysp_node *child;
    struct lysp_node_list *list = (struct lysp_node_list *)node;

    yprp_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(list->musts, u) {
        ypr_close_parent(ctx, &flag);
        yprp_restr(ctx, &list->musts[u], "must", "condition", &flag);
    }
    if (list->key) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_KEY, 0, list->key, list->exts);
    }
    LY_ARRAY_FOR(list->uniques, u) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_UNIQUE, u, list->uniques[u], list->exts);
    }

    ypr_config(ctx, node->flags, node->exts, NULL);

    if (list->flags & LYS_SET_MIN) {
        ypr_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, list->exts, list->min);
    }
    if (list->flags & LYS_SET_MAX) {
        if (list->max) {
            ypr_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, list->exts, list->max);
        } else {
            ypr_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", list->exts);
        }
    }

    if (list->flags & LYS_ORDBY_MASK) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_ORDEREDBY, 0, (list->flags & LYS_ORDBY_USER) ? "user" : "system", list->exts);
    }

    ypr_status(ctx, node->flags, node->exts, &flag);
    ypr_description(ctx, node->dsc, node->exts, &flag);
    ypr_reference(ctx, node->ref, node->exts, &flag);

    LY_ARRAY_FOR(list->typedefs, u) {
        ypr_close_parent(ctx, &flag);
        yprp_typedef(ctx, &list->typedefs[u]);
    }

    LY_ARRAY_FOR(list->groupings, u) {
        ypr_close_parent(ctx, &flag);
        yprp_grouping(ctx, &list->groupings[u]);
    }

    LY_LIST_FOR(list->child, child) {
        ypr_close_parent(ctx, &flag);
        yprp_node(ctx, child);
    }

    LY_ARRAY_FOR(list->actions, u) {
        ypr_close_parent(ctx, &flag);
        yprp_action(ctx, &list->actions[u]);
    }

    LY_ARRAY_FOR(list->notifs, u) {
        ypr_close_parent(ctx, &flag);
        yprp_notification(ctx, &list->notifs[u]);
    }

    LEVEL--;
    ypr_close(ctx, "list", flag);
}

static void
yprc_list(struct ypr_ctx *ctx, const struct lysc_node *node)
{
    unsigned int u, v;
    int flag = 0;
    struct lysc_node *child;
    struct lysc_node_list *list = (struct lysc_node_list *)node;

    yprc_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(list->musts, u) {
        yprc_must(ctx, &list->musts[u], NULL);
    }
    if (!(list->flags & LYS_KEYLESS)) {
        ypr_close_parent(ctx, &flag);
        ly_print(ctx->out, "%*s<key value=\"", INDENT);
        for (struct lysc_node *key = list->child; key && key->nodetype == LYS_LEAF && (key->flags & LYS_KEY); key = key->next) {
            ly_print(ctx->out, "%s%s", u > 0 ? ", " : "", key->name);
        }
        ly_print(ctx->out, "\"");
        ypr_close(ctx, "key", 0);
    }
    LY_ARRAY_FOR(list->uniques, u) {
        ypr_close_parent(ctx, &flag);
        ly_print(ctx->out, "%*s<unique tag=\"", INDENT);
        LY_ARRAY_FOR(list->uniques[u], v) {
            ly_print(ctx->out, "%s%s", v > 0 ? ", " : "", list->uniques[u][v]->name);
        }
        ly_print(ctx->out, "\"");
        ypr_close(ctx, "unique", 0);
    }

    ypr_config(ctx, node->flags, node->exts, NULL);

    ypr_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, list->exts, list->min);
    if (list->max) {
        ypr_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, list->exts, list->max);
    } else {
        ypr_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", list->exts);
    }

    ypr_substmt(ctx, LYEXT_SUBSTMT_ORDEREDBY, 0, (list->flags & LYS_ORDBY_USER) ? "user" : "system", list->exts);

    ypr_status(ctx, node->flags, node->exts, &flag);
    ypr_description(ctx, node->dsc, node->exts, &flag);
    ypr_reference(ctx, node->ref, node->exts, &flag);

    LY_LIST_FOR(list->child, child) {
        ypr_close_parent(ctx, &flag);
        yprc_node(ctx, child);
    }

    LY_ARRAY_FOR(list->actions, u) {
        ypr_close_parent(ctx, &flag);
        yprc_action(ctx, &list->actions[u]);
    }

    LY_ARRAY_FOR(list->notifs, u) {
        ypr_close_parent(ctx, &flag);
        yprc_notification(ctx, &list->notifs[u]);
    }

    LEVEL--;
    ypr_close(ctx, "list", flag);
}

static void
yprp_refine(struct ypr_ctx *ctx, struct lysp_refine *refine)
{
    unsigned int u;
    int flag = 0;

    ypr_open(ctx, "refine", "target-node", refine->nodeid, flag);
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, refine->exts, &flag, 0);
    yprp_iffeatures(ctx, refine->iffeatures, refine->exts, &flag);

    LY_ARRAY_FOR(refine->musts, u) {
        ypr_close_parent(ctx, &flag);
        yprp_restr(ctx, &refine->musts[u], "must", "condition", &flag);
    }

    if (refine->presence) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_PRESENCE, 0, refine->presence, refine->exts);
    }

    LY_ARRAY_FOR(refine->dflts, u) {
        ypr_close_parent(ctx, &flag);
        ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, u, refine->dflts[u], refine->exts);
    }

    ypr_config(ctx, refine->flags, refine->exts, &flag);
    ypr_mandatory(ctx, refine->flags, refine->exts, &flag);

    if (refine->flags & LYS_SET_MIN) {
        ypr_close_parent(ctx, &flag);
        ypr_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, refine->exts, refine->min);
    }
    if (refine->flags & LYS_SET_MAX) {
        ypr_close_parent(ctx, &flag);
        if (refine->max) {
            ypr_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, refine->exts, refine->max);
        } else {
            ypr_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", refine->exts);
        }
    }

    ypr_description(ctx, refine->dsc, refine->exts, &flag);
    ypr_reference(ctx, refine->ref, refine->exts, &flag);

    LEVEL--;
    ypr_close(ctx, "refine", flag);
}

static void
yprp_augment(struct ypr_ctx *ctx, const struct lysp_augment *aug)
{
    unsigned int u;
    struct lysp_node *child;

    ypr_open(ctx, "augment", "target-node", aug->nodeid, 1);
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
    ypr_close(ctx, "augment", 1);
}


static void
yprp_uses(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    unsigned int u;
    int flag = 0;
    struct lysp_node_uses *uses = (struct lysp_node_uses *)node;

    yprp_node_common1(ctx, node, &flag);
    yprp_node_common2(ctx, node, &flag);

    LY_ARRAY_FOR(uses->refines, u) {
        ypr_close_parent(ctx, &flag);
        yprp_refine(ctx, &uses->refines[u]);
    }

    LY_ARRAY_FOR(uses->augments, u) {
        ypr_close_parent(ctx, &flag);
        yprp_augment(ctx, &uses->augments[u]);
    }

    LEVEL--;
    ypr_close(ctx, "uses", flag);
}

static void
yprp_anydata(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    unsigned int u;
    int flag = 0;
    struct lysp_node_anydata *any = (struct lysp_node_anydata *)node;

    yprp_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(any->musts, u) {
        ypr_close_parent(ctx, &flag);
        yprp_restr(ctx, &any->musts[u], "must", "condition", &flag);
    }

    yprp_node_common2(ctx, node, &flag);

    LEVEL--;
    ypr_close(ctx, lys_nodetype2str(node->nodetype), flag);
}

static void
yprc_anydata(struct ypr_ctx *ctx, const struct lysc_node *node)
{
    unsigned int u;
    int flag = 0;
    struct lysc_node_anydata *any = (struct lysc_node_anydata *)node;

    yprc_node_common1(ctx, node, &flag);

    LY_ARRAY_FOR(any->musts, u) {
        ypr_close_parent(ctx, &flag);
        yprc_must(ctx, &any->musts[u], NULL);
    }

    yprc_node_common2(ctx, node, &flag);

    LEVEL--;
    ypr_close(ctx, lys_nodetype2str(node->nodetype), flag);
}

static void
yprp_node(struct ypr_ctx *ctx, const struct lysp_node *node)
{
    LYOUT_CHECK(ctx->out);

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
    LYOUT_CHECK(ctx->out);

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
    unsigned int v;
    struct lysp_deviate_add *add;
    struct lysp_deviate_rpl *rpl;
    struct lysp_deviate_del *del;
    struct lysp_deviate *elem;

    ypr_open(ctx, "deviation", "target-node", deviation->nodeid, 1);
    LEVEL++;

    yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, deviation->exts, NULL, 0);
    ypr_description(ctx, deviation->dsc, deviation->exts, NULL);
    ypr_reference(ctx, deviation->ref, deviation->exts, NULL);

    LY_LIST_FOR(deviation->deviates, elem) {
        ly_print(ctx->out, "%*s<deviate value=\"", INDENT);
        if (elem->mod == LYS_DEV_NOT_SUPPORTED) {
            if (elem->exts) {
                ly_print(ctx->out, "not-supported {\n");
                LEVEL++;

                yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, elem->exts, NULL, 0);
            } else {
                ly_print(ctx->out, "not-supported;\n");
                continue;
            }
        } else if (elem->mod == LYS_DEV_ADD) {
            add = (struct lysp_deviate_add*)elem;
            ly_print(ctx->out, "add\">\n");
            LEVEL++;

            yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, add->exts, NULL, 0);
            ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, add->units, add->exts);
            LY_ARRAY_FOR(add->musts, v) {
                yprp_restr(ctx, &add->musts[v], "must", "condition", NULL);
            }
            LY_ARRAY_FOR(add->uniques, v) {
                ypr_substmt(ctx, LYEXT_SUBSTMT_UNIQUE, v, add->uniques[v], add->exts);
            }
            LY_ARRAY_FOR(add->dflts, v) {
                ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, v, add->dflts[v], add->exts);
            }
            ypr_config(ctx, add->flags, add->exts, NULL);
            ypr_mandatory(ctx, add->flags, add->exts, NULL);
            if (add->flags & LYS_SET_MIN) {
                ypr_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, add->exts, add->min);
            }
            if (add->flags & LYS_SET_MAX) {
                if (add->max) {
                    ypr_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, add->exts, add->max);
                } else {
                    ypr_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", add->exts);
                }
            }
        } else if (elem->mod == LYS_DEV_REPLACE) {
            rpl = (struct lysp_deviate_rpl*)elem;
            ly_print(ctx->out, "replace\">\n");
            LEVEL++;

            yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, rpl->exts, NULL, 0);
            if (rpl->type) {
                yprp_type(ctx, rpl->type);
            }
            ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, rpl->units, rpl->exts);
            ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, 0, rpl->dflt, rpl->exts);
            ypr_config(ctx, rpl->flags, rpl->exts, NULL);
            ypr_mandatory(ctx, rpl->flags, rpl->exts, NULL);
            if (rpl->flags & LYS_SET_MIN) {
                ypr_unsigned(ctx, LYEXT_SUBSTMT_MIN, 0, rpl->exts, rpl->min);
            }
            if (rpl->flags & LYS_SET_MAX) {
                if (rpl->max) {
                    ypr_unsigned(ctx, LYEXT_SUBSTMT_MAX, 0, rpl->exts, rpl->max);
                } else {
                    ypr_substmt(ctx, LYEXT_SUBSTMT_MAX, 0, "unbounded", rpl->exts);
                }
            }
        } else if (elem->mod == LYS_DEV_DELETE) {
            del = (struct lysp_deviate_del*)elem;
            ly_print(ctx->out, "delete\">\n");
            LEVEL++;

            yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, del->exts, NULL, 0);
            ypr_substmt(ctx, LYEXT_SUBSTMT_UNITS, 0, del->units, del->exts);
            LY_ARRAY_FOR(del->musts, v) {
                yprp_restr(ctx, &del->musts[v], "must", "condition", NULL);
            }
            LY_ARRAY_FOR(del->uniques, v) {
                ypr_substmt(ctx, LYEXT_SUBSTMT_UNIQUE, v, del->uniques[v], del->exts);
            }
            LY_ARRAY_FOR(del->dflts, v) {
                ypr_substmt(ctx, LYEXT_SUBSTMT_DEFAULT, v, del->dflts[v], del->exts);
            }
        }

        LEVEL--;
        ypr_close(ctx, "deviate", 1);
    }

    LEVEL--;
    ypr_close(ctx, "deviation", 1);
}

/**
 * @brief Minimal print of a schema.
 *
 * To print
 * a) compiled schema when it is not compiled or
 * b) parsed when the parsed form was already removed
 */
static LY_ERR
ypr_missing_format(struct ypr_ctx *ctx, const struct lys_module *module)
{
    /* module-header-stmts */
    if (module->version) {
        if (module->version) {
            ypr_substmt(ctx, LYEXT_SUBSTMT_VERSION, 0, module->version == LYS_VERSION_1_1 ? "1.1" : "1", NULL);
        }
    }
    ypr_substmt(ctx, LYEXT_SUBSTMT_NAMESPACE, 0, module->ns, NULL);
    ypr_substmt(ctx, LYEXT_SUBSTMT_PREFIX, 0, module->prefix, NULL);

    /* meta-stmts */
    if (module->org || module->contact || module->dsc || module->ref) {
        ly_print(ctx->out, "\n");
    }
    ypr_substmt(ctx, LYEXT_SUBSTMT_ORGANIZATION, 0, module->org, NULL);
    ypr_substmt(ctx, LYEXT_SUBSTMT_CONTACT, 0, module->contact, NULL);
    ypr_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, module->dsc, NULL);
    ypr_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, module->ref, NULL);

    /* revision-stmts */
    if (module->revision) {
        ly_print(ctx->out, "\n%*srevision %s;\n", INDENT, module->revision);
    }

    LEVEL--;
    ly_print(ctx->out, "%*s}\n", INDENT);
    ly_print_flush(ctx->out);

    return LY_SUCCESS;
}

static void
ypr_xmlns(struct ypr_ctx *ctx, const struct lys_module *module)
{
    unsigned int u;
    char *space = "        ";

    ly_print(ctx->out, "%s%*sxmlns=\"%s\"", space, INDENT, YIN_NS_URI);
    ly_print(ctx->out, "\n%s%*sxmlns:%s=\"%s\"", space, INDENT, module->prefix, module->ns);

    struct lysp_module *modp = module->parsed;

    LY_ARRAY_FOR(modp->imports, u){
        ly_print(ctx->out, "\n%s%*sxmlns:%s=\"%s\"", space, INDENT, modp->imports[u].prefix, modp->imports[u].module->ns);
    }
}


struct ext_substmt_info_s stmt_attr_info[] = {
    {NULL,               NULL,          0},              /**< LY_STMT_NONE*/
    {"status",           "value",       SUBST_FLAG_ID}, /**< LY_STMT_STATUS */
    {"config",           "value",       SUBST_FLAG_ID}, /**< LY_STMT_CONFIG */
    {"mandatory",        "value",       SUBST_FLAG_ID}, /**< LY_STMT_MANDATORY */
    {"units",            "name",        SUBST_FLAG_ID}, /**< LY_STMT_UNITS */
    {"default",          "value",       SUBST_FLAG_ID}, /**< LY_STMT_DEFAULT */
    {"type",             "name",        SUBST_FLAG_ID}, /**< LY_STMT_TYPE */
    {"action",           "name",        SUBST_FLAG_ID}, /**< LY_STMT_ACTION */
    {"anydata",          "name",        SUBST_FLAG_ID}, /**< LY_STMT_ANYDATA */
    {"anyxml",           "name",        SUBST_FLAG_ID}, /**< LY_STMT_ANYXML */
    {"argument",         "name",        SUBST_FLAG_ID}, /**< LY_STMT_ARGUMENT */
    {"augment",          "target-node", SUBST_FLAG_ID}, /**< LY_STMT_AUGMENT */
    {"base",             "name",        SUBST_FLAG_ID}, /**< LY_STMT_BASE */
    {"belongs-to",       "module",      SUBST_FLAG_ID}, /**< LY_STMT_BELONGS_TO */
    {"bit",              "name",        SUBST_FLAG_ID}, /**< LY_STMT_BIT */
    {"case",             "name",        SUBST_FLAG_ID}, /**< LY_STMT_CASE */
    {"choice",           "name",        SUBST_FLAG_ID}, /**< LY_STMT_CHOICE */
    {"contact",          "text",        SUBST_FLAG_YIN},/**< LY_STMT_CONTACT */
    {"container",        "name",        SUBST_FLAG_ID}, /**< LY_STMT_CONTAINER */
    {"description",      "text",        SUBST_FLAG_YIN},/**< LY_STMT_DESCRIPTION */
    {"deviate",          "value",       SUBST_FLAG_ID}, /**< LY_STMT_DEVIATE */
    {"deviation",        "target-node", SUBST_FLAG_ID}, /**< LY_STMT_DEVIATION */
    {"enum",             "name",        SUBST_FLAG_ID}, /**< LY_STMT_ENUM */
    {"error-app-tag",    "value",       SUBST_FLAG_ID}, /**< LY_STMT_ERROR_APP_TAG */
    {"error-message",    "value",       SUBST_FLAG_YIN},/**< LY_STMT_ERROR_MESSAGE */
    {"extension",        "name",        SUBST_FLAG_ID}, /**< LY_STMT_EXTENSION */
    {"feature",          "name",        SUBST_FLAG_ID}, /**< LY_STMT_FEATURE */
    {"fraction-digits",  "value",       SUBST_FLAG_ID}, /**< LY_STMT_FRACTION_DIGITS */
    {"grouping",         "name",        SUBST_FLAG_ID}, /**< LY_STMT_GROUPING */
    {"identity",         "name",        SUBST_FLAG_ID}, /**< LY_STMT_IDENTITY */
    {"if-feature",       "name",        SUBST_FLAG_ID}, /**< LY_STMT_IF_FEATURE */
    {"import",           "module",      SUBST_FLAG_ID}, /**< LY_STMT_IMPORT */
    {"include",          "module",      SUBST_FLAG_ID}, /**< LY_STMT_INCLUDE */
    {"input",            NULL,          0},             /**< LY_STMT_INPUT */
    {"key",              "value",       SUBST_FLAG_ID}, /**< LY_STMT_KEY */
    {"leaf",             "name",        SUBST_FLAG_ID}, /**< LY_STMT_LEAF */
    {"leaf-list",        "name",        SUBST_FLAG_ID}, /**< LY_STMT_LEAF_LIST */
    {"length",           "value",       SUBST_FLAG_ID}, /**< LY_STMT_LENGTH */
    {"list",             "name",        SUBST_FLAG_ID}, /**< LY_STMT_LIST */
    {"max-elements",     "value",       SUBST_FLAG_ID}, /**< LY_STMT_MAX_ELEMENTS */
    {"min-elements",     "value",       SUBST_FLAG_ID}, /**< LY_STMT_MIN_ELEMENTS */
    {"modifier",         "value",       SUBST_FLAG_ID}, /**< LY_STMT_MODIFIER */
    {"module",           "name",        SUBST_FLAG_ID}, /**< LY_STMT_MODULE */
    {"must",             "condition",   SUBST_FLAG_ID}, /**< LY_STMT_MUST */
    {"namespace",        "uri",         SUBST_FLAG_ID}, /**< LY_STMT_NAMESPACE */
    {"notification",     "name",        SUBST_FLAG_ID}, /**< LY_STMT_NOTIFICATION */
    {"ordered-by",       "value",       SUBST_FLAG_ID}, /**< LY_STMT_ORDERED_BY */
    {"organization",     "text",        SUBST_FLAG_YIN},/**< LY_STMT_ORGANIZATION */
    {"output",           NULL,          0},             /**< LY_STMT_OUTPUT */
    {"path",             "value",       SUBST_FLAG_ID}, /**< LY_STMT_PATH */
    {"pattern",          "value",       SUBST_FLAG_ID}, /**< LY_STMT_PATTERN */
    {"position",         "value",       SUBST_FLAG_ID}, /**< LY_STMT_POSITION */
    {"prefix",           "value",       SUBST_FLAG_ID}, /**< LY_STMT_PREFIX */
    {"presence",         "value",       SUBST_FLAG_ID}, /**< LY_STMT_PRESENCE */
    {"range",            "value",       SUBST_FLAG_ID}, /**< LY_STMT_RANGE */
    {"reference",        "text",        SUBST_FLAG_YIN},/**< LY_STMT_REFERENCE */
    {"refine",           "target-node", SUBST_FLAG_ID}, /**< LY_STMT_REFINE */
    {"require-instance", "value",       SUBST_FLAG_ID}, /**< LY_STMT_REQUIRE_INSTANCE */
    {"revision",         "date",        SUBST_FLAG_ID}, /**< LY_STMT_REVISION */
    {"revision-date",    "date",        SUBST_FLAG_ID}, /**< LY_STMT_REVISION_DATE */
    {"rpc",              "name",        SUBST_FLAG_ID}, /**< LY_STMT_RPC */
    {"submodule",        "name",        SUBST_FLAG_ID}, /**< LY_STMT_SUBMODULE */
    {"typedef",          "name",        SUBST_FLAG_ID}, /**< LY_STMT_TYPEDEF */
    {"unique",           "tag",         SUBST_FLAG_ID}, /**< LY_STMT_UNIQUE */
    {"uses",             "name",        SUBST_FLAG_ID}, /**< LY_STMT_USES */
    {"value",            "value",       SUBST_FLAG_ID}, /**< LY_STMT_VALUE */
    {"when",             "condition",   SUBST_FLAG_ID}, /**< LY_STMT_WHEN */
    {"yang-version",     "value",       SUBST_FLAG_ID}, /**< LY_STMT_YANG_VERSION */
    {"yin-element",      "value",       SUBST_FLAG_ID}, /**< LY_STMT_YIN_ELEMENT */
    {NULL,               NULL,          0},             /**< LY_STMT_EXTENSION_INSTANCE */
    {NULL,               NULL,          0},             /**< LY_STMT_SYNTAX_SEMICOLON */
    {NULL,               NULL,          0},             /**< LY_STMT_SYNTAX_LEFT_BRACE */
    {NULL,               NULL,          0},             /**< LY_STMT_SYNTAX_RIGHT_BRACE */
    {NULL,               NULL,          0},             /**< LY_STMT_ARG_TEXT */
    {NULL,               NULL,          0},             /**< LY_STMT_ARG_VALUE */
};

static void
yprp_stmt(struct ypr_ctx *ctx, struct lysp_stmt *stmt)
{
    struct lysp_stmt *childstmt;
    int flag = stmt->child ? 1 : -1;

    /* TODO:
             the extension instance substatements in extension instances (LY_STMT_EXTENSION_INSTANCE)
             cannot find the compiled information, so it is needed to be done,
             currently it is ignored */
    if(stmt_attr_info[stmt->kw].name) {
        if(stmt_attr_info[stmt->kw].flags & SUBST_FLAG_YIN) {
            ypr_open(ctx, stmt->stmt, NULL, NULL, flag);
            ypr_yin_arg(ctx, stmt_attr_info[stmt->kw].arg, stmt->arg);
        }
        else {
            ypr_open(ctx, stmt->stmt, stmt_attr_info[stmt->kw].arg, stmt->arg, flag);
        }
    }

    if (stmt->child) {
        LEVEL++;
        LY_LIST_FOR(stmt->child, childstmt) {
            yprp_stmt(ctx, childstmt);
        }
        LEVEL--;
        ypr_close(ctx, stmt->stmt, flag);
    }
}

/**
 * @param[in] count Number of extensions to print, 0 to print them all.
 */
static void
yprp_extension_instances(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index,
                               struct lysp_ext_instance *ext, int *flag, unsigned int count)
{
    unsigned int u;
    char *str;
    struct lysp_stmt *stmt;
    const char *argument;
    const char *ext_argument;

    if (!count && ext) {
        count = LY_ARRAY_SIZE(ext);
    }
    LY_ARRAY_FOR(ext, u) {
        if (!count) {
            break;
        }

        count--;
        if (ext->insubstmt != substmt || ext->insubstmt_index != substmt_index) {
            continue;
        }

        if (!ext->compiled && ext->yin) {
            ly_print(ctx->out, "%*s<%s/> <!-- Model comes from different input format, extensions must be resolved first. -->\n", INDENT, ext[u].name);
            continue;
        }

        ypr_close_parent(ctx, flag);
        int inner_flag = 0;
        argument = NULL;
        ext_argument = NULL;

        if (ext[u].compiled) {
            argument = ext[u].compiled->argument;
            ext_argument = ext[u].compiled->def->argument;
        } else {
            argument = ext[u].argument;
        }

        if (ext->yin) {
            ypr_open(ctx, ext[u].name, NULL, NULL, 1);
            if (asprintf(&str, "%s:%s", ext[u].compiled->def->module->prefix, ext_argument) == -1) {
                LOGMEM(ctx->module->ctx);
                ctx->out->status = LY_EMEM;
                return;
            }
            LEVEL++;
            inner_flag = 1;
            ypr_yin_arg(ctx, str, argument);
            free(str);
            str = NULL;
            LEVEL--;
        } else {
            ypr_open(ctx, ext[u].name, ext_argument, argument, inner_flag);
        }

        LEVEL++;
        LY_LIST_FOR(ext[u].child, stmt) {
            ypr_close_parent(ctx, &inner_flag);
            yprp_stmt(ctx, stmt);
        }
        LEVEL--;
        ypr_close(ctx, ext[u].name, inner_flag);
    }
}

/**
 * @param[in] count Number of extensions to print, 0 to print them all.
 */
static void
yprc_extension_instances(struct ypr_ctx *ctx, LYEXT_SUBSTMT substmt, uint8_t substmt_index,
                                 struct lysc_ext_instance *ext, int *flag, unsigned int count)
{
    unsigned int u;

    if (!count && ext) {
        count = LY_ARRAY_SIZE(ext);
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

LY_ERR
yin_print_parsed(struct lyout *out, const struct lys_module *module)
{
    unsigned int u;
    struct lysp_node *data;
    struct lysp_module *modp = module->parsed;
    struct ypr_ctx ctx_ = {.out = out, .level = 0, .module = module, .schema = YPR_PARSED}, *ctx = &ctx_;

    ly_print(ctx->out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    ly_print(ctx->out, "%*s<module name=\"%s\"\n", INDENT, module->name);
    ypr_xmlns(ctx, module);
    ly_print(ctx->out, ">\n");

    LEVEL++;

    if (!modp) {
        ly_print(ctx->out, "%*s/* PARSED INFORMATION ARE NOT FULLY PRESENT */\n", INDENT);
        return ypr_missing_format(ctx, module);
    }

    /* module-header-stmts */
    if (module->version) {
        if (module->version) {
            ypr_substmt(ctx, LYEXT_SUBSTMT_VERSION, 0, module->version == LYS_VERSION_1_1 ? "1.1" : "1", modp->exts);
        }
    }
    ypr_substmt(ctx, LYEXT_SUBSTMT_NAMESPACE, 0, module->ns, modp->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_PREFIX, 0, module->prefix, modp->exts);

    /* linkage-stmts */
    LY_ARRAY_FOR(modp->imports, u) {
        ypr_open(ctx, "import", "module", modp->imports[u].module->name, 1);
        LEVEL++;
        yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, modp->imports[u].exts, NULL, 0);
        ypr_substmt(ctx, LYEXT_SUBSTMT_PREFIX, 0, modp->imports[u].prefix, modp->imports[u].exts);
        if (modp->imports[u].rev[0]) {
            ypr_substmt(ctx, LYEXT_SUBSTMT_REVISIONDATE, 0, modp->imports[u].rev, modp->imports[u].exts);
        }
        ypr_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, modp->imports[u].dsc, modp->imports[u].exts);
        ypr_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, modp->imports[u].ref, modp->imports[u].exts);
        LEVEL--;
        ypr_close(ctx, "import", 1);
    }
    LY_ARRAY_FOR(modp->includes, u) {
        if (modp->includes[u].rev[0] || modp->includes[u].dsc || modp->includes[u].ref || modp->includes[u].exts) {
            ypr_open(ctx, "include", "module", modp->includes[u].submodule->name, 1);
            LEVEL++;
            yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, modp->includes[u].exts, NULL, 0);
            if (modp->includes[u].rev[0]) {
                ypr_substmt(ctx, LYEXT_SUBSTMT_REVISIONDATE, 0, modp->includes[u].rev, modp->includes[u].exts);
            }
            ypr_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, modp->includes[u].dsc, modp->includes[u].exts);
            ypr_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, modp->includes[u].ref, modp->includes[u].exts);
            LEVEL--;
            ly_print(out, "%*s}\n", INDENT);
        } else {
            ypr_open(ctx, "include", "module", modp->includes[u].submodule->name, -1);
        }
    }

    /* meta-stmts */
    if (module->org || module->contact || module->dsc || module->ref) {
        ly_print(out, "\n");
    }
    ypr_substmt(ctx, LYEXT_SUBSTMT_ORGANIZATION, 0, module->org, modp->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_CONTACT, 0, module->contact, modp->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, module->dsc, modp->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, module->ref, modp->exts);

    /* revision-stmts */
    if (modp->revs) {
        ly_print(out, "\n");
    }
    LY_ARRAY_FOR(modp->revs, u) {
        yprp_revision(ctx, &modp->revs[u]);
    }
    /* body-stmts */
    LY_ARRAY_FOR(modp->extensions, u) {
        ly_print(out, "\n");
        yprp_extension(ctx, &modp->extensions[u]);
    }
    if (modp->exts) {
        ly_print(out, "\n");
        yprp_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, module->parsed->exts, NULL, 0);
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

    LEVEL--;
    ly_print(out, "%*s</module>\n", INDENT);
    ly_print_flush(out);

    return LY_SUCCESS;
}

LY_ERR
yin_print_compiled(struct lyout *out, const struct lys_module *module)
{
    unsigned int u;
    struct lysc_node *data;
    struct lysc_module *modc = module->compiled;
    struct ypr_ctx ctx_ = {.out = out, .level = 0, .module = module}, *ctx = &ctx_;

    ly_print(ctx->out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    ly_print(ctx->out, "%*s<module name=\"%s\"\n", INDENT, module->name);
    ypr_xmlns(ctx, module);
    ly_print(ctx->out, ">\n");
    LEVEL++;

    if (!modc) {
        ly_print(ctx->out, "%*s/* COMPILED INFORMATION ARE NOT PRESENT */\n", INDENT);
        return ypr_missing_format(ctx, module);
    }

    /* module-header-stmts */
    if (module->version) {
        if (module->version) {
            ypr_substmt(ctx, LYEXT_SUBSTMT_VERSION, 0, module->version == LYS_VERSION_1_1 ? "1.1" : "1", modc->exts);
        }
    }
    ypr_substmt(ctx, LYEXT_SUBSTMT_NAMESPACE, 0, module->ns, modc->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_PREFIX, 0, module->prefix, modc->exts);

    /* linkage-stmts */
    LY_ARRAY_FOR(modc->imports, u) {
        ypr_open(ctx, "import", "module", modc->imports[u].module->name, 1);
        LEVEL++;
        yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, modc->imports[u].exts, NULL, 0);
        ypr_substmt(ctx, LYEXT_SUBSTMT_PREFIX, 0, modc->imports[u].prefix, modc->imports[u].exts);
        if (modc->imports[u].module->revision) {
            ypr_substmt(ctx, LYEXT_SUBSTMT_REVISIONDATE, 0, modc->imports[u].module->revision, modc->imports[u].exts);
        }
        LEVEL--;
        ypr_close(ctx, "import", 1);
    }

    /* meta-stmts */
    if (module->org || module->contact || module->dsc || module->ref) {
        ly_print(out, "\n");
    }
    ypr_substmt(ctx, LYEXT_SUBSTMT_ORGANIZATION, 0, module->org, modc->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_CONTACT, 0, module->contact, modc->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_DESCRIPTION, 0, module->dsc, modc->exts);
    ypr_substmt(ctx, LYEXT_SUBSTMT_REFERENCE, 0, module->ref, modc->exts);

    /* revision-stmts */
    if (module->revision) {
        ly_print(ctx->out, "\n%*s<revision date=\"%s\"/>\n", INDENT, module->revision);
    }

    /* body-stmts */
    if (modc->exts) {
        ly_print(out, "\n");
        yprc_extension_instances(ctx, LYEXT_SUBSTMT_SELF, 0, module->compiled->exts, NULL, 0);
    }

    LY_ARRAY_FOR(modc->features, u) {
        yprc_feature(ctx, &modc->features[u]);
    }

    LY_ARRAY_FOR(modc->identities, u) {
        yprc_identity(ctx, &modc->identities[u]);
    }

    LY_LIST_FOR(modc->data, data) {
        yprc_node(ctx, data);
    }

    LY_ARRAY_FOR(modc->rpcs, u) {
        yprc_action(ctx, &modc->rpcs[u]);
    }

    LY_ARRAY_FOR(modc->notifs, u) {
        yprc_notification(ctx, &modc->notifs[u]);
    }

    LEVEL--;
    ly_print(out, "%*s</module>\n", INDENT);
    ly_print_flush(out);

    return LY_SUCCESS;
}

