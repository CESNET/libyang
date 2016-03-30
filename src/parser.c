/**
 * @file parser.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief common libyang parsers routines implementations
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
#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pcre.h>

#include "common.h"
#include "context.h"
#include "libyang.h"
#include "parser.h"
#include "resolve.h"
#include "tree_internal.h"
#include "parser_yang.h"

int
lyp_is_rpc(struct lys_node *node)
{
    assert(node);

    while(node->parent) {
        node = node->parent;
    }

    if (node->nodetype == LYS_RPC) {
        return 1;
    } else {
        return 0;
    }
}

int
lyp_check_options(int options)
{
    int x = options & LYD_OPT_TYPEMASK;

    /* LYD_OPT_NOAUTODEL can be used only with LYD_OPT_DATA or LYD_OPT_CONFIG */
    if (options & LYD_OPT_NOAUTODEL) {
        if (x != LYD_OPT_DATA && x != LYD_OPT_CONFIG) {
            return 1;
        }
    }

    /* "is power of 2" algorithm, with 0 exception */
    return x ? !(x && !(x & (x - 1))) : 0;
}

void
lyp_set_implemented(struct lys_module *module)
{
    int i;

    assert(module);
    module->implemented = 1;

    for (i = 0; i < module->inc_size && module->inc[i].submodule; i++) {
        lyp_set_implemented((struct lys_module *)module->inc[i].submodule);
    }
}

/**
 * @brief Alternative for lys_read() + lys_parse() in case of import
 *
 * @param[in] fd MUST be a regular file (will be used by mmap)
 */
struct lys_module *
lys_read_import(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, const char *revision)
{
    struct lys_module *module = NULL;
    struct stat sb;
    char *addr;

    if (!ctx || fd < 0) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }


    if (fstat(fd, &sb) == -1) {
        LOGERR(LY_ESYS, "Failed to stat the file descriptor (%s).", strerror(errno));
        return NULL;
    }
    addr = mmap(NULL, sb.st_size + 2, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        LOGERR(LY_EMEM,"Map file into memory failed (%s()).",__func__);
        return NULL;
    }
    switch (format) {
    case LYS_IN_YIN:
        module = yin_read_module(ctx, addr, revision, 0);
        break;
    case LYS_IN_YANG:
        module = yang_read_module(ctx, addr, sb.st_size + 2, revision, 0);
        break;
    default:
        /* TODO */
        break;
    }
    munmap(addr, sb.st_size + 2);

    return module;
}

/* if module is !NULL, then the function searches for submodule */
struct lys_module *
lyp_search_file(struct ly_ctx *ctx, struct lys_module *module, const char *name, const char *revision,
                struct unres_schema *unres)
{
    size_t len, flen;
    int fd;
    char *wd, *cwd, *model_path;
    DIR *dir;
    struct dirent *file, *file_match;
    LYS_INFORMAT format, format_match;
    struct lys_module *result = NULL;
    int localsearch = 1;

    if (module) {
        /* searching for submodule, try if it is already loaded */
        result = (struct lys_module *)ly_ctx_get_submodule2(module, name);
        if (result) {
            if (!revision || (result->rev_size && ly_strequal(result->rev[0].date, revision, 0))) {
                /* success */
                return result;
            } else {
                /* there is already another revision of the submodule */
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, result->rev[0].date, "revision");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Multiple revisions of a submodule included.");
                return NULL;
            }
        }
    }

    len = strlen(name);
    cwd = wd = get_current_dir_name();

opendir_search:
    if (cwd != wd) {
        if (chdir(wd)) {
            LOGERR(LY_ESYS, "Unable to use search directory \"%s\" (%s)",
                   wd, strerror(errno));
            free(wd);
            wd = cwd;
            goto cleanup;
        }
    }
    dir = opendir(wd);
    LOGVRB("Searching for \"%s\" in %s.", name, wd);
    if (!dir) {
        LOGWRN("Unable to open directory \"%s\" for searching referenced modules (%s)",
               wd, strerror(errno));
        /* try search directory */
        goto searchpath;
    }

    file_match = NULL;
    format_match = 0;
    while ((file = readdir(dir))) {
        if (strncmp(name, file->d_name, len) ||
                (file->d_name[len] != '.' && file->d_name[len] != '@')) {
            continue;
        }

        /* get type according to filename suffix */
        flen = strlen(file->d_name);
        if (!strcmp(&file->d_name[flen - 4], ".yin")) {
            format = LYS_IN_YIN;
        } else if (!strcmp(&file->d_name[flen - 5], ".yang")) {
            format = LYS_IN_YANG;
        } else {
            continue;
        }

        if (revision) {
            if (file->d_name[len] == '@') {
                /* check revision from the filename */
                if (strncmp(revision, &file->d_name[len + 1], strlen(revision))) {
                    /* another revision */
                    continue;
                }
            } else {
                /* try to find exact revision match, use this only if not found */
                file_match = file;
                format_match = format;
                continue;
            }
        }

        file_match = file;
        format_match = format;
        break;
    }

    if (!file_match) {
        goto searchpath;
    }

    /* open the file */
    fd = open(file_match->d_name, O_RDONLY);
    if (fd < 0) {
        LOGERR(LY_ESYS, "Unable to open data model file \"%s\" (%s).",
               file_match->d_name, strerror(errno));
        goto cleanup;
    }

    if (module) {
        result = (struct lys_module *)lys_submodule_read(module, fd, format_match, unres);
    } else {
        result = lys_read_import(ctx, fd, format_match, revision);
    }
    close(fd);

    if (!result) {
        goto cleanup;
    }

    if (asprintf(&model_path, "%s/%s", wd, file_match->d_name) == -1) {
        LOGMEM;
        result = NULL;
        goto cleanup;
    }
    result->filepath = lydict_insert_zc(ctx, model_path);
    /* success */
    goto cleanup;

searchpath:
    if (!ctx->models.search_path) {
        LOGWRN("No search path defined for the current context.");
    } else if (!result && localsearch) {
        /* search in local directory done, try context's search_path */
        if (dir) {
            closedir(dir);
            dir = NULL;
        }
        wd = strdup(ctx->models.search_path);
        if (!wd) {
            dir = NULL;
            LOGMEM;
            goto cleanup;
        }
        localsearch = 0;
        goto opendir_search;
    }

    LOGERR(LY_ESYS, "Data model \"%s\" not found (search path is \"%s\")", name, ctx->models.search_path);

cleanup:
    if (cwd != wd) {
        if (chdir(cwd)) {
            LOGWRN("Unable to return back to working directory \"%s\" (%s)",
                   cwd, strerror(errno));
        }
        free(wd);
    }
    free(cwd);
    if (dir) {
        closedir(dir);
    }

    return result;
}

/* logs directly */
static int
parse_int(const char *val_str, int64_t min, int64_t max, int base, int64_t *ret, struct lyd_node *node)
{
    char *strptr;

    if (!val_str) {
        LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, "", node->schema->name);
        return EXIT_FAILURE;
    }

    /* convert to 64-bit integer, all the redundant characters are handled */
    errno = 0;
    strptr = NULL;
    *ret = strtoll(val_str, &strptr, base);
    if (errno || (*ret < min) || (*ret > max)) {
        LOGVAL(LYE_OORVAL, LY_VLOG_LYD, node, val_str, node->schema->name);
        return EXIT_FAILURE;
    } else if (strptr && *strptr) {
        while (isspace(*strptr)) {
            ++strptr;
        }
        if (*strptr) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, val_str, node->schema->name);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* logs directly */
static int
parse_uint(const char *val_str, uint64_t max, int base, uint64_t *ret, struct lyd_node *node)
{
    char *strptr;

    if (!val_str) {
        LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, "", node->schema->name);
        return EXIT_FAILURE;
    }

    errno = 0;
    strptr = NULL;
    *ret = strtoull(val_str, &strptr, base);
    if (errno || (*ret > max)) {
        LOGVAL(LYE_OORVAL, LY_VLOG_LYD, node, val_str, node->schema->name);
        return EXIT_FAILURE;
    } else if (strptr && *strptr) {
        while (isspace(*strptr)) {
            ++strptr;
        }
        if (*strptr) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, val_str, node->schema->name);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* logs directly
 *
 * kind == 0 - unsigned (unum used), 1 - signed (snum used), 2 - floating point (fnum used)
 */
static int
validate_length_range(uint8_t kind, uint64_t unum, int64_t snum, long double fnum, struct lys_type *type,
                      const char *val_str, struct lyd_node *node)
{
    struct len_ran_intv *intv = NULL, *tmp_intv;
    int ret = EXIT_FAILURE;

    if (resolve_len_ran_interval(NULL, type, 0, &intv)) {
        /* already done during schema parsing */
        LOGINT;
        return EXIT_FAILURE;
    }
    if (!intv) {
        return EXIT_SUCCESS;
    }

    for (tmp_intv = intv; tmp_intv; tmp_intv = tmp_intv->next) {
        if (((kind == 0) && (unum < tmp_intv->value.uval.min))
                || ((kind == 1) && (snum < tmp_intv->value.sval.min))
                || ((kind == 2) && (fnum < tmp_intv->value.fval.min))) {
            break;
        }

        if (((kind == 0) && (unum >= tmp_intv->value.uval.min) && (unum <= tmp_intv->value.uval.max))
                || ((kind == 1) && (snum >= tmp_intv->value.sval.min) && (snum <= tmp_intv->value.sval.max))
                || ((kind == 2) && (fnum >= tmp_intv->value.fval.min) && (fnum <= tmp_intv->value.fval.max))) {
            ret = EXIT_SUCCESS;
            break;
        }
    }

    while (intv) {
        tmp_intv = intv->next;
        free(intv);
        intv = tmp_intv;
    }

    if (ret) {
        LOGVAL(LYE_OORVAL, LY_VLOG_LYD, node, (val_str ? val_str : ""));
    }
    return ret;
}

/* logs directly */
static int
validate_pattern(const char *val_str, struct lys_type *type, struct lyd_node *node)
{
    int i, err_offset;
    pcre *precomp;
    char *perl_regex;
    const char *err_ptr;

    assert(type->base == LY_TYPE_STRING);

    if (!val_str) {
        val_str = "";
    }

    if (type->der && validate_pattern(val_str, &type->der->type, node)) {
        return EXIT_FAILURE;
    }

    for (i = 0; i < type->info.str.pat_count; ++i) {
        /*
         * adjust the expression to a Perl equivalent
         *
         * http://www.w3.org/TR/2004/REC-xmlschema-2-20041028/#regexs
         */
        perl_regex = malloc((strlen(type->info.str.patterns[i].expr)+2) * sizeof(char));
        if (!perl_regex) {
            LOGMEM;
            return EXIT_FAILURE;
        }
        perl_regex[0] = '\0';
        strcat(perl_regex, type->info.str.patterns[i].expr);
        if (strncmp(type->info.str.patterns[i].expr
                + strlen(type->info.str.patterns[i].expr) - 2, ".*", 2)) {
            strcat(perl_regex, "$");
        }

        /* must return 0, already checked during parsing */
        precomp = pcre_compile(perl_regex, PCRE_ANCHORED | PCRE_DOLLAR_ENDONLY | PCRE_NO_AUTO_CAPTURE,
                               &err_ptr, &err_offset, NULL);
        if (!precomp) {
            LOGINT;
            free(perl_regex);
            return EXIT_FAILURE;
        }
        free(perl_regex);

        if (pcre_exec(precomp, NULL, val_str, strlen(val_str), 0, 0, NULL, 0)) {
            free(precomp);
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, val_str, node->schema->name);
            return EXIT_FAILURE;
        }
        free(precomp);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Checks the syntax of length or range statement,
 *        on success checks the semantics as well. Does not log.
 *
 * @param[in] expr Length or range expression.
 * @param[in] type Type with the restriction.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise.
 */
int
lyp_check_length_range(const char *expr, struct lys_type *type)
{
    struct len_ran_intv *intv = NULL, *tmp_intv;
    const char *c = expr;
    char *tail;
    int ret = EXIT_FAILURE, flg = 1; /* first run flag */

    assert(expr);

lengthpart:

    while (isspace(*c)) {
        c++;
    }

    /* lower boundary or explicit number */
    if (!strncmp(c, "max", 3)) {
max:
        c += 3;
        while (isspace(*c)) {
            c++;
        }
        if (*c != '\0') {
            goto error;
        }

        goto syntax_ok;

    } else if (!strncmp(c, "min", 3)) {
        if (!flg) {
            /* min cannot be used elsewhere than in the first length-part */
            goto error;
        } else {
            flg = 0;
        }
        c += 3;
        while (isspace(*c)) {
            c++;
        }

        if (*c == '|') {
            c++;
            /* process next length-parth */
            goto lengthpart;
        } else if (*c == '\0') {
            goto syntax_ok;
        } else if (!strncmp(c, "..", 2)) {
upper:
            c += 2;
            while (isspace(*c)) {
                c++;
            }
            if (*c == '\0') {
                goto error;
            }

            /* upper boundary */
            if (!strncmp(c, "max", 3)) {
                goto max;
            }

            if (!isdigit(*c) && (*c != '+') && (*c != '-')) {
                goto error;
            }

            errno = 0;
            strtoll(c, &tail, 10);
            if (errno) {
                goto error;
            }
            c = tail;
            while (isspace(*c)) {
                c++;
            }
            if (*c == '\0') {
                goto syntax_ok;
            } else if (*c == '|') {
                c++;
                /* process next length-parth */
                goto lengthpart;
            } else {
                goto error;
            }
        } else {
            goto error;
        }

    } else if (isdigit(*c) || (*c == '-') || (*c == '+')) {
        /* number */
        errno = 0;
        strtoll(c, &tail, 10);
        if (errno) {
            /* out of range value */
            goto error;
        }
        c = tail;
        while (isspace(*c)) {
            c++;
        }

        if (*c == '|') {
            c++;
            /* process next length-parth */
            goto lengthpart;
        } else if (*c == '\0') {
            goto syntax_ok;
        } else if (!strncmp(c, "..", 2)) {
            goto upper;
        }

    } else {
        goto error;
    }

syntax_ok:

    if (resolve_len_ran_interval(expr, type, 1, &intv)) {
        goto error;
    }

    ret = EXIT_SUCCESS;

error:

    while (intv) {
        tmp_intv = intv->next;
        free(intv);
        intv = tmp_intv;
    }

    return ret;
}

/*
 * logs directly
 *
 * resolve - whether resolve identityrefs and leafrefs (which must be in JSON form)
 */
static int
lyp_parse_value_(struct lyd_node_leaf_list *node, struct lys_type *stype, int resolve)
{
    #define DECSIZE 21
    struct lys_type *type;
    char dec[DECSIZE];
    int64_t num;
    uint64_t unum;
    int len;
    int c, i, j, d;
    int found;

    assert(node && (node->value_type == stype->base));

    switch (node->value_type) {
    case LY_TYPE_BINARY:
        if (validate_length_range(0, (node->value_str ? strlen(node->value_str) : 0), 0, 0, stype,
                                  node->value_str, (struct lyd_node *)node)) {
            return EXIT_FAILURE;
        }

        node->value.binary = node->value_str;
        break;

    case LY_TYPE_BITS:
        /* locate bits structure with the bits definitions */
        for (type = stype; type->der->type.der; type = &type->der->type);

        /* allocate the array of  pointers to bits definition */
        node->value.bit = calloc(type->info.bits.count, sizeof *node->value.bit);
        if (!node->value.bit) {
            LOGMEM;
            return EXIT_FAILURE;
        }

        if (!node->value_str) {
            /* no bits set */
            break;
        }

        c = 0;
        i = 0;
        while (node->value_str[c]) {
            /* skip leading whitespaces */
            while (isspace(node->value_str[c])) {
                c++;
            }

            /* get the length of the bit identifier */
            for (len = 0; node->value_str[c] && !isspace(node->value_str[c]); c++, len++);

            /* go back to the beginning of the identifier */
            c = c - len;

            /* find bit definition, identifiers appear ordered by their posititon */
            for (found = 0; i < type->info.bits.count; i++) {
                if (!strncmp(type->info.bits.bit[i].name, &node->value_str[c], len)
                        && !type->info.bits.bit[i].name[len]) {
                    /* we have match, store the pointer */
                    node->value.bit[i] = &type->info.bits.bit[i];

                    /* stop searching */
                    i++;
                    found = 1;
                    break;
                }
            }

            if (!found) {
                /* referenced bit value does not exists */
                LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, node->value_str, node->schema->name);
                return EXIT_FAILURE;
            }

            c = c + len;
        }

        break;

    case LY_TYPE_BOOL:
        if (!node->value_str) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, "", node->schema->name);
            return EXIT_FAILURE;
        }

        if (!strcmp(node->value_str, "true")) {
            node->value.bln = 1;
        } else if (strcmp(node->value_str, "false")) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, node->value_str, node->schema->name);
            return EXIT_FAILURE;
        }
        /* else stays 0 */
        break;

    case LY_TYPE_DEC64:
        if (!node->value_str) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, "", node->schema->name);
            return EXIT_FAILURE;
        }

        /* locate dec64 structure with the fraction-digits value */
        for (type = stype; type->der->type.der; type = &type->der->type);

        for (c = 0; isspace(node->value_str[c]); c++);
        for (len = 0; node->value_str[c] && !isspace(node->value_str[c]); c++, len++);
        c = c - len;
        if (len > DECSIZE) {
            /* too long */
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, node->value_str, node->schema->name);
            return EXIT_FAILURE;
        }

        /* normalize the number */
        dec[0] = '\0';
        for (i = j = d = found = 0; i < DECSIZE; i++) {
            if (node->value_str[c + i] == '.') {
                found = 1;
                j = type->info.dec64.dig;
                i--;
                c++;
                continue;
            }
            if (node->value_str[c + i] == '\0') {
                c--;
                if (!found) {
                    j = type->info.dec64.dig;
                    found = 1;
                }
                if (!j) {
                    dec[i] = '\0';
                    break;
                }
                d++;
                if (d > DECSIZE - 2) {
                    LOGVAL(LYE_OORVAL, LY_VLOG_LYD, node, node->value_str, node->schema->name);
                    return EXIT_FAILURE;
                }
                dec[i] = '0';
            } else {
                if (!isdigit(node->value_str[c + i])) {
                    if (i || node->value_str[c] != '-') {
                        LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, node->value_str, node->schema->name);
                        return EXIT_FAILURE;
                    }
                } else {
                    d++;
                }
                if (d > DECSIZE - 2 || (found && !j)) {
                    LOGVAL(LYE_OORVAL, LY_VLOG_LYD, node, node->value_str, node->schema->name);
                    return EXIT_FAILURE;
                }
                dec[i] = node->value_str[c + i];
            }
            if (j) {
                j--;
            }
        }

        if (parse_int(dec, __INT64_C(-9223372036854775807) - __INT64_C(1), __INT64_C(9223372036854775807), 10, &num,
                      (struct lyd_node *)node)
                || validate_length_range(2, 0, 0, ((long double)num)/(1 << type->info.dec64.dig), stype,
                                         node->value_str, (struct lyd_node *)node)) {
            return EXIT_FAILURE;
        }
        node->value.dec64 = num;
        break;

    case LY_TYPE_EMPTY:
        /* just check that it is empty */
        if (node->value_str && node->value_str[0]) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, node->value_str, node->schema->name);
            return EXIT_FAILURE;
        }
        break;

    case LY_TYPE_ENUM:
        if (!node->value_str) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, "", node->schema->name);
            return EXIT_FAILURE;
        }

        /* locate enums structure with the enumeration definitions */
        for (type = stype; type->der->type.der; type = &type->der->type);

        /* find matching enumeration value */
        for (i = 0; i < type->info.enums.count; i++) {
            if (!strcmp(node->value_str, type->info.enums.enm[i].name)) {
                /* we have match, store pointer to the definition */
                node->value.enm = &type->info.enums.enm[i];
                break;
            }
        }

        if (!node->value.enm) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, node->value_str, node->schema->name);
            return EXIT_FAILURE;
        }

        break;

    case LY_TYPE_IDENT:
        if (!node->value_str) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, "", node->schema->name);
            return EXIT_FAILURE;
        }

        node->value.ident = resolve_identref(stype->info.ident.ref, node->value_str, (struct lyd_node *)node);
        if (!node->value.ident) {
            return EXIT_FAILURE;
        }
        break;

    case LY_TYPE_INST:
        if (!node->value_str) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, "", node->schema->name);
            return EXIT_FAILURE;
        }

        if (!resolve) {
            node->value_type |= LY_TYPE_INST_UNRES;
        }
        break;

    case LY_TYPE_LEAFREF:
        if (!node->value_str) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, "", node->schema->name);
            return EXIT_FAILURE;
        }

        if (!resolve) {
            type = &((struct lys_node_leaf *)node->schema)->type.info.lref.target->type;
            while (type->base == LY_TYPE_LEAFREF) {
                type = &type->info.lref.target->type;
            }
            node->value_type = type->base | LY_TYPE_LEAFREF_UNRES;
        }
        break;

    case LY_TYPE_STRING:
        if (validate_length_range(0, (node->value_str ? strlen(node->value_str) : 0), 0, 0, stype,
                                  node->value_str, (struct lyd_node *)node)) {
            return EXIT_FAILURE;
        }

        if (validate_pattern(node->value_str, stype, (struct lyd_node *)node)) {
            return EXIT_FAILURE;
        }

        node->value.string = node->value_str;
        break;

    case LY_TYPE_INT8:
        if (parse_int(node->value_str, __INT64_C(-128), __INT64_C(127), 0, &num, (struct lyd_node *)node)
                || validate_length_range(1, 0, num, 0, stype, node->value_str, (struct lyd_node *)node)) {
            return EXIT_FAILURE;
        }
        node->value.int8 = num;
        break;

    case LY_TYPE_INT16:
        if (parse_int(node->value_str, __INT64_C(-32768), __INT64_C(32767), 0, &num, (struct lyd_node *)node)
                || validate_length_range(1, 0, num, 0, stype, node->value_str, (struct lyd_node *)node)) {
            return EXIT_FAILURE;
        }
        node->value.int16 = num;
        break;

    case LY_TYPE_INT32:
        if (parse_int(node->value_str, __INT64_C(-2147483648), __INT64_C(2147483647), 0, &num, (struct lyd_node *)node)
                || validate_length_range(1, 0, num, 0, stype, node->value_str, (struct lyd_node *)node)) {
            return EXIT_FAILURE;
        }
        node->value.int32 = num;
        break;

    case LY_TYPE_INT64:
        if (parse_int(node->value_str, __INT64_C(-9223372036854775807) - __INT64_C(1), __INT64_C(9223372036854775807),
                      0, &num, (struct lyd_node *)node)
                || validate_length_range(1, 0, num, 0, stype, node->value_str, (struct lyd_node *)node)) {
            return EXIT_FAILURE;
        }
        node->value.int64 = num;
        break;

    case LY_TYPE_UINT8:
        if (parse_uint(node->value_str, __UINT64_C(255), __UINT64_C(0), &unum, (struct lyd_node *)node)
                || validate_length_range(0, unum, 0, 0, stype, node->value_str, (struct lyd_node *)node)) {
            return EXIT_FAILURE;
        }
        node->value.uint8 = unum;
        break;

    case LY_TYPE_UINT16:
        if (parse_uint(node->value_str, __UINT64_C(65535), __UINT64_C(0), &unum, (struct lyd_node *)node)
                || validate_length_range(0, unum, 0, 0, stype, node->value_str, (struct lyd_node *)node)) {
            return EXIT_FAILURE;
        }
        node->value.uint16 = unum;
        break;

    case LY_TYPE_UINT32:
        if (parse_uint(node->value_str, __UINT64_C(4294967295), __UINT64_C(0), &unum, (struct lyd_node *)node)
                || validate_length_range(0, unum, 0, 0, stype, node->value_str, (struct lyd_node *)node)) {
            return EXIT_FAILURE;
        }
        node->value.uint32 = unum;
        break;

    case LY_TYPE_UINT64:
        if (parse_uint(node->value_str, __UINT64_C(18446744073709551615), __UINT64_C(0), &unum, (struct lyd_node *)node)
                || validate_length_range(0, unum, 0, 0, stype, node->value_str, (struct lyd_node *)node)) {
            return EXIT_FAILURE;
        }
        node->value.uint64 = unum;
        break;

    default:
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int
lyp_parse_value(struct lyd_node_leaf_list *leaf, struct lyxml_elem *xml, int resolve)
{
    int found = 0;
    struct lys_type *type, *stype;

    assert(leaf);

    stype = &((struct lys_node_leaf *)leaf->schema)->type;
    if (stype->base == LY_TYPE_UNION) {
        /* turn logging off, we are going to try to validate the value with all the types in order */
        ly_vlog_hide(1);

        type = lyp_get_next_union_type(stype, NULL, &found);
        while (type) {
            leaf->value_type = type->base;
            memset(&leaf->value, 0, sizeof leaf->value);

            /* in these cases we use JSON format */
            if (xml && ((type->base == LY_TYPE_IDENT) || (type->base == LY_TYPE_INST))) {
                xml->content = leaf->value_str;
                leaf->value_str = transform_xml2json(leaf->schema->module->ctx, xml->content, xml, 0);
                if (!leaf->value_str) {
                    leaf->value_str = xml->content;
                    xml->content = NULL;

                    found = 0;
                    type = lyp_get_next_union_type(stype, type, &found);
                    continue;
                }
            }

            if (!lyp_parse_value_(leaf, type, resolve)) {
                /* success */
                break;
            }

            if (xml && ((type->base == LY_TYPE_IDENT) || (type->base == LY_TYPE_INST))) {
                lydict_remove(leaf->schema->module->ctx, leaf->value_str);
                leaf->value_str = xml->content;
                xml->content = NULL;
            }

            found = 0;
            type = lyp_get_next_union_type(stype, type, &found);
        }

        ly_vlog_hide(0);

        if (!type) {
            /* failure */
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, (leaf->value_str ? leaf->value_str : ""), leaf->schema->name);
            return EXIT_FAILURE;
        }
    } else {
        memset(&leaf->value, 0, sizeof leaf->value);
        if (lyp_parse_value_(leaf, stype, resolve)) {
            ly_errno = LY_EVALID;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* does not log, cannot fail */
struct lys_type *
lyp_get_next_union_type(struct lys_type *type, struct lys_type *prev_type, int *found)
{
    int i;
    struct lys_type *ret = NULL;

    for (i = 0; i < type->info.uni.count; ++i) {
        if (type->info.uni.types[i].base == LY_TYPE_UNION) {
            ret = lyp_get_next_union_type(&type->info.uni.types[i], prev_type, found);
            if (ret) {
                break;;
            }
            continue;
        }

        if (!prev_type || *found) {
            ret = &type->info.uni.types[i];
            break;
        }

        if (&type->info.uni.types[i] == prev_type) {
            *found = 1;
        }
    }

    if (!ret && type->der) {
        ret = lyp_get_next_union_type(&type->der->type, prev_type, found);
    }

    return ret;
}

/* does not log */
static int
dup_typedef_check(const char *type, struct lys_tpdf *tpdf, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        if (!strcmp(type, tpdf[i].name)) {
            /* name collision */
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* does not log */
static int
dup_feature_check(const char *id, struct lys_module *module)
{
    int i;

    for (i = 0; i < module->features_size; i++) {
        if (!strcmp(id, module->features[i].name)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* does not log */
int
dup_prefix_check(const char *prefix, struct lys_module *module)
{
    int i;

    if (module->prefix && !strcmp(module->prefix, prefix)) {
        return EXIT_FAILURE;
    }
    for (i = 0; i < module->imp_size; i++) {
        if (!strcmp(module->imp[i].prefix, prefix)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* logs directly */
int
lyp_check_identifier(const char *id, enum LY_IDENT type, struct lys_module *module, struct lys_node *parent)
{
    int i;
    int size;
    struct lys_tpdf *tpdf;
    struct lys_node *node;

    assert(id);

    /* check id syntax */
    if (!(id[0] >= 'A' && id[0] <= 'Z') && !(id[0] >= 'a' && id[0] <= 'z') && id[0] != '_') {
        LOGVAL(LYE_INID, LY_VLOG_NONE, NULL, id, "invalid start character");
        return EXIT_FAILURE;
    }
    for (i = 1; id[i]; i++) {
        if (!(id[i] >= 'A' && id[i] <= 'Z') && !(id[i] >= 'a' && id[i] <= 'z')
                && !(id[i] >= '0' && id[i] <= '9') && id[i] != '_' && id[i] != '-' && id[i] != '.') {
            LOGVAL(LYE_INID, LY_VLOG_NONE, NULL, id, "invalid character");
            return EXIT_FAILURE;
        }
    }

    if (i > 64) {
        LOGWRN("Identifier \"%s\" is long, you should use something shorter.", id);
    }

    switch (type) {
    case LY_IDENT_NAME:
        /* check uniqueness of the node within its siblings */
        if (!parent) {
            break;
        }

        LY_TREE_FOR(parent->child, node) {
            if (ly_strequal(node->name, id, 1)) {
                LOGVAL(LYE_INID, LY_VLOG_NONE, NULL, id, "name duplication");
                return EXIT_FAILURE;
            }
        }
        break;
    case LY_IDENT_TYPE:
        assert(module);

        /* check collision with the built-in types */
        if (!strcmp(id, "binary") || !strcmp(id, "bits") ||
                !strcmp(id, "boolean") || !strcmp(id, "decimal64") ||
                !strcmp(id, "empty") || !strcmp(id, "enumeration") ||
                !strcmp(id, "identityref") || !strcmp(id, "instance-identifier") ||
                !strcmp(id, "int8") || !strcmp(id, "int16") ||
                !strcmp(id, "int32") || !strcmp(id, "int64") ||
                !strcmp(id, "leafref") || !strcmp(id, "string") ||
                !strcmp(id, "uint8") || !strcmp(id, "uint16") ||
                !strcmp(id, "uint32") || !strcmp(id, "uint64") || !strcmp(id, "union")) {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, id, "typedef");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Typedef name duplicates a built-in type.");
            return EXIT_FAILURE;
        }

        /* check locally scoped typedefs (avoid name shadowing) */
        for (; parent; parent = parent->parent) {
            switch (parent->nodetype) {
            case LYS_CONTAINER:
                size = ((struct lys_node_container *)parent)->tpdf_size;
                tpdf = ((struct lys_node_container *)parent)->tpdf;
                break;
            case LYS_LIST:
                size = ((struct lys_node_list *)parent)->tpdf_size;
                tpdf = ((struct lys_node_list *)parent)->tpdf;
                break;
            case LYS_GROUPING:
                size = ((struct lys_node_grp *)parent)->tpdf_size;
                tpdf = ((struct lys_node_grp *)parent)->tpdf;
                break;
            default:
                continue;
            }

            if (dup_typedef_check(id, tpdf, size)) {
                LOGVAL(LYE_DUPID, LY_VLOG_NONE, NULL, "typedef", id);
                return EXIT_FAILURE;
            }
        }

        /* check top-level names */
        if (dup_typedef_check(id, module->tpdf, module->tpdf_size)) {
            LOGVAL(LYE_DUPID, LY_VLOG_NONE, NULL, "typedef", id);
            return EXIT_FAILURE;
        }

        /* check submodule's top-level names */
        for (i = 0; i < module->inc_size && module->inc[i].submodule; i++) {
            if (dup_typedef_check(id, module->inc[i].submodule->tpdf, module->inc[i].submodule->tpdf_size)) {
                LOGVAL(LYE_DUPID, LY_VLOG_NONE, NULL, "typedef", id);
                return EXIT_FAILURE;
            }
        }

        break;
    case LY_IDENT_PREFIX:
        assert(module);

        /* check the module itself */
        if (dup_prefix_check(id, module)) {
            LOGVAL(LYE_DUPID, LY_VLOG_NONE, NULL, "prefix", id);
            return EXIT_FAILURE;
        }

        /* and all its submodules */
        for (i = 0; i < module->inc_size && module->inc[i].submodule; i++) {
            if (dup_prefix_check(id, (struct lys_module *)module->inc[i].submodule)) {
                LOGVAL(LYE_DUPID, LY_VLOG_NONE, NULL, "prefix", id);
                return EXIT_FAILURE;
            }
        }
        break;
    case LY_IDENT_FEATURE:
        assert(module);

        /* check feature name uniqness*/
        /* check features in the current module */
        if (dup_feature_check(id, module)) {
            LOGVAL(LYE_DUPID, LY_VLOG_NONE, NULL, "feature", id);
            return EXIT_FAILURE;
        }

        /* and all its submodules */
        for (i = 0; i < module->inc_size && module->inc[i].submodule; i++) {
            if (dup_feature_check(id, (struct lys_module *)module->inc[i].submodule)) {
                LOGVAL(LYE_DUPID, LY_VLOG_NONE, NULL, "feature", id);
                return EXIT_FAILURE;
            }
        }
        break;

    default:
        /* no check required */
        break;
    }

    return EXIT_SUCCESS;
}

/* logs directly */
int
lyp_check_date(const char *date)
{
    int i;

    assert(date);

    if (strlen(date) != LY_REV_SIZE - 1) {
        goto error;
    }

    for (i = 0; i < LY_REV_SIZE - 1; i++) {
        if (i == 4 || i == 7) {
            if (date[i] != '-') {
                goto error;
            }
        } else if (!isdigit(date[i])) {
            goto error;
        }
    }

    return EXIT_SUCCESS;

error:

    LOGVAL(LYE_INDATE, LY_VLOG_NONE, NULL, date);
    return EXIT_FAILURE;
}

/* does not log */
int
lyp_check_mandatory(struct lys_node *node)
{
    struct lys_node *child;

    assert(node);

    if (node->flags & LYS_MAND_TRUE) {
        return EXIT_FAILURE;
    }

    if (node->nodetype == LYS_CASE || node->nodetype == LYS_CHOICE) {
        LY_TREE_FOR(node->child, child) {
            if (lyp_check_mandatory(child)) {
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

int
lyp_check_status(uint8_t flags1, struct lys_module *mod1, const char *name1,
                 uint8_t flags2, struct lys_module *mod2, const char *name2,
                 const struct lys_node *node)
{
    uint8_t flg1, flg2;

    flg1 = (flags1 & LYS_STATUS_MASK) ? (flags1 & LYS_STATUS_MASK) : LYS_STATUS_CURR;
    flg2 = (flags2 & LYS_STATUS_MASK) ? (flags2 & LYS_STATUS_MASK) : LYS_STATUS_CURR;

    if ((flg1 < flg2) && (mod1 == mod2)) {
        LOGVAL(LYE_INSTATUS, node ? LY_VLOG_LYS : LY_VLOG_NONE, node,
               flg1 == LYS_STATUS_CURR ? "current" : "deprecated", name1,
               flg2 == LYS_STATUS_OBSLT ? "obsolete" : "deprecated", name2);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int
lyp_check_include(struct lys_module *module, struct lys_submodule *submodule, const char *value,
                  struct lys_include *inc, struct unres_schema *unres)
{
    char *module_data;
    void (*module_data_free)(void *module_data) = NULL;
    LYS_INFORMAT format = LYS_IN_UNKNOWN;
    int count, i;

    /* check that the submodule was not included yet (previous submodule could have included it) */
    for (i = 0; i < module->inc_size; ++i) {
        if (module->inc[i].submodule && (ly_strequal(module->inc[i].submodule->name, value, 1))) {
            /* copy the duplicate into the result */
            memcpy(inc, &module->inc[i], sizeof *inc);

            if (submodule) {
                /* we don't care if it was external or not */
                inc->external = 0;
            } else if (inc->external) {
                /* remove the duplicate */
                --module->inc_size;
                memmove(&module->inc[i], &module->inc[i + 1], (module->inc_size - i) * sizeof *inc);
                module->inc = ly_realloc(module->inc, module->inc_size * sizeof *module->inc);

                /* it is no longer external */
                inc->external = 0;
            }
            /* if !submodule && !inc->external, we just create a duplicate so it is detected and ended with error */

            return EXIT_SUCCESS;
        }
    }

    /* check for circular include, store it if passed */
    if (!module->ctx->models.parsing) {
        count = 0;
    } else {
        for (count = 0; module->ctx->models.parsing[count]; ++count) {
            if (ly_strequal(value, module->ctx->models.parsing[count], 1)) {
                LOGERR(LY_EVALID, "Circular include dependency on the submodule \"%s\".", value);
                goto error;
            }
        }
    }
    ++count;
    module->ctx->models.parsing =
        ly_realloc(module->ctx->models.parsing, (count + 1) * sizeof *module->ctx->models.parsing);
    if (!module->ctx->models.parsing) {
        LOGMEM;
        goto error;
    }
    module->ctx->models.parsing[count - 1] = value;
    module->ctx->models.parsing[count] = NULL;

    /* try to load the submodule */
    inc->submodule = (struct lys_submodule *)ly_ctx_get_submodule2(module, value);
    if (inc->submodule) {
        if (inc->rev[0]) {
            if (!inc->submodule->rev_size || !ly_strequal(inc->rev, inc->submodule->rev[0].date, 1)) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, inc->rev[0], "revision");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Multiple revisions of the same submodule included.");
                goto error;
            }
        }
    } else {
        if (module->ctx->module_clb) {
            module_data = module->ctx->module_clb(value, inc->rev[0] ? inc->rev : NULL, module->ctx->module_clb_data,
                                                  &format, &module_data_free);
            if (module_data) {
                inc->submodule = lys_submodule_parse(module, module_data, format, unres);
                if (module_data_free) {
                    module_data_free(module_data);
                } else {
                    free(module_data);
                }
            } else {
                LOGERR(LY_EVALID, "User module retrieval callback failed!");
            }
        } else {
            inc->submodule = (struct lys_submodule *)lyp_search_file(module->ctx, module, value,
                                                                     inc->rev[0] ? inc->rev : NULL, unres);
        }
    }

    /* remove the new submodule name now that its parsing is finished (even if failed) */
    if (module->ctx->models.parsing[count] || !ly_strequal(module->ctx->models.parsing[count - 1], value, 1)) {
        LOGINT;
    }
    --count;
    if (count) {
        module->ctx->models.parsing[count] = NULL;
    } else {
        free(module->ctx->models.parsing);
        module->ctx->models.parsing = NULL;
    }

    /* check the result */
    if (!inc->submodule) {
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "include");
        LOGERR(LY_EVALID, "Including \"%s\" module into \"%s\" failed.", value, module->name);
        goto error;
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

int
lyp_check_import(struct lys_module *module, const char *value, struct lys_import *imp)
{
    int count;

    /* check for circular import, store it if passed */
    if (!module->ctx->models.parsing) {
        count = 0;
    } else {
        for (count = 0; module->ctx->models.parsing[count]; ++count) {
            if (ly_strequal(value, module->ctx->models.parsing[count], 1)) {
                LOGERR(LY_EVALID, "Circular import dependency on the module \"%s\".", value);
                goto error;
            }
        }
    }
    ++count;
    module->ctx->models.parsing =
        ly_realloc(module->ctx->models.parsing, (count + 1) * sizeof *module->ctx->models.parsing);
    if (!module->ctx->models.parsing) {
        LOGMEM;
        goto error;
    }
    module->ctx->models.parsing[count - 1] = value;
    module->ctx->models.parsing[count] = NULL;

    /* try to load the module */
    imp->module = (struct lys_module *)ly_ctx_get_module(module->ctx, value, imp->rev[0] ? imp->rev : NULL);
    if (!imp->module) {
        /* whether to use a user callback is decided in the function */
        imp->module = (struct lys_module *)ly_ctx_load_module(module->ctx, value, imp->rev[0] ? imp->rev : NULL);
    }

    /* remove the new module name now that its parsing is finished (even if failed) */
    if (module->ctx->models.parsing[count] || !ly_strequal(module->ctx->models.parsing[count - 1], value, 1)) {
        LOGINT;
    }
    --count;
    if (count) {
        module->ctx->models.parsing[count] = NULL;
    } else {
        free(module->ctx->models.parsing);
        module->ctx->models.parsing = NULL;
    }

    /* check the result */
    if (!imp->module) {
        LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "import");
        LOGERR(LY_EVALID, "Importing \"%s\" module into \"%s\" failed.", value, module->name);
        goto error;
    }

    return EXIT_SUCCESS;

error:

    return EXIT_FAILURE;
}

/* Propagate imports and includes into the main module */
int
lyp_propagate_submodule(struct lys_module *module, struct lys_submodule *submodule)
{
    int i, j, r;
    struct lys_include *aux_inc;
    struct lys_import *aux_imp;

    /* propagate imports into the main module */
    for (i = r = 0; i < submodule->imp_size; i++) {
        for (j = 0; j < module->imp_size; j++) {
            if (submodule->imp[i].module == module->imp[j].module &&
                    !strcmp(submodule->imp[i].rev, module->imp[j].rev)) {
                /* check prefix match */
                if (!ly_strequal(submodule->imp[i].prefix, module->imp[j].prefix, 1)) {
                    LOGVAL(LYE_INID, LY_VLOG_NONE, NULL, submodule->imp[i].prefix,
                           "non-matching prefixes of imported module in main module and submodule");
                    goto error;
                }
                break;
            }
        }
        if (j == module->imp_size) {
            /* new import */
            r++;
        }
    }
    if (r) {
        aux_imp = realloc(module->imp, (module->imp_size + r) * sizeof *module->imp);
        if (!aux_imp) {
            LOGMEM;
            goto error;
        }
        module->imp = aux_imp;
        for (i = r = 0; i < submodule->imp_size; i++) {
            for (j = 0; j < module->imp_size; j++) {
                if (submodule->imp[i].module == module->imp[j].module) {
                    break;
                }
            }
            if (j == module->imp_size) {
                /* new import */
                /* check prefix uniqueness */
                if (dup_prefix_check(submodule->imp[i].prefix, module)) {
                    LOGVAL(LYE_DUPID, LY_VLOG_NONE, NULL, "prefix", submodule->imp[i].prefix);
                    goto error;
                }
                memcpy(&module->imp[module->imp_size + r], &submodule->imp[i], sizeof *submodule->imp);
                module->imp[module->imp_size + r].external = 1;
                r++;
            }
        }
        module->imp_size += r;
    }

    /* propagate includes into the main module */
    for (i = r = 0; i < submodule->inc_size; i++) {
        for (j = 0; j < module->inc_size; j++) {
            if (submodule->inc[i].submodule == module->inc[j].submodule) {
                break;
            }
        }
        if (j == module->inc_size) {
            /* new include */
            r++;
        }
    }

    if (r) {
        aux_inc = realloc(module->inc, (module->inc_size + r) * sizeof *module->inc);
        if (!aux_inc) {
            LOGMEM;
            goto error;
        }
        module->inc = aux_inc;
        for (i = r = 0; i < submodule->inc_size; i++) {
            for (j = 0; j < module->inc_size; j++) {
                if (submodule->inc[i].submodule == module->inc[j].submodule) {
                    break;
                }
            }
            if (j == module->inc_size) {
                /* new include */
                memcpy(&module->inc[module->inc_size + r], &submodule->inc[i], sizeof *submodule->inc);
                module->inc[module->inc_size + r].external = 1;
                r++;
            }
        }
        module->inc_size += r;
    }
    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

int
lyp_add_module(struct lys_module *module, int implement)
{
    struct ly_ctx *ctx;
    struct lys_module **newlist = NULL;
    int i;

    ctx = module->ctx;

    /* add to the context's list of modules */
    if (ctx->models.used == ctx->models.size) {
        newlist = realloc(ctx->models.list, (2 * ctx->models.size) * sizeof *newlist);
        if (!newlist) {
            LOGMEM;
            goto error;
        }
        for (i = ctx->models.size; i < ctx->models.size * 2; i++) {
            newlist[i] = NULL;
        }
        ctx->models.size *= 2;
        ctx->models.list = newlist;
    }
    for (i = 0; ctx->models.list[i]; i++) {
        /* check name (name/revision) and namespace uniqueness */
        if (!strcmp(ctx->models.list[i]->name, module->name)) {
            if (ctx->models.list[i]->rev_size == module->rev_size) {
                /* both have the same number of revisions */
                if (!module->rev_size || !strcmp(ctx->models.list[i]->rev[0].date, module->rev[0].date)) {
                    /* both have the same revision -> we already have the same module */
                    /* so free the new one and update the old one's implement flag if needed */
                    LOGVRB("Module \"%s\" already in context.", ctx->models.list[i]->name);

                    lys_free(module, NULL, 1);
                    module = ctx->models.list[i];
                    if (implement && !module->implemented) {
                        lyp_set_implemented(module);
                    }

                    return EXIT_SUCCESS;
                }
            }
            /* else (both elses) keep searching, for now the caller is just adding
             * another revision of an already present schema
             */
        } else if (!strcmp(ctx->models.list[i]->ns, module->ns)) {
            LOGERR(LY_EINVAL, "Two different modules (\"%s\" and \"%s\") have the same namespace \"%s\".",
                   ctx->models.list[i]->name, module->name, module->ns);
            goto error;
        }
    }
    ctx->models.list[i] = module;
    ctx->models.used++;
    ctx->models.module_set_id++;
    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
}

void
lyp_fail_module(struct lys_module *module)
{
    int i, j, flag;
    struct lys_node *next, *elem;
    struct lys_module *orig_mod;

    /* remove applied deviations */
    for (i = 0; i < module->deviation_size; ++i) {
        if (module->deviation[i].orig_node) {
            resolve_augment_schema_nodeid(module->deviation[i].target_name, NULL, module, (const struct lys_node **)&elem);
            lys_node_switch(elem, module->deviation[i].orig_node);
            module->deviation[i].orig_node = elem;
        }

        /* remove our deviation import, clear deviated flag is possible */
        orig_mod = lys_node_module(module->deviation[i].orig_node);
        flag = 0;
        for (j = 0; j < orig_mod->imp_size; ++j) {
            if (orig_mod->imp[j].external == 2) {
                if (orig_mod->imp[j].module == module) {
                    /* our deviation import, remove it */
                    --orig_mod->imp_size;
                    if (j < orig_mod->imp_size) {
                        memcpy(&orig_mod->imp[j], &orig_mod->imp[j + 1], (orig_mod->imp_size - j) * sizeof *orig_mod->imp);
                    }
                    --j;
                } else {
                    /* some other deviation, we cannot clear the deviated flag */
                    flag = 1;
                }
            }
        }
        if (!flag) {
            /* it's safe to clear the deviated flag */
            orig_mod->deviated = 0;
        }
    }

    /* remove applied augments */
    for (i = 0; i < module->augment_size; ++i) {
        if (module->augment[i].target) {
            LY_TREE_FOR_SAFE(module->augment[i].target->child, next, elem) {
                if (elem->parent == (struct lys_node *)&module->augment[i]) {
                    lys_node_free(elem, NULL, 0);
                }
            }
        }
    }
}

void
lyp_fail_submodule(struct lys_submodule *submodule)
{
    struct lys_node *next, *elem;
    struct lys_module *orig_mod, *module;
    uint8_t i, j, flag;

    module = submodule->belongsto;

    /* remove parsed data */
    LY_TREE_FOR_SAFE(module->data, next, elem) {
        if (elem->module == (struct lys_module *)submodule) {
            lys_node_free(elem, NULL, 0);
        }
    }

    /* remove applied deviations */
    for (i = 0; i < submodule->deviation_size; ++i) {
        if (submodule->deviation[i].orig_node) {
            resolve_augment_schema_nodeid(submodule->deviation[i].target_name, NULL, module, (const struct lys_node **)&elem);
            lys_node_switch(elem, submodule->deviation[i].orig_node);
            submodule->deviation[i].orig_node = elem;
        }

        /* remove our deviation import, clear deviated flag is possible */
        orig_mod = lys_node_module(submodule->deviation[i].orig_node);
        flag = 0;
        for (j = 0; j < orig_mod->imp_size; ++j) {
            if (orig_mod->imp[j].external == 2) {
                if (orig_mod->imp[j].module == submodule->belongsto) {
                    /* our deviation import, remove it */
                    --orig_mod->imp_size;
                    if (j < orig_mod->imp_size) {
                        memcpy(&orig_mod->imp[j], &orig_mod->imp[j + 1], (orig_mod->imp_size - j) * sizeof *orig_mod->imp);
                    }
                    --j;
                } else {
                    /* some other deviation, we cannot clear the deviated flag */
                    flag = 1;
                }
            }
        }
        if (!flag) {
            /* it's safe to clear the deviated flag */
            orig_mod->deviated = 0;
        }
    }

    /* remove applied augments */
    for (i = 0; i < submodule->augment_size; ++i) {
        if (submodule->augment[i].target) {
            LY_TREE_FOR_SAFE(submodule->augment[i].target->child, next, elem) {
                if (elem->parent == (struct lys_node *)&submodule->augment[i]) {
                    lys_node_free(elem, NULL, 0);
                }
            }
        }
    }
}

/**
 * Store UTF-8 character specified as 4byte integer into the dst buffer.
 * Returns number of written bytes (4 max), expects that dst has enough space.
 *
 * UTF-8 mapping:
 * 00000000 -- 0000007F:    0xxxxxxx
 * 00000080 -- 000007FF:    110xxxxx 10xxxxxx
 * 00000800 -- 0000FFFF:    1110xxxx 10xxxxxx 10xxxxxx
 * 00010000 -- 001FFFFF:    11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 */
unsigned int
pututf8(char *dst, int32_t value)
{
    if (value < 0x80) {
        /* one byte character */
        dst[0] = value;

        return 1;
    } else if (value < 0x800) {
        /* two bytes character */
        dst[0] = 0xc0 | (value >> 6);
        dst[1] = 0x80 | (value & 0x3f);

        return 2;
    } else if (value < 0x10000) {
        /* three bytes character */
        dst[0] = 0xe0 | (value >> 12);
        dst[1] = 0x80 | ((value >> 6) & 0x3f);
        dst[2] = 0x80 | (value & 0x3f);

        return 3;
    } else if (value < 0x200000) {
        /* four bytes character */
        dst[0] = 0xf0 | (value >> 18);
        dst[1] = 0x80 | ((value >> 12) & 0x3f);
        dst[2] = 0x80 | ((value >> 6) & 0x3f);
        dst[3] = 0x80 | (value & 0x3f);

        return 4;
    } else {
        /* out of range */
        LOGVAL(LYE_XML_INCHAR, LY_VLOG_NONE, NULL, value);
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid UTF-8 value 0x%08x", value);
        return 0;
    }
}
