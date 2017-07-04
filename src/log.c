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

#define _GNU_SOURCE
#define _BSD_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "common.h"
#include "tree_internal.h"

extern LY_ERR ly_errno_int;
volatile int8_t ly_log_level = LY_LLERR;
static void (*ly_log_clb)(LY_LOG_LEVEL level, const char *msg, const char *path);
static volatile int path_flag = 1;
#ifndef NDEBUG
volatile int ly_log_dbg_groups = 0;
#endif

API void
ly_verb(LY_LOG_LEVEL level)
{
    ly_log_level = level;
}

API void
ly_verb_dbg(int dbg_groups)
{
#ifndef NDEBUG
    ly_log_dbg_groups = dbg_groups;
#else
    (void)dbg_groups;
#endif
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
log_vprintf(LY_LOG_LEVEL level, uint8_t hide, const char *format, const char *path, va_list args)
{
    char *msg, *bufdup = NULL;
    struct ly_err *e = ly_err_location();
    struct ly_err_item *eitem;

    if (&ly_errno == &ly_errno_int) {
        msg = "Internal logger error";
    } else if (!format) {
        /* postponed print of path related to the previous error, do not rewrite stored original message */
        msg = "Path is related to the previous error message.";
    } else {
        if (level == LY_LLERR) {
            /* store error message into msg buffer ... */
            msg = e->msg;
        } else if (!hide) {
            /* other messages are stored in working string buffer and not available for later access */
            msg = e->buf;
            if (ly_buf_used && msg[0]) {
                bufdup = strndup(msg, LY_BUF_SIZE - 1);
            }
        } else { /* hide */
            return;
        }
        if (msg != format) {
            vsnprintf(msg, LY_BUF_SIZE - 1, format, args);
            msg[LY_BUF_SIZE - 1] = '\0';
        }
    }

    if (level == LY_LLERR) {
        if (!path) {
            /* erase previous path */
            e->path_index = LY_BUF_SIZE - 1;
        }

        /* if the error-app-tag should be set, do it after calling LOGVAL */
        e->apptag[0] = '\0';

        /* store error information into a list */
        if (!e->errlist) {
            eitem = e->errlist = malloc(sizeof *eitem);
        } else {
            for (eitem = e->errlist; eitem->next; eitem = eitem->next);
            eitem->next = malloc(sizeof *eitem->next);
            eitem = eitem->next;
        }
        if (eitem) {
            eitem->no = ly_errno;
            eitem->code = ly_vecode;
            eitem->msg = strdup(msg);
            if (path) {
                eitem->path = strdup(path);
            } else {
                eitem->path = NULL;
            }
            eitem->next = NULL;
        }
    }

    if (hide || (level > ly_log_level)) {
        /* do not print the message */
        goto clean;
    }

    if (ly_log_clb) {
        ly_log_clb(level, msg, path);
    } else {
        fprintf(stderr, "libyang[%d]: %s%s", level, msg, path ? " " : "\n");
        if (path) {
            fprintf(stderr, "(path: %s)\n", path);
        }
    }

clean:
    if (bufdup) {
        /* return previous internal buffer content */
        strncpy(msg, bufdup, LY_BUF_SIZE - 1);
        free(bufdup);
    }
}

void
ly_log(LY_LOG_LEVEL level, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    log_vprintf(level, (*ly_vlog_hide_location()), format, NULL, ap);
    va_end(ap);
}

#ifndef NDEBUG

void
ly_log_dbg(LY_LOG_DBG_GROUP group, const char *format, ...)
{
    char *dbg_format;
    const char *str_group;
    va_list ap;

    if (!(ly_log_dbg_groups & group)) {
        return;
    }

    switch (group) {
    case LY_LDGDICT:
        str_group = "DICT";
        break;
    case LY_LDGYANG:
        str_group = "YANG";
        break;
    case LY_LDGYIN:
        str_group = "YIN";
        break;
    case LY_LDGXPATH:
        str_group = "XPATH";
        break;
    case LY_LDGDIFF:
        str_group = "DIFF";
        break;
    default:
        LOGINT;
        return;
    }

    if (asprintf(&dbg_format, "%s: %s", str_group, format) == -1) {
        LOGMEM;
        return;
    }

    va_start(ap, format);
    log_vprintf(LY_LLDBG, (*ly_vlog_hide_location()), dbg_format, NULL, ap);
    va_end(ap);
}

#endif

void
lyext_log(LY_LOG_LEVEL level, const char *plugin, const char *function, const char *format, ...)
{
    va_list ap;
    char *plugin_msg;

    if (level == LY_LLERR) {
        /* set errno */
        ly_errno = LY_EEXT;
    }
    if (ly_log_level < level) {
        return;
    }

    if (asprintf(&plugin_msg, "%s (reported by extension plugin %s, %s())", format, plugin, function) == -1) {
        LOGMEM;
        return;
    }

    va_start(ap, format);
    log_vprintf(level, (*ly_vlog_hide_location()), plugin_msg, NULL, ap);
    va_end(ap);

    free(plugin_msg);
}

const char *ly_errs[] = {
/* LYE_SUCCESS */      "",
/* LYE_XML_MISS */     "Missing %s \"%s\".",
/* LYE_XML_INVAL */    "Invalid %s.",
/* LYE_XML_INCHAR */   "Encountered invalid character sequence \"%.10s\".",

/* LYE_EOF */          "Unexpected end of input data.",
/* LYE_INSTMT */       "Invalid keyword \"%s\".",
/* LYE_INCHILDSTMT */  "Invalid keyword \"%s\" as a child to \"%s\".",
/* LYE_INPAR */        "Invalid ancestor \"%s\" of \"%s\".",
/* LYE_INID */         "Invalid identifier \"%s\" (%s).",
/* LYE_INDATE */       "Invalid date format of \"%s\", \"YYYY-MM-DD\" expected.",
/* LYE_INARG */        "Invalid value \"%s\" of \"%s\".",
/* LYE_MISSSTMT */     "Missing keyword \"%s\".",
/* LYE_MISSCHILDSTMT */ "Missing keyword \"%s\" as a child to \"%s\".",
/* LYE_MISSARG */      "Missing argument \"%s\" to keyword \"%s\".",
/* LYE_TOOMANY */      "Too many instances of \"%s\" in \"%s\".",
/* LYE_DUPID */        "Duplicated %s identifier \"%s\".",
/* LYE_DUPLEAFLIST */  "Duplicated instance of \"%s\" leaf-list (\"%s\").",
/* LYE_DUPLIST */      "Duplicated instance of \"%s\" list.",
/* LYE_NOUNIQ */       "Unique data leaf(s) \"%s\" not satisfied in \"%s\" and \"%s\".",
/* LYE_ENUM_INVAL */   "Invalid value \"%d\" of \"%s\" enum, restricted enum value does not match the base type value \"%d\".",
/* LYE_ENUM_INNAME */  "Adding new enum name \"%s\" in restricted enumeration type is not allowed.",
/* LYE_ENUM_DUPVAL */  "The value \"%d\" of \"%s\" enum has already been assigned to \"%s\" enum.",
/* LYE_ENUM_DUPNAME */ "The enum name \"%s\" has already been assigned to another enum.",
/* LYE_ENUM_WS */      "The enum name \"%s\" includes invalid leading or trailing whitespaces.",
/* LYE_BITS_INVAL */   "Invalid position \"%d\" of \"%s\" bit, restricted bits position does not match the base type position \"%d\".",
/* LYE_BITS_INNAME */  "Adding new bit name \"%s\" in restricted bits type is not allowed.",
/* LYE_BITS_DUPVAL */  "The position \"%d\" of \"%s\" bit has already been assigned to \"%s\" bit.",
/* LYE_BITS_DUPNAME */ "The bit name \"%s\" has already been assigned to another bit.",
/* LYE_INMOD */        "Module name \"%s\" refers to an unknown module.",
/* LYE_INMOD_LEN */    "Module name \"%.*s\" refers to an unknown module.",
/* LYE_KEY_NLEAF */    "Key \"%s\" is not a leaf.",
/* LYE_KEY_TYPE */     "Key \"%s\" must not be the built-in type \"empty\".",
/* LYE_KEY_CONFIG */   "The \"config\" value of the \"%s\" key differs from its list config value.",
/* LYE_KEY_MISS */     "Leaf \"%s\" defined as key in a list not found.",
/* LYE_KEY_DUP */      "Key identifier \"%s\" is not unique.",
/* LYE_INREGEX */      "Regular expression \"%s\" is not valid (\"%s\": %s).",
/* LYE_INRESOLV */     "Failed to resolve %s \"%s\".",
/* LYE_INSTATUS */     "A %s definition \"%s\" references %s definition \"%s\".",
/* LYE_CIRC_LEAFREFS */"A circular chain of leafrefs detected.",
/* LYE_CIRC_FEATURES */"A circular chain features detected in \"%s\" feature.",
/* LYE_CIRC_IMPORTS */ "A circular dependency (import) for module \"%s\".",
/* LYE_CIRC_INCLUDES */"A circular dependency (include) for submodule \"%s\".",
/* LYE_INVER */        "Different YANG versions of a submodule and its main module.",
/* LYE_SUBMODULE */    "Unable to parse submodule, parse the main module instead.",

/* LYE_OBSDATA */      "Obsolete data \"%s\" instantiated.",
/* LYE_OBSTYPE */      "Data node \"%s\" with obsolete type \"%s\" instantiated.",
/* LYE_NORESOLV */     "No resolvents found for %s \"%s\".",
/* LYE_INELEM */       "Unknown element \"%s\".",
/* LYE_INELEM_LEN */   "Unknown element \"%.*s\".",
/* LYE_MISSELEM */     "Missing required element \"%s\" in \"%s\".",
/* LYE_INVAL */        "Invalid value \"%s\" in \"%s\" element.",
/* LYE_INMETA */       "Invalid \"%s:%s\" metadata with value \"%s\".",
/* LYE_INATTR */       "Invalid attribute \"%s\".",
/* LYE_MISSATTR */     "Missing attribute \"%s\" in \"%s\" element.",
/* LYE_NOCONSTR */     "Value \"%s\" does not satisfy the constraint \"%s\" (range, length, or pattern).",
/* LYE_INCHAR */       "Unexpected character(s) '%c' (%.15s).",
/* LYE_INPRED */       "Predicate resolution failed on \"%s\".",
/* LYE_MCASEDATA */    "Data for more than one case branch of \"%s\" choice present.",
/* LYE_NOMUST */       "Must condition \"%s\" not satisfied.",
/* LYE_NOWHEN */       "When condition \"%s\" not satisfied.",
/* LYE_INORDER */      "Invalid order of elements \"%s\" and \"%s\".",
/* LYE_INWHEN */       "Irresolvable when condition \"%s\".",
/* LYE_NOMIN */        "Too few \"%s\" elements.",
/* LYE_NOMAX */        "Too many \"%s\" elements.",
/* LYE_NOREQINS */     "Required instance of \"%s\" does not exists.",
/* LYE_NOLEAFREF */    "Leafref \"%s\" of value \"%s\" points to a non-existing leaf.",
/* LYE_NOMANDCHOICE */ "Mandatory choice \"%s\" missing a case branch.",

/* LYE_XPATH_INSNODE */"Schema node \"%.*s\" not found (%.*s).",
/* LYE_XPATH_INTOK */  "Unexpected XPath token %s (%.15s).",
/* LYE_XPATH_EOF */    "Unexpected XPath expression end.",
/* LYE_XPATH_INOP_1 */ "Cannot apply XPath operation %s on %s.",
/* LYE_XPATH_INOP_2 */ "Cannot apply XPath operation %s on %s and %s.",
/* LYE_XPATH_INCTX */  "Invalid context type %s in %s.",
/* LYE_XPATH_INMOD */  "Unknown module \"%.*s\".",
/* LYE_XPATH_INFUNC */ "Unknown XPath function \"%.*s\".",
/* LYE_XPATH_INARGCOUNT */ "Invalid number of arguments (%d) for the XPath function %.*s.",
/* LYE_XPATH_INARGTYPE */ "Wrong type of argument #%d (%s) for the XPath function %s.",
/* LYE_XPATH_DUMMY */   "Accessing the value of the dummy node \"%s\".",
/* LYE_XPATH_NOEND */   "Unterminated string delimited with %c (%.15s).",

/* LYE_PATH_INCHAR */  "Unexpected character(s) '%c' (\"%s\").",
/* LYE_PATH_INMOD */   "Module not found.",
/* LYE_PATH_MISSMOD */ "Missing module name.",
/* LYE_PATH_INNODE */  "Schema node not found.",
/* LYE_PATH_INKEY */   "List key not found or on incorrect position (\"%s\").",
/* LYE_PATH_MISSKEY */ "List keys or position missing (\"%s\").",
/* LYE_PATH_EXISTS */  "Node already exists.",
/* LYE_PATH_MISSPAR */ "Parent does not exist.",
};

static const LY_VECODE ecode2vecode[] = {
    LYVE_SUCCESS,      /* LYE_SUCCESS */

    LYVE_XML_MISS,     /* LYE_XML_MISS */
    LYVE_XML_INVAL,    /* LYE_XML_INVAL */
    LYVE_XML_INCHAR,   /* LYE_XML_INCHAR */

    LYVE_EOF,          /* LYE_EOF */
    LYVE_INSTMT,       /* LYE_INSTMT */
    LYVE_INSTMT,       /* LYE_INCHILDSTMT */
    LYVE_INPAR,        /* LYE_INPAR */
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
    LYVE_NOUNIQ,       /* LYE_NOUNIQ */
    LYVE_ENUM_INVAL,   /* LYE_ENUM_INVAL */
    LYVE_ENUM_INNAME,  /* LYE_ENUM_INNAME */
    LYVE_ENUM_INVAL,   /* LYE_ENUM_DUPVAL */
    LYVE_ENUM_INNAME,  /* LYE_ENUM_DUPNAME */
    LYVE_ENUM_WS,      /* LYE_ENUM_WS */
    LYVE_BITS_INVAL,   /* LYE_BITS_INVAL */
    LYVE_BITS_INNAME,  /* LYE_BITS_INNAME */
    LYVE_BITS_INVAL,   /* LYE_BITS_DUPVAL */
    LYVE_BITS_INNAME,  /* LYE_BITS_DUPNAME */
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
    LYVE_CIRC_LEAFREFS,/* LYE_CIRC_LEAFREFS */
    LYVE_CIRC_FEATURES,/* LYE_CIRC_FEATURES */
    LYVE_CIRC_IMPORTS, /* LYE_CIRC_IMPORTS */
    LYVE_CIRC_INCLUDES,/* LYE_CIRC_INCLUDES */
    LYVE_INVER,        /* LYE_INVER */
    LYVE_SUBMODULE,    /* LYE_SUBMODULE */

    LYVE_OBSDATA,      /* LYE_OBSDATA */
    LYVE_OBSDATA,      /* LYE_OBSTYPE */
    LYVE_NORESOLV,     /* LYE_NORESOLV */
    LYVE_INELEM,       /* LYE_INELEM */
    LYVE_INELEM,       /* LYE_INELEM_LEN */
    LYVE_MISSELEM,     /* LYE_MISSELEM */
    LYVE_INVAL,        /* LYE_INVAL */
    LYVE_INMETA,       /* LYE_INMETA */
    LYVE_INATTR,       /* LYE_INATTR */
    LYVE_MISSATTR,     /* LYE_MISSATTR */
    LYVE_NOCONSTR,     /* LYE_NOCONSTR */
    LYVE_INCHAR,       /* LYE_INCHAR */
    LYVE_INPRED,       /* LYE_INPRED */
    LYVE_MCASEDATA,    /* LYE_MCASEDATA */
    LYVE_NOMUST,       /* LYE_NOMUST */
    LYVE_NOWHEN,       /* LYE_NOWHEN */
    LYVE_INORDER,      /* LYE_INORDER */
    LYVE_INWHEN,       /* LYE_INWHEN */
    LYVE_NOMIN,        /* LYE_NOMIN */
    LYVE_NOMAX,        /* LYE_NOMAX */
    LYVE_NOREQINS,     /* LYE_NOREQINS */
    LYVE_NOLEAFREF,    /* LYE_NOLEAFREF */
    LYVE_NOMANDCHOICE, /* LYE_NOMANDCHOICE */

    LYVE_XPATH_INSNODE,/* LYE_XPATH_INSNODE */
    LYVE_XPATH_INTOK,  /* LYE_XPATH_INTOK */
    LYVE_XPATH_EOF,    /* LYE_XPATH_EOF */
    LYVE_XPATH_INOP,   /* LYE_XPATH_INOP_1 */
    LYVE_XPATH_INOP,   /* LYE_XPATH_INOP_2 */
    LYVE_XPATH_INCTX,  /* LYE_XPATH_INCTX */
    LYVE_XPATH_INMOD,  /* LYE_XPATH_INMOD */
    LYVE_XPATH_INFUNC, /* LYE_XPATH_INFUNC */
    LYVE_XPATH_INARGCOUNT, /* LYE_XPATH_INARGCOUNT */
    LYVE_XPATH_INARGTYPE, /* LYE_XPATH_INARGTYPE */
    LYVE_XPATH_DUMMY,  /* LYE_XPATH_DUMMY */
    LYVE_XPATH_NOEND,  /* LYE_XPATH_NOEND */

    LYVE_PATH_INCHAR,  /* LYE_PATH_INCHAR */
    LYVE_PATH_INMOD,   /* LYE_PATH_INMOD */
    LYVE_PATH_MISSMOD, /* LYE_PATH_MISSMOD */
    LYVE_PATH_INNODE,  /* LYE_PATH_INNODE */
    LYVE_PATH_INKEY,   /* LYE_PATH_INKEY */
    LYVE_PATH_MISSKEY, /* LYE_PATH_MISSKEY */
    LYVE_PATH_EXISTS,  /* LYE_PATH_EXISTS */
    LYVE_PATH_MISSPAR, /* LYE_PATH_MISSPAR */
};


void
ly_vlog_hide(uint8_t hide)
{
    (*ly_vlog_hide_location()) = hide;
}

void
ly_vlog_build_path_reverse(enum LY_VLOG_ELEM elem_type, const void *elem, char *path, uint16_t *index, int prefix_all)
{
    int i, j;
    struct lys_node_list *slist;
    struct lys_node *sparent;
    struct lyd_node *dlist, *diter;
    struct lys_module *top_module = NULL;
    const char *name, *prefix = NULL, *val_end, *val_start;
    char *str;
    size_t len;

    while (elem) {
        switch (elem_type) {
        case LY_VLOG_XML:
            name = ((struct lyxml_elem *)elem)->name;
            prefix = ((struct lyxml_elem *)elem)->ns ? ((struct lyxml_elem *)elem)->ns->prefix : NULL;
            elem = ((struct lyxml_elem *)elem)->parent;
            break;
        case LY_VLOG_LYS:
            if (!top_module) {
                /* find and store the top-level node module */
                if (((struct lys_node *)elem)->nodetype == LYS_EXT) {
                    top_module = ((struct lys_ext_instance *)elem)->module;
                } else {
                    for (sparent = (struct lys_node *)elem; lys_parent(sparent); sparent = lys_parent(sparent));
                    top_module = lys_node_module(sparent);
                }
            }

            if (((struct lys_node *)elem)->nodetype & (LYS_AUGMENT | LYS_GROUPING)) {
                --(*index);
                path[*index] = ']';

                name = ((struct lys_node *)elem)->name;
                len = strlen(name);
                (*index) -= len;
                memcpy(&path[*index], name, len);

                if (((struct lys_node *)elem)->nodetype == LYS_GROUPING) {
                    name = "{grouping}[";
                } else { /* augment */
                    name = "{augment}[";
                }
            } else if (((struct lys_node *)elem)->nodetype == LYS_EXT) {
                name = ((struct lys_ext_instance *)elem)->def->name;
            } else {
                name = ((struct lys_node *)elem)->name;
            }

            if (prefix_all || !lys_parent((struct lys_node *)elem) || (lys_node_module((struct lys_node *)elem) != top_module)) {
                prefix = lys_node_module((struct lys_node *)elem)->name;
            } else {
                prefix = NULL;
            }
            if (((struct lys_node *)elem)->nodetype == LYS_EXT) {
                if (((struct lys_ext_instance*)elem)->parent_type == LYEXT_PAR_NODE) {
                    elem = (struct lys_node*)((struct lys_ext_instance*)elem)->parent;
                } else {
                    sparent = NULL;
                    elem = NULL;
                }
                break;
            }

            /* need to find the parent again because we don't want to skip augments */
            do {
                sparent = ((struct lys_node *)elem)->parent;
                elem = lys_parent((struct lys_node *)elem);
            } while (elem && (((struct lys_node *)elem)->nodetype == LYS_USES));
            break;
        case LY_VLOG_LYD:
            if (!top_module) {
                /* find and store the top-level node module */
                for (diter = (struct lyd_node *)elem; diter->parent; diter = diter->parent);
                top_module = lyd_node_module(diter);
            }

            name = ((struct lyd_node *)elem)->schema->name;
            if (prefix_all || !((struct lyd_node *)elem)->parent || (lyd_node_module((struct lyd_node *)elem) != top_module)) {
                prefix = lyd_node_module((struct lyd_node *)elem)->name;
            } else {
                prefix = NULL;
            }

            /* handle predicates (keys) in case of lists */
            if (((struct lyd_node *)elem)->schema->nodetype == LYS_LIST) {
                dlist = (struct lyd_node *)elem;
                slist = (struct lys_node_list *)((struct lyd_node *)elem)->schema;
                if (slist->keys_size) {
                    /* schema list with keys - use key values in predicates */
                    for (i = slist->keys_size - 1; i > -1; i--) {
                        LY_TREE_FOR(dlist->child, diter) {
                            if (diter->schema == (struct lys_node *)slist->keys[i]) {
                                break;
                            }
                        }
                        if (diter && ((struct lyd_node_leaf_list *)diter)->value_str) {
                            if (strchr(((struct lyd_node_leaf_list *)diter)->value_str, '\'')) {
                                val_start = "=\"";
                                val_end = "\"]";
                            } else {
                                val_start = "='";
                                val_end = "']";
                            }

                            (*index) -= 2;
                            memcpy(&path[(*index)], val_end, 2);
                            len = strlen(((struct lyd_node_leaf_list *)diter)->value_str);
                            (*index) -= len;
                            memcpy(&path[(*index)], ((struct lyd_node_leaf_list *)diter)->value_str, len);
                            (*index) -= 2;
                            memcpy(&path[(*index)], val_start, 2);
                            len = strlen(diter->schema->name);
                            (*index) -= len;
                            memcpy(&path[(*index)], diter->schema->name, len);
                            if (prefix_all || (lyd_node_module(diter) != top_module)) {
                                path[--(*index)] = ':';
                                len = strlen(lyd_node_module(diter)->name);
                                (*index) -= len;
                                memcpy(&path[(*index)], lyd_node_module(diter)->name, len);
                            }
                            path[--(*index)] = '[';
                        }
                    }
                } else {
                    /* schema list without keys - use instance position */
                    path[--(*index)] = ']';

                    i = j = lyd_list_pos(dlist);
                    len = 1;
                    while (j > 9) {
                        ++len;
                        j /= 10;
                    }

                    str = malloc(len + 1);
                    LY_CHECK_ERR_RETURN(!str, LOGMEM, );
                    sprintf(str, "%d", i);

                    (*index) -= len;
                    strncpy(&path[(*index)], str, len);

                    free(str);

                    --(*index);
                    path[*index] = '[';
                }
            } else if (((struct lyd_node *)elem)->schema->nodetype == LYS_LEAFLIST &&
                    ((struct lyd_node_leaf_list *)elem)->value_str) {
                if (strchr(((struct lyd_node_leaf_list *)elem)->value_str, '\'')) {
                    val_start = "[.=\"";
                    val_end = "\"]";
                } else {
                    val_start = "[.='";
                    val_end = "']";
                }

                (*index) -= 2;
                memcpy(&path[(*index)], val_end, 2);
                len = strlen(((struct lyd_node_leaf_list *)elem)->value_str);
                (*index) -= len;
                memcpy(&path[(*index)], ((struct lyd_node_leaf_list *)elem)->value_str, len);
                (*index) -= 4;
                memcpy(&path[(*index)], val_start, 4);
            }

            elem = ((struct lyd_node *)elem)->parent;
            break;
        case LY_VLOG_STR:
            len = strlen((const char *)elem) + 1;
            if (len > LY_BUF_SIZE) {
                len = LY_BUF_SIZE - 1;
            }
            (*index) = LY_BUF_SIZE - len;
            memcpy(&path[(*index)], (const char *)elem, len - 1);
            return;
        default:
            /* shouldn't be here */
            LOGINT;
            return;
        }
        if (name) {
            len = strlen(name);
            (*index) -= len;
            memcpy(&path[*index], name, len);
            if (prefix) {
                path[--(*index)] = ':';
                len = strlen(prefix);
                (*index) = (*index) - len;
                memcpy(&path[(*index)], prefix, len);
            }
        }
        path[--(*index)] = '/';
        if (elem_type == LY_VLOG_LYS && !elem && sparent && sparent->nodetype == LYS_AUGMENT) {
            len = strlen(((struct lys_node_augment *)sparent)->target_name);
            (*index) = (*index) - len;
            memcpy(&path[(*index)], ((struct lys_node_augment *)sparent)->target_name, len);
        }
    }
}

void
ly_vlog(LY_ECODE code, enum LY_VLOG_ELEM elem_type, const void *elem, ...)
{
    va_list ap;
    const char *fmt;
    char* path = NULL;
    uint16_t *index = NULL;

    ly_errno = LY_EVALID;

    if ((code == LYE_PATH) && !path_flag) {
        return;
    }
    if (code > 0) {
        ly_vecode = ecode2vecode[code];
    }

    if (!path_flag) {
        goto log;
    }

    /* resolve path */
    path = ((struct ly_err *)&ly_errno)->path;
    index = &((struct ly_err *)&ly_errno)->path_index;
    if ((elem_type != LY_VLOG_NONE) && (elem_type != LY_VLOG_PREV)) { /* != LY_VLOG_NONE */
        /* update path */
        (*index) = LY_BUF_SIZE - 1;
        path[(*index)] = '\0';
        if (!elem) {
            /* top-level */
            path[--(*index)] = '/';
        } else {
            ly_vlog_build_path_reverse(elem_type, elem, path, index, 0);
        }
    } else if (elem_type == LY_VLOG_NONE) {
        /* erase path, the rest will be erased by log_vprintf() since it will get NULL path parameter */
        path[(*index)] = '\0';
    }

log:
    va_start(ap, elem);
    switch (code) {
    case LYE_SPEC:
        fmt = va_arg(ap, char *);
        log_vprintf(LY_LLERR, (*ly_vlog_hide_location()), fmt, index && path[(*index)] ? &path[(*index)] : NULL, ap);
        break;
    case LYE_PATH:
        log_vprintf(LY_LLERR, (*ly_vlog_hide_location()), NULL, &path[(*index)], ap);
        break;
    default:
        log_vprintf(LY_LLERR, (*ly_vlog_hide_location()), ly_errs[code],
                    index && path[(*index)] ? &path[(*index)] : NULL, ap);
        break;
    }
    va_end(ap);
}

void
ly_err_repeat(void)
{
    struct ly_err_item *i;

    if ((ly_log_level >= LY_LLERR) && !*ly_vlog_hide_location()) {
        for (i = ly_err_location()->errlist; i; i = i->next) {
            if (ly_log_clb) {
                ly_log_clb(LY_LLERR, i->msg, i->path);
            } else {
                fprintf(stderr, "libyang[%d]: %s%s", LY_LLERR, i->msg, i->path ? " " : "\n");
                if (i->path) {
                    fprintf(stderr, "(path: %s)\n", i->path);
                }
            }
        }
    }
}
