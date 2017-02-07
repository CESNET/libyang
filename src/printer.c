/**
 * @file printer.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Wrapper for all libyang printers.
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* vasprintf(), vdprintf() */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "tree_schema.h"
#include "tree_data.h"
#include "printer.h"

struct ext_substmt_info_s ext_substmt_info[] = {
  {NULL, NULL, 0},                              /**< LYEXT_SUBSTMT_SELF */
  {"argument", "target-node", SUBST_FLAG_ID},   /**< LYEXT_SUBSTMT_ARGUMENT */
  {"base", "name", SUBST_FLAG_ID},              /**< LYEXT_SUBSTMT_BASE */
  {"belongs-to", "module", SUBST_FLAG_ID},      /**< LYEXT_SUBSTMT_BELONGSTO */
  {"contact", "text", SUBST_FLAG_YIN},          /**< LYEXT_SUBSTMT_CONTACT */
  {"default", "value", 0},                      /**< LYEXT_SUBSTMT_DEFAULT */
  {"description", "text", SUBST_FLAG_YIN},      /**< LYEXT_SUBSTMT_DESCRIPTION */
  {"error-app-tag", "value", 0},                /**< LYEXT_SUBSTMT_ERRTAG */
  {"error-message", "value", SUBST_FLAG_YIN},   /**< LYEXT_SUBSTMT_ERRMSG */
  {"key", "value", 0},                          /**< LYEXT_SUBSTMT_KEY */
  {"namespace", "uri", 0},                      /**< LYEXT_SUBSTMT_NAMESPACE */
  {"organization", "text", SUBST_FLAG_YIN},     /**< LYEXT_SUBSTMT_ORGANIZATION */
  {"path", "value", 0},                         /**< LYEXT_SUBSTMT_PATH */
  {"prefix", "value", SUBST_FLAG_ID},           /**< LYEXT_SUBSTMT_PREFIX */
  {"presence", "value", 0},                     /**< LYEXT_SUBSTMT_PRESENCE */
  {"reference", "text", SUBST_FLAG_YIN},        /**< LYEXT_SUBSTMT_REFERENCE */
  {"revision-date", "date", SUBST_FLAG_ID},     /**< LYEXT_SUBSTMT_REVISIONDATE */
  {"units", "name", 0},                         /**< LYEXT_SUBSTMT_UNITS */
  {"value", "value", SUBST_FLAG_ID},            /**< LYEXT_SUBSTMT_VALUE */
  {"yang-version", "value", SUBST_FLAG_ID},     /**< LYEXT_SUBSTMT_VERSION */
  {"modifier", "value", SUBST_FLAG_ID},         /**< LYEXT_SUBSTMT_MODIFIER */
  {"require-instance", "value", SUBST_FLAG_ID}, /**< LYEXT_SUBSTMT_REQINST */
  {"yin-element", "value", SUBST_FLAG_ID},      /**< LYEXT_SUBSTMT_YINELEM */
  {"config", "value", SUBST_FLAG_ID},           /**< LYEXT_SUBSTMT_CONFIG */
  {"mandatory", "value", SUBST_FLAG_ID},        /**< LYEXT_SUBSTMT_MANDATORY */
  {"ordered-by", "value", SUBST_FLAG_ID},       /**< LYEXT_SUBSTMT_ORDEREDBY */
  {"status", "value", SUBST_FLAG_ID},           /**< LYEXT_SUBSTMT_STATUS */
  {"fraction-digits", "value", SUBST_FLAG_ID},  /**< LYEXT_SUBSTMT_DIGITS */
  {"max-elements", "value", SUBST_FLAG_ID},     /**< LYEXT_SUBSTMT_MAX */
  {"min-elements", "value", SUBST_FLAG_ID},     /**< LYEXT_SUBSTMT_MIN */
  {"position", "value", SUBST_FLAG_ID},         /**< LYEXT_SUBSTMT_POSITION */
  {"unique", "tag", 0},                         /**< LYEXT_SUBSTMT_UNIQUE */
};

/* 0 - same, 1 - different */
int
nscmp(const struct lyd_node *node1, const struct lyd_node *node2)
{
    /* we have to cover submodules belonging to the same module */
    if (lys_node_module(node1->schema) == lys_node_module(node2->schema)) {
        /* belongs to the same module */
        return 0;
    } else {
        /* different modules */
        return 1;
    }
}

int
ly_print(struct lyout *out, const char *format, ...)
{
    int count = 0;
    char *msg = NULL, *aux;
    va_list ap;

    va_start(ap, format);

    switch(out->type) {
    case LYOUT_FD:
        count = vdprintf(out->method.fd, format, ap);
        break;
    case LYOUT_STREAM:
        count = vfprintf(out->method.f, format, ap);
        break;
    case LYOUT_MEMORY:
        count = vasprintf(&msg, format, ap);
        if (out->method.mem.len + count + 1 > out->method.mem.size) {
            aux = ly_realloc(out->method.mem.buf, out->method.mem.len + count + 1);
            if (!aux) {
                out->method.mem.buf = NULL;
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM;
                va_end(ap);
                return -1;
            }
            out->method.mem.buf = aux;
            out->method.mem.size = out->method.mem.len + count + 1;
        }
        memcpy(&out->method.mem.buf[out->method.mem.len], msg, count);
        out->method.mem.len += count;
        out->method.mem.buf[out->method.mem.len] = '\0';
        free(msg);
        break;
    case LYOUT_CALLBACK:
        count = vasprintf(&msg, format, ap);
        count = out->method.clb.f(out->method.clb.arg, msg, count);
        free(msg);
        break;
    }

    va_end(ap);
    return count;
}

void
ly_print_flush(struct lyout *out)
{
    switch (out->type) {
    case LYOUT_STREAM:
        fflush(out->method.f);
        break;
    case LYOUT_FD:
    case LYOUT_MEMORY:
    case LYOUT_CALLBACK:
        /* nothing to do */
        break;
    }
}

int
ly_write(struct lyout *out, const char *buf, size_t count)
{
    char *aux;

    switch(out->type) {
    case LYOUT_FD:
        return write(out->method.fd, buf, count);
    case LYOUT_STREAM:
        return fwrite(buf, sizeof *buf, count, out->method.f);
    case LYOUT_MEMORY:
        if (out->method.mem.len + count + 1 > out->method.mem.size) {
            aux = ly_realloc(out->method.mem.buf, out->method.mem.len + count + 1);
            if (!aux) {
                out->method.mem.buf = NULL;
                out->method.mem.len = 0;
                out->method.mem.size = 0;
                LOGMEM;
                return -1;
            }
            out->method.mem.buf = aux;
            out->method.mem.size = out->method.mem.len + count + 1;
        }
        memcpy(&out->method.mem.buf[out->method.mem.len], buf, count + 1);
        out->method.mem.len += count;
        return count;
    case LYOUT_CALLBACK:
        return out->method.clb.f(out->method.clb.arg, buf, count);
    }

    return 0;
}

static int
write_iff(struct lyout *out, const struct lys_module *module, struct lys_iffeature *expr, int *index_e, int *index_f)
{
    int count = 0, brackets_flag = *index_e;
    uint8_t op;

    op = iff_getop(expr->expr, *index_e);
    (*index_e)++;

    switch (op) {
    case LYS_IFF_F:
        if (lys_main_module(expr->features[*index_f]->module) != lys_main_module(module)) {
            count += ly_print(out, "%s:", transform_module_name2import_prefix(module, lys_main_module(expr->features[*index_f]->module)->name));
        }
        count += ly_print(out, expr->features[*index_f]->name);
        (*index_f)++;
        break;
    case LYS_IFF_NOT:
        count += ly_print(out, "not ");
        count += write_iff(out, module, expr, index_e, index_f);
        break;
    case LYS_IFF_AND:
        if (brackets_flag) {
            /* AND need brackets only if previous op was not */
            if (*index_e < 2 || iff_getop(expr->expr, *index_e - 2) != LYS_IFF_NOT) {
                brackets_flag = 0;
            }
        }
        /* no break */
    case LYS_IFF_OR:
        if (brackets_flag) {
            count += ly_print(out, "(");
        }
        count += write_iff(out, module, expr, index_e, index_f);
        count += ly_print(out, " %s ", op == LYS_IFF_OR ? "or" : "and");
        count += write_iff(out, module, expr, index_e, index_f);
        if (brackets_flag) {
            count += ly_print(out, ")");
        }
    }

    return count;
}

int
ly_print_iffeature(struct lyout *out, const struct lys_module *module, struct lys_iffeature *expr)
{
    int index_e = 0, index_f = 0;

    if (expr->expr) {
        return write_iff(out, module, expr, &index_e, &index_f);
    }

    return 0;
}

static int
lys_print_(struct lyout *out, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node)
{
    int ret;
    int grps = 0;

    switch (format) {
    case LYS_OUT_YIN:
        lys_switch_deviations((struct lys_module *)module);
        ret = yin_print_model(out, module);
        lys_switch_deviations((struct lys_module *)module);
        break;
    case LYS_OUT_YANG:
        lys_switch_deviations((struct lys_module *)module);
        ret = yang_print_model(out, module);
        lys_switch_deviations((struct lys_module *)module);
        break;
    case LYS_OUT_TREE_GRPS:
        grps = 1;
        /* no break */
    case LYS_OUT_TREE:
        ret = tree_print_model(out, module, grps);
        break;
    case LYS_OUT_INFO:
        ret = info_print_model(out, module, target_node);
        break;
    default:
        LOGERR(LY_EINVAL, "Unknown output format.");
        ret = EXIT_FAILURE;
        break;
    }

    return ret;
}

API int
lys_print_file(FILE *f, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node)
{
    struct lyout out;

    if (!f || !module) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_STREAM;
    out.method.f = f;

    return lys_print_(&out, module, format, target_node);
}

API int
lys_print_fd(int fd, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node)
{
    struct lyout out;

    if (fd < 0 || !module) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_FD;
    out.method.fd = fd;

    return lys_print_(&out, module, format, target_node);
}

API int
lys_print_mem(char **strp, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node)
{
    struct lyout out;
    int r;

    if (!strp || !module) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_MEMORY;
    out.method.mem.buf = NULL;
    out.method.mem.len = 0;
    out.method.mem.size = 0;

    r = lys_print_(&out, module, format, target_node);

    *strp = out.method.mem.buf;
    return r;
}

API int
lys_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg, const struct lys_module *module, LYS_OUTFORMAT format, const char *target_node)
{
    struct lyout out;

    if (!writeclb || !module) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_CALLBACK;
    out.method.clb.f = writeclb;
    out.method.clb.arg = arg;

    return lys_print_(&out, module, format, target_node);
}

static int
lyd_print_(struct lyout *out, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    if (!root) {
        /* no data to print, but even empty tree is valid */
        if (out->type == LYOUT_MEMORY || out->type == LYOUT_CALLBACK) {
            ly_print(out, "");
        }
        return EXIT_SUCCESS;
    }

    switch (format) {
    case LYD_XML:
        return xml_print_data(out, root, options);
    case LYD_JSON:
        return json_print_data(out, root, options);
    default:
        LOGERR(LY_EINVAL, "Unknown output format.");
        return EXIT_FAILURE;
    }
}

API int
lyd_print_file(FILE *f, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct lyout out;

    if (!f) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_STREAM;
    out.method.f = f;

    return lyd_print_(&out, root, format, options);
}

API int
lyd_print_fd(int fd, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct lyout out;

    if (fd < 0) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_FD;
    out.method.fd = fd;

    return lyd_print_(&out, root, format, options);
}

API int
lyd_print_mem(char **strp, const struct lyd_node *root, LYD_FORMAT format, int options)
{
    struct lyout out;
    int r;

    if (!strp) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_MEMORY;
    out.method.mem.buf = NULL;
    out.method.mem.len = 0;
    out.method.mem.size = 0;

    r = lyd_print_(&out, root, format, options);

    *strp = out.method.mem.buf;
    return r;
}

API int
lyd_print_clb(ssize_t (*writeclb)(void *arg, const void *buf, size_t count), void *arg, const struct lyd_node *root,
              LYD_FORMAT format, int options)
{
    struct lyout out;

    if (!writeclb) {
        ly_errno = LY_EINVAL;
        return EXIT_FAILURE;
    }

    out.type = LYOUT_CALLBACK;
    out.method.clb.f = writeclb;
    out.method.clb.arg = arg;

    return lyd_print_(&out, root, format, options);
}

int
lyd_wd_toprint(const struct lyd_node *node, int options)
{
    const struct lyd_node *subroot, *next, *elem;
    int flag = 0;

    if (options & LYP_WD_TRIM) {
        /* do not print default nodes */
        if (node->dflt) {
            /* implicit default node */
            return 0;
        } else if (node->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
            if (lyd_wd_default((struct lyd_node_leaf_list *)node)) {
                /* explicit default node */
                return 0;
            }
        } else if ((node->schema->nodetype & (LYS_CONTAINER)) && !((struct lys_node_container *)node->schema)->presence) {
            /* get know if non-presence container contains non-default node */
            for (subroot = node->child; subroot && !flag; subroot = subroot->next) {
                LY_TREE_DFS_BEGIN(subroot, next, elem) {
                    if (elem->dflt) {
                        /* skip subtree */
                        goto trim_dfs_nextsibling;
                    } else if (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST)) {
                        if (!lyd_wd_default((struct lyd_node_leaf_list *)elem)) {
                            /* non-default node */
                            flag = 1;
                            break;
                        }
                    }

                    /* modified LY_TREE_DFS_END */
                    /* select element for the next run - children first */
                    /* child exception for leafs, leaflists and anyxml without children */
                    if (elem->schema->nodetype & (LYS_LEAF | LYS_LEAFLIST | LYS_ANYDATA)) {
                        next = NULL;
                    } else {
                        next = elem->child;
                    }
                    if (!next) {
trim_dfs_nextsibling:
                        /* no children */
                        if (elem == subroot) {
                            /* we are done, (START) has no children */
                            break;
                        }
                        /* try siblings */
                        next = elem->next;
                    }
                    while (!next) {
                        /* parent is already processed, go to its sibling */
                        elem = elem->parent;
                        /* no siblings, go back through parents */
                        if (elem->parent == subroot->parent) {
                            /* we are done, no next element to process */
                            break;
                        }
                        next = elem->next;
                    }
                }
            }
            if (!flag) {
                /* only default nodes in subtree, do not print the container */
                return 0;
            }
        }
    } else if (node->dflt && !(options & LYP_WD_MASK) && !(node->schema->flags & LYS_CONFIG_R)) {
        /* LYP_WD_EXPLICIT
         * - print only if it contains status data in its subtree */
        LY_TREE_DFS_BEGIN(node, next, elem) {
            if (elem->schema->flags & LYS_CONFIG_R) {
                flag = 1;
                break;
            }
            LY_TREE_DFS_END(node, next, elem)
        }
        if (!flag) {
            return 0;
        }
    } else if (node->dflt && node->schema->nodetype == LYS_CONTAINER && !(options & LYP_KEEPEMPTYCONT)) {
        /* avoid empty default containers */
        LY_TREE_DFS_BEGIN(node, next, elem) {
            if (elem->schema->nodetype != LYS_CONTAINER) {
                flag = 1;
                break;
            }
            LY_TREE_DFS_END(node, next, elem)
        }
        if (!flag) {
            return 0;
        }
    }

    return 1;
}
