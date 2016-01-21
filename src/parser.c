/**
 * @file parser.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief common libyang parsers routines implementations
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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

    /* "is power of 2" algorithm, with 0 exception */
    return x ? !(x && !(x & (x - 1))) : 0;
}

void
lyp_set_implemented(struct lys_module *module)
{
    int i;

    module->implemented = 1;

    for (i = 0; i < module->inc_size; i++) {
        lyp_set_implemented((struct lys_module *)module->inc[i].submodule);
    }
}

/**
 * @brief Alternative for lys_read() + lys_parse() in case of import
 *
 * @param[in] fd MUST be a regular file (will be used by mmap)
 */
struct lys_module *
lys_read_import(struct ly_ctx *ctx, int fd, LYS_INFORMAT format)
{
    struct unres_schema *unres;
    struct lys_module *module = NULL;
    struct stat sb;
    char *addr;

    if (!ctx || fd < 0) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    unres = calloc(1, sizeof *unres);
    if (!unres) {
        LOGMEM;
        return NULL;
    }

    if (fstat(fd, &sb) == -1) {
        LOGERR(LY_ESYS, "Failed to stat the file descriptor (%s).", strerror(errno));
        free(unres);
        return NULL;
    }
    addr = mmap(NULL, sb.st_size + 1, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        LOGERR(LY_EMEM,"Map file into memory failed (%s()).",__func__);
        free(unres);
        return NULL;
    }
    switch (format) {
    case LYS_IN_YIN:
        module = yin_read_module(ctx, addr, 0, unres);
        break;
    case LYS_IN_YANG:
    default:
        /* TODO */
        break;
    }
    munmap(addr, sb.st_size);

    if (module && unres->count && resolve_unres_schema(module, unres)) {
        unres_schema_free(ctx, unres);
        lys_free(module, 0);
        module = NULL;
    } else {
        unres_schema_free(ctx, unres);
    }

    return module;
}

struct lys_module *
lyp_search_file(struct ly_ctx *ctx, struct lys_module *module, const char *name, const char *revision)
{
    size_t len, flen;
    int fd;
    char *wd, *cwd, *model_path;
    DIR *dir;
    struct dirent *file;
    LYS_INFORMAT format;
    struct lys_module *result = NULL;
    int localsearch = 1;

    len = strlen(name);
    cwd = wd = get_current_dir_name();

opendir_search:
    chdir(wd);
    dir = opendir(wd);
    LOGVRB("Searching for \"%s\" in %s.", name, wd);
    if (!dir) {
        LOGWRN("Unable to open directory \"%s\" for searching referenced modules (%s)",
               wd, strerror(errno));
        /* try search directory */
        goto searchpath;
    }

    while ((file = readdir(dir))) {
        if (strncmp(name, file->d_name, len) ||
                (file->d_name[len] != '.' && file->d_name[len] != '@')) {
            continue;
        }

        flen = strlen(file->d_name);
        if (revision && flen > len + 5) {
            /* check revision from the filename */
            /* TODO */
        }

        /* get type according to filename suffix */
        if (!strcmp(&file->d_name[flen - 4], ".yin")) {
            format = LYS_IN_YIN;
        /*TODO } else if (!strcmp(&file->d_name[flen - 5], ".yang")) {
            continue;
            format = LYS_IN_YANG;*/
        } else {
            continue;
        }

        /* open the file */
        fd = open(file->d_name, O_RDONLY);
        if (fd < 0) {
            LOGERR(LY_ESYS, "Unable to open data model file \"%s\" (%s).",
                   file->d_name, strerror(errno));
            goto cleanup;
        }

        if (module) {
            result = (struct lys_module *)lys_submodule_read(module, fd, format, module->implemented);
        } else {
            result = lys_read_import(ctx, fd, format);
        }
        close(fd);

        if (!result) {
            goto cleanup;
        }

        if (asprintf(&model_path, "file://%s/%s", wd, file->d_name) == -1) {
            LOGMEM;
            result = NULL;
            goto cleanup;
        }
        result->uri = lydict_insert(ctx, model_path, 0);
        free(model_path);
        /* success */
        goto cleanup;
    }

searchpath:
    if (!ctx->models.search_path) {
        LOGWRN("No search path defined for the current context.");
    } else if (!result && localsearch) {
        /* search in local directory done, try context's search_path */
        if (dir) {
            closedir(dir);
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
    chdir(cwd);
    if (cwd != wd) {
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
parse_int(const char *val_str, int64_t min, int64_t max, int base, int64_t *ret, const char *node_name, uint32_t line)
{
    char *strptr;

    if (!val_str) {
        LOGVAL(LYE_INVAL, line, "", node_name);
        return EXIT_FAILURE;
    }

    /* convert to 64-bit integer, all the redundant characters are handled */
    errno = 0;
    strptr = NULL;
    *ret = strtoll(val_str, &strptr, base);
    if (errno || (*ret < min) || (*ret > max)) {
        LOGVAL(LYE_OORVAL, line, val_str, node_name);
        return EXIT_FAILURE;
    } else if (strptr && *strptr) {
        while (isspace(*strptr)) {
            ++strptr;
        }
        if (*strptr) {
            LOGVAL(LYE_INVAL, line, val_str, node_name);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* logs directly */
static int
parse_uint(const char *val_str, uint64_t max, int base, uint64_t *ret, const char *node_name, uint32_t line)
{
    char *strptr;

    if (!val_str) {
        LOGVAL(LYE_INVAL, line, "", node_name);
        return EXIT_FAILURE;
    }

    errno = 0;
    strptr = NULL;
    *ret = strtoull(val_str, &strptr, base);
    if (errno || (*ret > max)) {
        LOGVAL(LYE_OORVAL, line, val_str, node_name);
        return EXIT_FAILURE;
    } else if (strptr && *strptr) {
        while (isspace(*strptr)) {
            ++strptr;
        }
        if (*strptr) {
            LOGVAL(LYE_INVAL, line, val_str, node_name);
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
                      const char *val_str, uint32_t line)
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
        LOGVAL(LYE_OORVAL, line, (val_str ? val_str : ""));
    }
    return ret;
}

/* logs directly */
static int
validate_pattern(const char *val_str, struct lys_type *type, const char *node_name, uint32_t line)
{
    int i, err_offset;
    pcre *precomp;
    char *perl_regex;
    const char *err_ptr;

    assert(type->base == LY_TYPE_STRING);

    if (!val_str) {
        val_str = "";
    }

    if (type->der && validate_pattern(val_str, &type->der->type, node_name, line)) {
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
            LOGVAL(LYE_INVAL, line, val_str, node_name);
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
 * unres - whether to try to resolve and on failure store it as unres or fail if resolving fails
 */
static int
lyp_parse_value_(struct lyd_node_leaf_list *node, struct lys_type *stype, int resolve, struct unres_data *unres, uint32_t line)
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
                                  node->value_str, line)) {
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
                LOGVAL(LYE_INVAL, line, node->value_str, node->schema->name);
                return EXIT_FAILURE;
            }

            c = c + len;
        }

        break;

    case LY_TYPE_BOOL:
        if (!node->value_str) {
            LOGVAL(LYE_INVAL, line, "", node->schema->name);
            return EXIT_FAILURE;
        }

        if (!strcmp(node->value_str, "true")) {
            node->value.bln = 1;
        } else if (strcmp(node->value_str, "false")) {
            LOGVAL(LYE_INVAL, line, node->value_str, node->schema->name);
            return EXIT_FAILURE;
        }
        /* else stays 0 */
        break;

    case LY_TYPE_DEC64:
        if (!node->value_str) {
            LOGVAL(LYE_INVAL, line, "", node->schema->name);
            return EXIT_FAILURE;
        }

        /* locate dec64 structure with the fraction-digits value */
        for (type = stype; type->der->type.der; type = &type->der->type);

        for (c = 0; isspace(node->value_str[c]); c++);
        for (len = 0; node->value_str[c] && !isspace(node->value_str[c]); c++, len++);
        c = c - len;
        if (len > DECSIZE) {
            /* too long */
            LOGVAL(LYE_INVAL, line, node->value_str, node->schema->name);
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
                    LOGVAL(LYE_OORVAL, line, node->value_str, node->schema->name);
                    return EXIT_FAILURE;
                }
                dec[i] = '0';
            } else {
                if (!isdigit(node->value_str[c + i])) {
                    if (i || node->value_str[c] != '-') {
                        LOGVAL(LYE_INVAL, line, node->value_str, node->schema->name);
                        return EXIT_FAILURE;
                    }
                } else {
                    d++;
                }
                if (d > DECSIZE - 2 || (found && !j)) {
                    LOGVAL(LYE_OORVAL, line, node->value_str, node->schema->name);
                    return EXIT_FAILURE;
                }
                dec[i] = node->value_str[c + i];
            }
            if (j) {
                j--;
            }
        }

        if (parse_int(dec, __INT64_C(-9223372036854775807) - __INT64_C(1), __INT64_C(9223372036854775807), 10, &num, node->schema->name, line)
                || validate_length_range(2, 0, 0, ((long double)num)/(1 << type->info.dec64.dig), stype,
                                         node->value_str, line)) {
            return EXIT_FAILURE;
        }
        node->value.dec64 = num;
        break;

    case LY_TYPE_EMPTY:
        /* just check that it is empty */
        if (node->value_str && node->value_str[0]) {
            LOGVAL(LYE_INVAL, line, node->value_str, node->schema->name);
            return EXIT_FAILURE;
        }
        break;

    case LY_TYPE_ENUM:
        if (!node->value_str) {
            LOGVAL(LYE_INVAL, line, "", node->schema->name);
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
            LOGVAL(LYE_INVAL, line, node->value_str, node->schema->name);
            return EXIT_FAILURE;
        }

        break;

    case LY_TYPE_IDENT:
        if (!node->value_str) {
            LOGVAL(LYE_INVAL, line, "", node->schema->name);
            return EXIT_FAILURE;
        }

        node->value.ident = resolve_identref(stype->info.ident.ref, node->value_str, line);
        if (!node->value.ident) {
            return EXIT_FAILURE;
        }
        break;

    case LY_TYPE_INST:
        if (!node->value_str) {
            LOGVAL(LYE_INVAL, line, "", node->schema->name);
            return EXIT_FAILURE;
        }

        if (!resolve) {
            node->value_type |= LY_TYPE_INST_UNRES;
        } else {
            /* validity checking is performed later, right now the data tree
             * is not complete, so many instanceids cannot be resolved
             */
            if (unres) {
                if (unres_data_add(unres, (struct lyd_node *)node, UNRES_INSTID, line)) {
                    return EXIT_FAILURE;
                }
            } else {
                if (resolve_unres_data_item((struct lyd_node *)node, UNRES_INSTID, 0, line)) {
                    return EXIT_FAILURE;
                }
            }
        }
        break;

    case LY_TYPE_LEAFREF:
        if (!node->value_str) {
            LOGVAL(LYE_INVAL, line, "", node->schema->name);
            return EXIT_FAILURE;
        }

        if (!resolve) {
            type = &((struct lys_node_leaf *)node->schema)->type.info.lref.target->type;
            while (type->base == LY_TYPE_LEAFREF) {
                type = &type->info.lref.target->type;
            }
            node->value_type = type->base | LY_TYPE_LEAFREF_UNRES;
        } else {
            /* validity checking is performed later, right now the data tree
             * is not complete, so many noderefs cannot be resolved
             */
            if (unres) {
                if (unres_data_add(unres, (struct lyd_node *)node, UNRES_LEAFREF, line)) {
                    return EXIT_FAILURE;
                }
            } else {
                if (resolve_unres_data_item((struct lyd_node *)node, UNRES_LEAFREF, 0, line)) {
                    return EXIT_FAILURE;
                }
            }
        }
        break;

    case LY_TYPE_STRING:
        if (validate_length_range(0, (node->value_str ? strlen(node->value_str) : 0), 0, 0, stype,
                                  node->value_str, line)) {
            return EXIT_FAILURE;
        }

        if (validate_pattern(node->value_str, stype, node->schema->name, line)) {
            return EXIT_FAILURE;
        }

        node->value.string = node->value_str;
        break;

    case LY_TYPE_INT8:
        if (parse_int(node->value_str, __INT64_C(-128), __INT64_C(127), 0, &num, node->schema->name, line)
                || validate_length_range(1, 0, num, 0, stype, node->value_str, line)) {
            return EXIT_FAILURE;
        }
        node->value.int8 = num;
        break;

    case LY_TYPE_INT16:
        if (parse_int(node->value_str, __INT64_C(-32768), __INT64_C(32767), 0, &num, node->schema->name, line)
                || validate_length_range(1, 0, num, 0, stype, node->value_str, line)) {
            return EXIT_FAILURE;
        }
        node->value.int16 = num;
        break;

    case LY_TYPE_INT32:
        if (parse_int(node->value_str, __INT64_C(-2147483648), __INT64_C(2147483647), 0, &num, node->schema->name, line)
                || validate_length_range(1, 0, num, 0, stype, node->value_str, line)) {
            return EXIT_FAILURE;
        }
        node->value.int32 = num;
        break;

    case LY_TYPE_INT64:
        if (parse_int(node->value_str, __INT64_C(-9223372036854775807) - __INT64_C(1), __INT64_C(9223372036854775807), 0, &num,
                node->schema->name, line)
                || validate_length_range(1, 0, num, 0, stype, node->value_str, line)) {
            return EXIT_FAILURE;
        }
        node->value.int64 = num;
        break;

    case LY_TYPE_UINT8:
        if (parse_uint(node->value_str, __UINT64_C(255), __UINT64_C(0), &unum, node->schema->name, line)
                || validate_length_range(0, unum, 0, 0, stype, node->value_str, line)) {
            return EXIT_FAILURE;
        }
        node->value.uint8 = unum;
        break;

    case LY_TYPE_UINT16:
        if (parse_uint(node->value_str, __UINT64_C(65535), __UINT64_C(0), &unum, node->schema->name, line)
                || validate_length_range(0, unum, 0, 0, stype, node->value_str, line)) {
            return EXIT_FAILURE;
        }
        node->value.uint16 = unum;
        break;

    case LY_TYPE_UINT32:
        if (parse_uint(node->value_str, __UINT64_C(4294967295), __UINT64_C(0), &unum, node->schema->name, line)
                || validate_length_range(0, unum, 0, 0, stype, node->value_str, line)) {
            return EXIT_FAILURE;
        }
        node->value.uint32 = unum;
        break;

    case LY_TYPE_UINT64:
        if (parse_uint(node->value_str, __UINT64_C(18446744073709551615), __UINT64_C(0), &unum, node->schema->name, line)
                || validate_length_range(0, unum, 0, 0, stype, node->value_str, line)) {
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
lyp_parse_value(struct lyd_node_leaf_list *leaf, struct lyxml_elem *xml, int resolve, struct unres_data *unres, int line)
{
    int found = 0;
    struct lys_type *type, *stype;

    assert(leaf);

    stype = &((struct lys_node_leaf *)leaf->schema)->type;
    if (stype->base == LY_TYPE_UNION) {
        type = lyp_get_next_union_type(stype, NULL, &found);
        while (type) {
            leaf->value_type = type->base;

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

            if (!lyp_parse_value_(leaf, type, resolve, unres, UINT_MAX)) {
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

        if (!type) {
            /* failure */
            LOGVAL(LYE_INVAL, line, (leaf->value_str ? leaf->value_str : ""), leaf->schema->name);
            return EXIT_FAILURE;
        }
    } else if (lyp_parse_value_(leaf, stype, resolve, unres, line)) {
        ly_errno = LY_EVALID;
        return EXIT_FAILURE;
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

int
check_status(uint8_t flags1, struct lys_module *mod1, const char *name1,
             uint8_t flags2, struct lys_module *mod2, const char *name2, unsigned int line)
{
    uint8_t flg1, flg2;

    flg1 = (flags1 & LYS_STATUS_MASK) ? (flags1 & LYS_STATUS_MASK) : LYS_STATUS_CURR;
    flg2 = (flags2 & LYS_STATUS_MASK) ? (flags2 & LYS_STATUS_MASK) : LYS_STATUS_CURR;

    if ((flg1 < flg2) && (mod1 == mod2)) {
        LOGVAL(LYE_INSTATUS, line, flg1 == LYS_STATUS_CURR ? "current" : "deprecated", name1,
               flg2 == LYS_STATUS_OBSLT ? "obsolete" : "deprecated", name2);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
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
pututf8(char *dst, int32_t value, uint32_t lineno)
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
        LOGVAL(LYE_SPEC, lineno, "Invalid UTF-8 value 0x%08x", value);
        return 0;
    }
}
