/**
 * @file log.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang logger implementation
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _BSD_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#include "common.h"

extern LY_ERR ly_errno_int;
volatile uint8_t ly_log_level = LY_LLERR;
static void (*ly_log_clb)(LY_LOG_LEVEL level, const char *msg, const char *path);
static volatile int path_flag = 1;

API void
ly_verb(LY_LOG_LEVEL level)
{
    ly_log_level = level;
}

API void
ly_set_log_clb(void (*clb)(LY_LOG_LEVEL level, const char *msg, const char *path), int path)
{
    ly_log_clb = clb;
    path_flag = path;
}

API void
(*ly_get_log_clb(void))(LY_LOG_LEVEL, const char *, const char *)
{
    return ly_log_clb;
}

static void
log_vprintf(LY_LOG_LEVEL level, const char *format, const char *path, va_list args)
{
    char *msg;

    if (&ly_errno == &ly_errno_int) {
        msg = "Internal logger error";
    } else if (!format) {
        /* postpone print of path related to the previous error */
        msg = ((struct ly_err *)&ly_errno)->msg;
        snprintf(msg, LY_ERR_MSG_SIZE - 1, "Path related to the last error: \"%s\".", path);
        msg[LY_ERR_MSG_SIZE - 1] = '\0';
    } else {
        msg = ((struct ly_err *)&ly_errno)->msg;
        vsnprintf(msg, LY_ERR_MSG_SIZE - 1, format, args);
        msg[LY_ERR_MSG_SIZE - 1] = '\0';
    }

    if (!path) {
        /* erase previous path */
        ((struct ly_err *)&ly_errno)->path_index = LY_ERR_MSG_SIZE - 1;
    }

    if (ly_log_clb) {
        ly_log_clb(level, msg, path);
    } else {
        fprintf(stderr, "libyang[%d]: %s%s", level, msg, path ? " " : "\n");
        if (path) {
            fprintf(stderr, "(path: %s)\n", path);
        }
    }
#undef PRV_MSG_SIZE
}

void
ly_log(LY_LOG_LEVEL level, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    log_vprintf(level, format, NULL, ap);
    va_end(ap);
}

const char *ly_errs[] = {
/* LYE_SUCCESS */      "",
/* LYE_XML_MISS */     "Missing %s \"%s\".",
/* LYE_XML_INVAL */    "Invalid %s.",
/* LYE_XML_INCHAR */   "Encountered invalid character sequence \"%.10s\".",

/* LYE_EOF */          "Unexpected end of input data.",
/* LYE_INSTMT */       "Invalid keyword \"%s\".",
/* LYE_INCHILDSTMT */  "Invalid keyword \"%s\" as a child to \"%s\".",
/* LYE_INID */         "Invalid identifier \"%s\" (%s).",
/* LYE_INDATE */       "Invalid date format of \"%s\", \"YYYY-MM-DD\" expected.",
/* LYE_INARG */        "Invalid value \"%s\" of \"%s\".",
/* LYE_MISSSTMT */     "Missing keyword \"%s\".",
/* LYE_MISSCHILDSTMT */ "Missing keyword \"%s\" as a child to \"%s\".",
/* LYE_MISSARG */      "Missing argument \"%s\" to keyword \"%s\".",
/* LYE_TOOMANY */      "Too many instances of \"%s\" in \"%s\".",
/* LYE_DUPID */        "Duplicated %s identifier \"%s\".",
/* LYE_DUPLEAFLIST */  "Instances of \"%s\" leaf-list are not unique (\"%s\").",
/* LYE_DUPLIST */      "Instances of \"%s\" list are not unique.",
/* LYE_ENUM_DUPVAL */  "The value \"%d\" of \"%s\" enum has already been assigned to another enum value.",
/* LYE_ENUM_DUPNAME */ "The enum name \"%s\" has already been assigned to another enum.",
/* LYE_ENUM_WS */      "The enum name \"%s\" includes invalid leading or trailing whitespaces.",
/* LYE_BITS_DUPVAL */  "The position \"%d\" of \"%s\" bits has already been used to another named bit.",
/* LYE_BITS_DUPNAME */ "The bit name \"%s\" has already been assigned to another bit.",
/* LYE_INMOD */        "Module name \"%s\" refers to an unknown module.",
/* LYE_INMOD_LEN */    "Module name \"%.*s\" refers to an unknown module.",
/* LYE_KEY_NLEAF */    "Key \"%s\" is not a leaf.",
/* LYE_KEY_TYPE */     "Key \"%s\" must not be the built-in type \"empty\".",
/* LYE_KEY_CONFIG */   "The \"config\" value of the \"%s\" key differs from its list config value.",
/* LYE_KEY_MISS */     "Leaf \"%s\" defined as key in a list not found.",
/* LYE_KEY_DUP */      "Key identifier \"%s\" is not unique.",
/* LYE_INREGEX */      "Regular expression \"%s\" is not valid (%s).",
/* LYE_INRESOLV */     "Failed to resolve %s \"%s\".",
/* LYE_INSTATUS */     "A \"%s\" definition %s references \"%s\" definition %s.",

/* LYE_OBSDATA */      "Obsolete data \"%s\" instantiated.",
/* LYE_OBSTYPE */      "Data node \"%s\" with obsolete type \"%s\" instantiated.",
/* LYE_NORESOLV */     "No resolvents found for \"%s\".",
/* LYE_INELEM */       "Unknown element \"%s\".",
/* LYE_INELEM_LEN */   "Unknown element \"%.*s\".",
/* LYE_MISSELEM */     "Missing required element \"%s\" in \"%s\".",
/* LYE_INVAL */        "Invalid value \"%s\" in \"%s\" element.",
/* LYE_INATTR */       "Invalid attribute \"%s\" in \"%s\" element.",
/* LYE_MISSATTR */     "Missing attribute \"%s\" in \"%s\" element.",
/* LYE_OORVAL */       "Value \"%s\" is out of range or length.",
/* LYE_INCHAR */       "Unexpected character(s) '%c' (%.15s).",
/* LYE_INPRED */       "Predicate resolution failed on \"%s\".",
/* LYE_MCASEDATA */    "Data for more than one case branch of \"%s\" choice present.",
/* LYE_NOCOND */       "%s condition \"%s\" not satisfied.",
/* LYE_INORDER */      "Invalid order of elements \"%s\" and \"%s\".",
/* LYE_INCOUNT */      "Wrong number of \"%s\" elements.",

/* LYE_XPATH_INTOK */  "Unexpected XPath token %s (%.15s).",
/* LYE_XPATH_EOF */    "Unexpected XPath expression end.",
/* LYE_XPATH_INOP_1 */ "Cannot apply XPath operation %s on %s.",
/* LYE_XPATH_INOP_2 */ "Cannot apply XPath operation %s on %s and %s.",
/* LYE_XPATH_INCTX */  "Invalid context type %s in %s.",
/* LYE_XPATH_INARGCOUNT */ "Invalid number of arguments (%d) for the XPath function %s.",
/* LYE_XPATH_INARGTYPE */ "Wrong type of argument #%d (%s) for the XPath function %s.",

/* LYE_PATH_INCHAR */  "Unexpected character(s) '%c' (%s).",
/* LYE_PATH_INMOD */   "Module not found (%s).",
/* LYE_PATH_MISSMOD */ "Missing module name (%s).",
/* LYE_PATH_INNODE */  "Schema node not found (%s).",
/* LYE_PATH_NLIST */   "Predicate used on a node type other than a list (%s).",
/* LYE_PATH_INKEY */   "List key not found (%s).",
/* LYE_PATH_MISSKEY */ "Not all list keys specified (%s).",
/* LYE_PATH_EXISTS */  "Node already exists.",
/* LYE_PATH_MISSPAR */ "Parent does not exist (%s).",
};

static const LY_VECODE ecode2vecode[] = {
    LYVE_SUCCESS,      /* LYE_SUCCESS */

    LYVE_XML_MISS,     /* LYE_XML_MISS */
    LYVE_XML_INVAL,    /* LYE_XML_INVAL */
    LYVE_XML_INCHAR,   /* LYE_XML_INCHAR */

    LYVE_EOF,          /* LYE_EOF */
    LYVE_INSTMT,       /* LYE_INSTMT */
    LYVE_INSTMT,       /* LYE_INCHILDSTMT */
    LYVE_INID,         /* LYE_INID */
    LYVE_INDATE,       /* LYE_INDATE */
    LYVE_INARG,        /* LYE_INARG */
    LYVE_MISSSTMT,     /* LYE_MISSCHILDSTMT */
    LYVE_MISSSTMT,     /* LYE_MISSSTMT */
    LYVE_MISSARG,      /* LYE_MISSARG */
    LYVE_TOOMANY,      /* LYE_TOOMANY */
    LYVE_DUPID,        /* LYE_DUPID */
    LYVE_DUPLEAFLIST,  /* LYE_DUPLEAFLIST */
    LYVE_DUPLIST,      /* LYE_DUPLIST */
    LYVE_ENUM_DUPVAL,  /* LYE_ENUM_DUPVAL */
    LYVE_ENUM_DUPNAME, /* LYE_ENUM_DUPNAME */
    LYVE_ENUM_WS,      /* LYE_ENUM_WS */
    LYVE_BITS_DUPVAL,  /* LYE_BITS_DUPVAL */
    LYVE_BITS_DUPNAME, /* LYE_BITS_DUPNAME */
    LYVE_INMOD,        /* LYE_INMOD */
    LYVE_INMOD,        /* LYE_INMOD_LEN */
    LYVE_KEY_NLEAF,    /* LYE_KEY_NLEAF */
    LYVE_KEY_TYPE,     /* LYE_KEY_TYPE */
    LYVE_KEY_CONFIG,   /* LYE_KEY_CONFIG */
    LYVE_KEY_MISS,     /* LYE_KEY_MISS */
    LYVE_KEY_DUP,      /* LYE_KEY_DUP */
    LYVE_INREGEX,      /* LYE_INREGEX */
    LYVE_INRESOLV,     /* LYE_INRESOLV */
    LYVE_INSTATUS,     /* LYE_INSTATUS */

    LYVE_OBSDATA,      /* LYE_OBSDATA */
    LYVE_OBSDATA,      /* LYE_OBSTYPE */
    LYVE_NORESOLV,     /* LYE_NORESOLV */
    LYVE_INELEM,       /* LYE_INELEM */
    LYVE_INELEM,       /* LYE_INELEM_LEN */
    LYVE_MISSELEM,     /* LYE_MISSELEM */
    LYVE_INVAL,        /* LYE_INVAL */
    LYVE_INATTR,       /* LYE_INATTR */
    LYVE_MISSATTR,     /* LYE_MISSATTR */
    LYVE_OORVAL,       /* LYE_OORVAL */
    LYVE_INCHAR,       /* LYE_INCHAR */
    LYVE_INPRED,       /* LYE_INPRED */
    LYVE_MCASEDATA,    /* LYE_MCASEDATA */
    LYVE_NOCOND,       /* LYE_NOCOND */
    LYVE_INORDER,      /* LYE_INORDER */
    LYVE_INCOUNT,      /* LYE_INCOUNT */

    LYVE_XPATH_INTOK,  /* LYE_XPATH_INTOK */
    LYVE_XPATH_EOF,    /* LYE_XPATH_EOF */
    LYVE_XPATH_INOP,   /* LYE_XPATH_INOP_1 */
    LYVE_XPATH_INOP,   /* LYE_XPATH_INOP_2 */
    LYVE_XPATH_INCTX,  /* LYE_XPATH_INCTX */
    LYVE_XPATH_INARGCOUNT, /* LYE_XPATH_INARGCOUNT */
    LYVE_XPATH_INARGTYPE, /* LYE_XPATH_INARGTYPE */

    LYVE_PATH_INCHAR,  /* LYE_PATH_INCHAR */
    LYVE_PATH_INMOD,   /* LYE_PATH_INMOD */
    LYVE_PATH_MISSMOD, /* LYE_PATH_MISSMOD */
    LYVE_PATH_INNODE,  /* LYE_PATH_INNODE */
    LYVE_PATH_NLIST,   /* LYE_PATH_NLIST */
    LYVE_PATH_INKEY,   /* LYE_PATH_INKEY */
    LYVE_PATH_MISSKEY, /* LYE_PATH_MISSKEY */
    LYVE_PATH_EXISTS,  /* LYE_PATH_EXISTS */
    LYVE_PATH_MISSPAR, /* LYE_PATH_MISSPAR */
};

void
ly_vlog(LY_ECODE code, unsigned int line, enum LY_VLOG_ELEM elem_type, const void *elem, ...)
{
    va_list ap;
    const char *fmt;
    char line_msg[41];
    char* path;
    int *index;
    int i;
    const void *iter = elem;
    struct lys_node_list *slist;
    struct lyd_node *dlist, *diter;
    const char *name, *prefix = NULL;
    size_t len;

    if (line == UINT_MAX) {
        return;
    }

    ly_errno = LY_EVALID;
    if (line) {
        if (ly_log_clb) {
            sprintf(line_msg, "Parser fails around the line %u.", line);
            ly_log_clb(LY_LLERR, line_msg, NULL);
        } else {
            fprintf(stderr, "libyang[%d]: Parser fails around the line %u.\n", LY_LLERR, line);
        }
    }

    if (code == LYE_LINE || (code == LYE_PATH && !path_flag)) {
        return;
    }
    if (code > 0) {
        ly_vecode = ecode2vecode[code];
    }

    /* resolve path */
    path = ((struct ly_err *)&ly_errno)->path;
    index = &((struct ly_err *)&ly_errno)->path_index;
    (*index) = LY_ERR_MSG_SIZE - 1;
    path[(*index)] = '\0';
    if (path_flag && elem_type) { /* != LY_VLOG_NONE */
        if (!iter) {
            /* top-level */
            path[--(*index)] = '/';
        } else while (iter) {
            switch (elem_type) {
            case LY_VLOG_XML:
                name = ((struct lyxml_elem *)iter)->name;
                prefix = ((struct lyxml_elem *)iter)->ns ? ((struct lyxml_elem *)iter)->ns->prefix : NULL;
                iter = ((struct lyxml_elem *)iter)->parent;
                break;
            case LY_VLOG_LYS:
                name = ((struct lys_node *)iter)->name;
                iter = ((struct lys_node *)iter)->parent;
                break;
            case LY_VLOG_LYD:
                name = ((struct lyd_node *)iter)->schema->name;
                if (!((struct lyd_node *)iter)->parent ||
                        ((struct lyd_node *)iter)->schema->module != ((struct lyd_node *)iter)->parent->schema->module) {
                    prefix = ((struct lyd_node *)iter)->schema->module->name;
                }

                /* handle predicates (keys) in case of lists */
                if (((struct lyd_node *)iter)->schema->nodetype == LYS_LIST) {
                    dlist = (struct lyd_node *)iter;
                    slist = (struct lys_node_list *)((struct lyd_node *)iter)->schema;
                    for (i = 0; i < slist->keys_size; i++) {
                        LY_TREE_FOR(dlist->child, diter) {
                            if (diter->schema == (struct lys_node *)slist->keys[i]) {
                                break;
                            }
                        }
                        if (diter && ((struct lyd_node_leaf_list *)diter)->value_str) {
                            (*index) -= 2;
                            memcpy(&path[(*index)], "']", 2);
                            len = strlen(((struct lyd_node_leaf_list *)diter)->value_str);
                            (*index) -= len;
                            memcpy(&path[(*index)], ((struct lyd_node_leaf_list *)diter)->value_str, len);
                            (*index) -=2;
                            memcpy(&path[(*index)], "='", 2);
                            len = strlen(diter->schema->name);
                            (*index) -= len;
                            memcpy(&path[(*index)], diter->schema->name, len);
                            if (dlist->schema->module != diter->schema->module) {
                                path[--(*index)] = ':';
                                len = strlen(diter->schema->module->name);
                                (*index) -= len;
                                memcpy(&path[(*index)], diter->schema->module->name, len);
                            }
                            path[--(*index)] = '[';
                        }
                    }
                }

                iter = ((struct lyd_node *)iter)->parent;
                break;
            default:
                /* shouldn't be here */
                iter = NULL;
                continue;
            }
            len = strlen(name);
            (*index) = (*index) - len;
            memcpy(&path[(*index)], name, len);
            if (prefix) {
                path[--(*index)] = ':';
                len = strlen(prefix);
                (*index) = (*index) - len;
                memcpy(&path[(*index)], prefix, len);
            }
            path[--(*index)] = '/';
        }
    }

    va_start(ap, elem);
    switch (code) {
    case LYE_SPEC:
        fmt = va_arg(ap, char *);
        log_vprintf(LY_LLERR, fmt, path[(*index)] ? &path[(*index)] : NULL, ap);
        break;
    case LYE_PATH:
        log_vprintf(LY_LLERR, NULL, &path[(*index)], ap);
        break;
    default:
        log_vprintf(LY_LLERR, ly_errs[code], path[(*index)] ? &path[(*index)] : NULL, ap);
        break;
    }
    va_end(ap);
}
