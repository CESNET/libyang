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

#define LYP_URANGE_LEN 19

static char *lyp_ublock2urange[][2] = {
    {"BasicLatin", "[\\x{0000}-\\x{007F}]"},
    {"Latin-1Supplement", "[\\x{0080}-\\x{00FF}]"},
    {"LatinExtended-A", "[\\x{0100}-\\x{017F}]"},
    {"LatinExtended-B", "[\\x{0180}-\\x{024F}]"},
    {"IPAExtensions", "[\\x{0250}-\\x{02AF}]"},
    {"SpacingModifierLetters", "[\\x{02B0}-\\x{02FF}]"},
    {"CombiningDiacriticalMarks", "[\\x{0300}-\\x{036F}]"},
    {"Greek", "[\\x{0370}-\\x{03FF}]"},
    {"Cyrillic", "[\\x{0400}-\\x{04FF}]"},
    {"Armenian", "[\\x{0530}-\\x{058F}]"},
    {"Hebrew", "[\\x{0590}-\\x{05FF}]"},
    {"Arabic", "[\\x{0600}-\\x{06FF}]"},
    {"Syriac", "[\\x{0700}-\\x{074F}]"},
    {"Thaana", "[\\x{0780}-\\x{07BF}]"},
    {"Devanagari", "[\\x{0900}-\\x{097F}]"},
    {"Bengali", "[\\x{0980}-\\x{09FF}]"},
    {"Gurmukhi", "[\\x{0A00}-\\x{0A7F}]"},
    {"Gujarati", "[\\x{0A80}-\\x{0AFF}]"},
    {"Oriya", "[\\x{0B00}-\\x{0B7F}]"},
    {"Tamil", "[\\x{0B80}-\\x{0BFF}]"},
    {"Telugu", "[\\x{0C00}-\\x{0C7F}]"},
    {"Kannada", "[\\x{0C80}-\\x{0CFF}]"},
    {"Malayalam", "[\\x{0D00}-\\x{0D7F}]"},
    {"Sinhala", "[\\x{0D80}-\\x{0DFF}]"},
    {"Thai", "[\\x{0E00}-\\x{0E7F}]"},
    {"Lao", "[\\x{0E80}-\\x{0EFF}]"},
    {"Tibetan", "[\\x{0F00}-\\x{0FFF}]"},
    {"Myanmar", "[\\x{1000}-\\x{109F}]"},
    {"Georgian", "[\\x{10A0}-\\x{10FF}]"},
    {"HangulJamo", "[\\x{1100}-\\x{11FF}]"},
    {"Ethiopic", "[\\x{1200}-\\x{137F}]"},
    {"Cherokee", "[\\x{13A0}-\\x{13FF}]"},
    {"UnifiedCanadianAboriginalSyllabics", "[\\x{1400}-\\x{167F}]"},
    {"Ogham", "[\\x{1680}-\\x{169F}]"},
    {"Runic", "[\\x{16A0}-\\x{16FF}]"},
    {"Khmer", "[\\x{1780}-\\x{17FF}]"},
    {"Mongolian", "[\\x{1800}-\\x{18AF}]"},
    {"LatinExtendedAdditional", "[\\x{1E00}-\\x{1EFF}]"},
    {"GreekExtended", "[\\x{1F00}-\\x{1FFF}]"},
    {"GeneralPunctuation", "[\\x{2000}-\\x{206F}]"},
    {"SuperscriptsandSubscripts", "[\\x{2070}-\\x{209F}]"},
    {"CurrencySymbols", "[\\x{20A0}-\\x{20CF}]"},
    {"CombiningMarksforSymbols", "[\\x{20D0}-\\x{20FF}]"},
    {"LetterlikeSymbols", "[\\x{2100}-\\x{214F}]"},
    {"NumberForms", "[\\x{2150}-\\x{218F}]"},
    {"Arrows", "[\\x{2190}-\\x{21FF}]"},
    {"MathematicalOperators", "[\\x{2200}-\\x{22FF}]"},
    {"MiscellaneousTechnical", "[\\x{2300}-\\x{23FF}]"},
    {"ControlPictures", "[\\x{2400}-\\x{243F}]"},
    {"OpticalCharacterRecognition", "[\\x{2440}-\\x{245F}]"},
    {"EnclosedAlphanumerics", "[\\x{2460}-\\x{24FF}]"},
    {"BoxDrawing", "[\\x{2500}-\\x{257F}]"},
    {"BlockElements", "[\\x{2580}-\\x{259F}]"},
    {"GeometricShapes", "[\\x{25A0}-\\x{25FF}]"},
    {"MiscellaneousSymbols", "[\\x{2600}-\\x{26FF}]"},
    {"Dingbats", "[\\x{2700}-\\x{27BF}]"},
    {"BraillePatterns", "[\\x{2800}-\\x{28FF}]"},
    {"CJKRadicalsSupplement", "[\\x{2E80}-\\x{2EFF}]"},
    {"KangxiRadicals", "[\\x{2F00}-\\x{2FDF}]"},
    {"IdeographicDescriptionCharacters", "[\\x{2FF0}-\\x{2FFF}]"},
    {"CJKSymbolsandPunctuation", "[\\x{3000}-\\x{303F}]"},
    {"Hiragana", "[\\x{3040}-\\x{309F}]"},
    {"Katakana", "[\\x{30A0}-\\x{30FF}]"},
    {"Bopomofo", "[\\x{3100}-\\x{312F}]"},
    {"HangulCompatibilityJamo", "[\\x{3130}-\\x{318F}]"},
    {"Kanbun", "[\\x{3190}-\\x{319F}]"},
    {"BopomofoExtended", "[\\x{31A0}-\\x{31BF}]"},
    {"EnclosedCJKLettersandMonths", "[\\x{3200}-\\x{32FF}]"},
    {"CJKCompatibility", "[\\x{3300}-\\x{33FF}]"},
    {"CJKUnifiedIdeographsExtensionA", "[\\x{3400}-\\x{4DB5}]"},
    {"CJKUnifiedIdeographs", "[\\x{4E00}-\\x{9FFF}]"},
    {"YiSyllables", "[\\x{A000}-\\x{A48F}]"},
    {"YiRadicals", "[\\x{A490}-\\x{A4CF}]"},
    {"HangulSyllables", "[\\x{AC00}-\\x{D7A3}]"},
    {"PrivateUse", "[\\x{E000}-\\x{F8FF}]"},
    {"CJKCompatibilityIdeographs", "[\\x{F900}-\\x{FAFF}]"},
    {"AlphabeticPresentationForms", "[\\x{FB00}-\\x{FB4F}]"},
    {"ArabicPresentationForms-A", "[\\x{FB50}-\\x{FDFF}]"},
    {"CombiningHalfMarks", "[\\x{FE20}-\\x{FE2F}]"},
    {"CJKCompatibilityForms", "[\\x{FE30}-\\x{FE4F}]"},
    {"SmallFormVariants", "[\\x{FE50}-\\x{FE6F}]"},
    {"ArabicPresentationForms-B", "[\\x{FE70}-\\x{FEFE}]"},
    {"HalfwidthandFullwidthForms", "[\\x{FF00}-\\x{FFEF}]"},
    {NULL, NULL}
};

int
lyp_is_rpc(struct lys_node *node)
{
    assert(node);

    while (lys_parent(node)) {
        node = lys_parent(node);
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

    if (!sb.st_size) {
        LOGERR(LY_EINVAL, "File empty.");
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
        LOGERR(LY_EINVAL, "%s: Invalid format parameter.", __func__);
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
    size_t len, flen, match_len = 0, dir_len;
    int fd;
    char *wd, *cwd;
    DIR *dir;
    struct dirent *file;
    char *match_name = NULL;
    LYS_INFORMAT format, match_format = 0;
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
    dir_len = strlen(wd);
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
                } else {
                    /* exact revision */
                    free(match_name);
                    asprintf(&match_name, "%s/%s", wd, file->d_name);
                    match_len = dir_len + 1 + len;
                    match_format = format;
                    goto matched;
                }
            } else {
                /* continue trying to find exact revision match, use this only if not found */
                free(match_name);
                asprintf(&match_name, "%s/%s", wd, file->d_name);
                match_len = dir_len + 1 +len;
                match_format = format;
                continue;
            }
        } else {
            /* remember the revision and try to find the newest one */
            if (match_name) {
                int a;
                if (file->d_name[len] != '@' || lyp_check_date(&file->d_name[len + 1])) {
                    continue;
                } else if (match_name[match_len] == '@' &&
                    (a = strncmp(&match_name[match_len + 1], &file->d_name[len + 1], LY_REV_SIZE - 1)) >= 0) {
                    continue;
                }
                free(match_name);
            }

            asprintf(&match_name, "%s/%s", wd, file->d_name);
            match_len = dir_len + 1 + len;
            match_format = format;
            continue;
        }
    }

searchpath:
    if (!ctx->models.search_path) {
        LOGWRN("No search path defined for the current context.");
    } else if (localsearch) {
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

    if (!match_name) {
        LOGERR(LY_ESYS, "Data model \"%s\" not found (search path is \"%s\")", name, ctx->models.search_path);
        goto cleanup;
    }

matched:
    /* open the file */
    fd = open(match_name, O_RDONLY);
    if (fd < 0) {
        LOGERR(LY_ESYS, "Unable to open data model file \"%s\" (%s).",
               match_name, strerror(errno));
        goto cleanup;
    }

    /* go back to cwd if changed */
    if (cwd != wd) {
        if (chdir(cwd)) {
            LOGWRN("Unable to return back to working directory \"%s\" (%s)",
                   cwd, strerror(errno));
        }
        free(wd);
        wd = cwd;
    }

    if (module) {
        result = (struct lys_module *)lys_submodule_read(module, fd, match_format, unres);
    } else {
        result = lys_read_import(ctx, fd, match_format, revision);
    }
    close(fd);

    if (!result) {
        goto cleanup;
    }

    result->filepath = lydict_insert_zc(ctx, match_name);
    match_name = NULL;
    /* success */

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
    free(match_name);

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
        LOGVAL(LYE_NOCONSTR, LY_VLOG_LYD, node, val_str);
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
        LOGVAL(LYE_NOCONSTR, LY_VLOG_LYD, node, val_str);
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
    struct lys_restr *restr = NULL;
    struct len_ran_intv *intv = NULL, *tmp_intv;
    struct lys_type *cur_type;
    int match;

    if (resolve_len_ran_interval(NULL, type, &intv)) {
        /* already done during schema parsing */
        LOGINT;
        return EXIT_FAILURE;
    }
    if (!intv) {
        return EXIT_SUCCESS;
    }

    /* I know that all intervals belonging to a single restriction share one type pointer */
    tmp_intv = intv;
    cur_type = intv->type;
    do {
        match = 0;
        for (; tmp_intv && (tmp_intv->type == cur_type); tmp_intv = tmp_intv->next) {
            if (match) {
                /* just iterate through the rest of this restriction intervals */
                continue;
            }

            if (((kind == 0) && (unum < tmp_intv->value.uval.min))
                    || ((kind == 1) && (snum < tmp_intv->value.sval.min))
                    || ((kind == 2) && (fnum < tmp_intv->value.fval.min))) {
                break;
            }

            if (((kind == 0) && (unum >= tmp_intv->value.uval.min) && (unum <= tmp_intv->value.uval.max))
                    || ((kind == 1) && (snum >= tmp_intv->value.sval.min) && (snum <= tmp_intv->value.sval.max))
                    || ((kind == 2) && (fnum >= tmp_intv->value.fval.min) && (fnum <= tmp_intv->value.fval.max))) {
                match = 1;
            }
        }

        if (!match) {
            break;
        } else if (tmp_intv) {
            cur_type = tmp_intv->type;
        }
    } while (tmp_intv);

    while (intv) {
        tmp_intv = intv->next;
        free(intv);
        intv = tmp_intv;
    }

    if (!match) {
        switch (cur_type->base) {
        case LY_TYPE_BINARY:
            restr = cur_type->info.binary.length;
            break;
        case LY_TYPE_DEC64:
            restr = cur_type->info.dec64.range;
            break;
        case LY_TYPE_INT8:
        case LY_TYPE_INT16:
        case LY_TYPE_INT32:
        case LY_TYPE_INT64:
        case LY_TYPE_UINT8:
        case LY_TYPE_UINT16:
        case LY_TYPE_UINT32:
        case LY_TYPE_UINT64:
            restr = cur_type->info.num.range;
            break;
        case LY_TYPE_STRING:
            restr = cur_type->info.str.length;
            break;
        default:
            LOGINT;
            return EXIT_FAILURE;
        }

        LOGVAL(LYE_NOCONSTR, LY_VLOG_LYD, node, (val_str ? val_str : ""));
        if (restr && restr->emsg) {
            LOGVAL(LYE_SPEC, LY_VLOG_LYD, node, restr->emsg);
        }
        if (restr && restr->eapptag) {
            strncpy(((struct ly_err *)&ly_errno)->apptag, restr->eapptag, LY_APPTAG_LEN - 1);
        }
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/* logs directly */
static int
validate_pattern(const char *val_str, struct lys_type *type, struct lyd_node *node)
{
    int i;
    pcre *precomp;

    assert(type->base == LY_TYPE_STRING);

    if (!val_str) {
        val_str = "";
    }

    if (type->der && validate_pattern(val_str, &type->der->type, node)) {
        return EXIT_FAILURE;
    }

    for (i = 0; i < type->info.str.pat_count; ++i) {
        if (lyp_check_pattern(type->info.str.patterns[i].expr, &precomp)) {
            LOGINT;
            return EXIT_FAILURE;
        }

        if (pcre_exec(precomp, NULL, val_str, strlen(val_str), 0, 0, NULL, 0)) {
            LOGVAL(LYE_NOCONSTR, LY_VLOG_LYD, node, val_str);
            if (type->info.str.patterns[i].emsg) {
                LOGVAL(LYE_SPEC, LY_VLOG_LYD, node, type->info.str.patterns[i].emsg);
            }
            if (type->info.str.patterns[i].eapptag) {
                strncpy(((struct ly_err *)&ly_errno)->apptag, type->info.str.patterns[i].eapptag, LY_APPTAG_LEN - 1);
            }
            free(precomp);
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
            if (type->base == LY_TYPE_UINT64) {
                strtoull(c, &tail, 10);
            } else {
                strtoll(c, &tail, 10);
            }
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
        if (type->base == LY_TYPE_UINT64) {
            strtoull(c, &tail, 10);
        } else {
            strtoll(c, &tail, 10);
        }
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
            /* process next length-part */
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
    if (resolve_len_ran_interval(expr, type, &intv)) {
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

/**
 * @brief Checks pattern syntax. Logs directly.
 *
 * @param[in] pattern Pattern to check.
 * @param[out] pcre_precomp Precompiled PCRE pattern. Can be NULL.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE otherwise.
 */
int
lyp_check_pattern(const char *pattern, pcre **pcre_precomp)
{
    int idx, start, end, err_offset;
    char *perl_regex, *ptr;
    const char *err_msg;
    pcre *precomp;

    /*
     * adjust the expression to a Perl equivalent
     *
     * http://www.w3.org/TR/2004/REC-xmlschema-2-20041028/#regexs
     */
    perl_regex = malloc((strlen(pattern) + 2) * sizeof(char));
    if (!perl_regex) {
        LOGMEM;
        return EXIT_FAILURE;
    }
    strcpy(perl_regex, pattern);
    if (strncmp(pattern + strlen(pattern) - 2, ".*", 2)) {
        strcat(perl_regex, "$");
    }

    /* substitute Unicode Character Blocks with exact Character Ranges */
    while ((ptr = strstr(perl_regex, "\\p{Is"))) {
        start = ptr - perl_regex;

        ptr = strchr(ptr, '}');
        if (!ptr) {
            LOGVAL(LYE_INREGEX, LY_VLOG_NONE, NULL, pattern, perl_regex + start + 2, "unterminated character property");
            free(perl_regex);
            return EXIT_FAILURE;
        }

        end = (ptr - perl_regex) + 1;

        /* need more space */
        if (end - start < LYP_URANGE_LEN) {
            perl_regex = ly_realloc(perl_regex, strlen(perl_regex) + (LYP_URANGE_LEN - (end - start)) + 1);
            if (!perl_regex) {
                LOGMEM;
                free(perl_regex);
                return EXIT_FAILURE;
            }
        }

        /* find our range */
        for (idx = 0; lyp_ublock2urange[idx][0]; ++idx) {
            if (!memcmp(perl_regex + start + 5, lyp_ublock2urange[idx][0], strlen(lyp_ublock2urange[idx][0]))) {
                break;
            }
        }
        if (!lyp_ublock2urange[idx][0]) {
            LOGVAL(LYE_INREGEX, LY_VLOG_NONE, NULL, pattern, perl_regex + start + 5, "unknown block name");
            free(perl_regex);
            return EXIT_FAILURE;
        }

        /* make the space in the string and replace the block */
        memmove(perl_regex + start + LYP_URANGE_LEN, perl_regex + end, strlen(perl_regex + end) + 1);
        memcpy(perl_regex + start, lyp_ublock2urange[idx][1], LYP_URANGE_LEN);
    }

    /* must return 0, already checked during parsing */
    precomp = pcre_compile(perl_regex, PCRE_ANCHORED | PCRE_DOLLAR_ENDONLY | PCRE_NO_AUTO_CAPTURE,
                           &err_msg, &err_offset, NULL);
    free(perl_regex);
    if (!precomp) {
        LOGVAL(LYE_INREGEX, LY_VLOG_NONE, NULL, pattern, pattern + err_offset, err_msg);
        return EXIT_FAILURE;
    }

    if (pcre_precomp) {
        *pcre_precomp = precomp;
    } else {
        free(precomp);
    }

    return EXIT_SUCCESS;
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
                    LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, node->value_str, node->schema->name);
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
                    LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, node->value_str, node->schema->name);
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
                || validate_length_range(2, 0, 0, ((long double)num) / type->info.dec64.div, stype,
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
                /* success, erase set ly_errno and ly_vecode */
                ly_errno = LY_SUCCESS;
                ly_vecode = LYVE_SUCCESS;
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
        for (; parent; parent = lys_parent(parent)) {
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
lyp_check_status(uint16_t flags1, struct lys_module *mod1, const char *name1,
                 uint16_t flags2, struct lys_module *mod2, const char *name2,
                 const struct lys_node *node)
{
    uint16_t flg1, flg2;

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

static void
lyp_check_circmod_pop(struct lys_module *module)
{
    struct ly_modules_list *models = &module->ctx->models;

    /* update the list of currently being parsed modules */
    models->parsing_number--;
    if (models->parsing_number == 1) {
        free(models->parsing);
        models->parsing = NULL;
        models->parsing_number = models->parsing_size = 0;
    } else {
        models->parsing[models->parsing_number] = NULL;
    }
}

/*
 * types: 0 - include, 1 - import
 */
static int
lyp_check_circmod(struct lys_module *module, const char *value, int type)
{
    LY_ECODE code = type ? LYE_CIRC_IMPORTS : LYE_CIRC_INCLUDES;
    struct ly_modules_list *models = &module->ctx->models;
    int i;

    /* circular import check */
    if (!models->parsing_size) {
        if (ly_strequal(module->name, value, 1)) {
            LOGVAL(code, LY_VLOG_NONE, NULL, value);
            return -1;
        }

        /* storing - first import, besides the module being imported, add also the starting module */
        models->parsing_size = models->parsing_number = 2;
        models->parsing = malloc(2 * sizeof *models->parsing);
        if (!models->parsing) {
            LOGMEM;
            return -1;
        }
        models->parsing[0] = module->name;
        models->parsing[1] = value;
    } else {
        for (i = 0; i < models->parsing_number; i++) {
            if (ly_strequal(models->parsing[i], value, 1)) {
                LOGVAL(code, LY_VLOG_NONE, NULL, value);
                return -1;
            }
        }
        /* storing - enlarge the list of modules being currently parsed */
        models->parsing_number++;
        if (models->parsing_number >= models->parsing_size) {
            models->parsing_size++;
            models->parsing = ly_realloc(models->parsing, models->parsing_size * sizeof *models->parsing);
            if (!models->parsing) {
                LOGMEM;
                return -1;
            }
        }
        models->parsing[models->parsing_number - 1] = value;
    }

    return 0;
}

/* returns:
 *  0 - inc successfully filled
 * -1 - error, inc is cleaned
 *  1 - duplication, ignore the inc structure, inc is cleaned
 */
int
lyp_check_include(struct lys_module *module, struct lys_submodule *submodule, const char *value,
                  struct lys_include *inc, struct unres_schema *unres)
{
    char *module_data;
    void (*module_data_free)(void *module_data) = NULL;
    LYS_INFORMAT format = LYS_IN_UNKNOWN;
    int i, j;

    /* check that the submodule was not included yet (previous submodule could have included it) */
    for (i = 0; i < module->inc_size; ++i) {
        if (!module->inc[i].submodule) {
            /* skip the not yet filled records */
            continue;
        }
        if (ly_strequal(module->inc[i].submodule->name, value, 1)) {
            /* check revisions, including multiple revisions of a single module is error */
            if (inc->rev[0] && (!module->inc[i].submodule->rev_size || strcmp(module->inc[i].submodule->rev[0].date, inc->rev))) {
                /* the already included submodule has
                 * - no revision, but here we require some
                 * - different revision than the one required here */
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "include");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Including multiple revisions of submodule \"%s\".", value);
                return -1;
            }
            /* we want to load module, which is already included in the main module */
            if (!submodule && !module->inc[i].external) {
                /* it was already included by the main module */
                LOGWRN("Duplicated include of the \"%s\" submodule in the \"%s\" module.", value, module->name);
            } else if (submodule && module->inc[i].external) {
                for (j = 0; j < submodule->inc_size && submodule->inc[j].submodule; j++) {
                    if (ly_strequal(submodule->inc[j].submodule->name, value, 1)) {
                        LOGWRN("Duplicated include of the \"%s\" submodule in the \"%s\" submodule.", value, submodule->name);
                        break;
                    }
                }
            }

            if (!submodule) {
                /* the included submodule is no longer external */
                module->inc[i].external = 0;
            }
            return 1;
        }
    }

    /* circular include check */
    if (lyp_check_circmod(module, value, 0)) {
        return -1;
    }

    /* try to load the submodule */
    inc->submodule = (struct lys_submodule *)ly_ctx_get_submodule2(module, value);
    if (inc->submodule) {
        if (inc->rev[0]) {
            if (!inc->submodule->rev_size || !ly_strequal(inc->rev, inc->submodule->rev[0].date, 1)) {
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, inc->rev[0], "revision");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Multiple revisions of the same submodule included.");
                lyp_check_circmod_pop(module);
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

    /* update the list of currently being parsed modules */
    lyp_check_circmod_pop(module);

    /* check the result */
    if (!inc->submodule) {
        if (!ly_vecode) {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "include");
        }
        LOGERR(LY_EVALID, "Including \"%s\" module into \"%s\" failed.", value, module->name);
        goto error;
    }

    /* propagate submodule's includes and imports into the main module */
    if (submodule && lyp_propagate_submodule(module, inc)) {
        goto error;
    }

    return 0;

error:

    return -1;
}

/* returns:
 *  0 - imp successfully filled
 * -1 - error, imp not cleaned
 */
int
lyp_check_import(struct lys_module *module, const char *value, struct lys_import *imp)
{
    int i;
    struct lys_module *dup = NULL;
    LY_LOG_LEVEL verb;

    /* check for importing a single module in multiple revisions */
    for (i = 0; i < module->imp_size; i++) {
        if (!module->imp[i].module) {
            /* skip the not yet filled records */
            continue;
        }
        if (ly_strequal(module->imp[i].module->name, value, 1)) {
            /* check revisions, including multiple revisions of a single module is error */
            if (imp->rev[0] && (!module->imp[i].module->rev_size || strcmp(module->imp[i].module->rev[0].date, imp->rev))) {
                /* the already imported module has
                 * - no revision, but here we require some
                 * - different revision than the one required here */
                LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "import");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Importing multiple revisions of module \"%s\".", value);
                return -1;
            } else if (!imp->rev[0]) {
                /* no revision, remember the duplication, but check revisions after loading the module
                 * because the current revision can be the same (then it is ok) or it can differ (then it
                 * is error */
                dup = module->imp[i].module;
                break;
            }

            /* there is duplication, but since prefixes differs (checked in caller of this function),
             * it is ok */
            imp->module = module->imp[i].module;
            return 0;
        }
    }

    /* circular import check */
    if (lyp_check_circmod(module, value, 1)) {
        return -1;
    }

    /* try to load the module */
    if (!imp->rev[0]) {
        /* no revision specified, try to load the newest module from the search locations into the context */
        verb = ly_log_level;
        ly_verb(LY_LLSILENT);
        ly_ctx_load_module(module->ctx, value, imp->rev[0] ? imp->rev : NULL);
        ly_verb(verb);
        if (ly_errno == LY_ESYS) {
            /* it is ok, that the e.g. input file was not found */
            ly_errno = LY_SUCCESS;
        } else if (ly_errno != LY_SUCCESS) {
            /* but it is not ok if e.g. the input data were found and they are invalid */
            lyp_check_circmod_pop(module);
            return -1;
        }

        /* If the loaded module (if any) is really the newest, it will be loaded on the next line
         * by ly_ctx_get_module() */
    }
    imp->module = (struct lys_module *)ly_ctx_get_module(module->ctx, value, imp->rev[0] ? imp->rev : NULL);
    if (!imp->module) {
        /* whether to use a user callback is decided in the function */
        imp->module = (struct lys_module *)ly_ctx_load_module(module->ctx, value, imp->rev[0] ? imp->rev : NULL);
    }

    /* update the list of currently being parsed modules */
    lyp_check_circmod_pop(module);

    /* check the result */
    if (!imp->module) {
        if (!ly_vecode) {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "import");
        }
        LOGERR(LY_EVALID, "Importing \"%s\" module into \"%s\" failed.", value, module->name);
        return -1;
    }

    if (dup) {
        /* check the revisions */
        if ((dup != imp->module) ||
                (dup->rev_size != imp->module->rev_size && (!dup->rev_size || imp->module->rev_size)) ||
                (dup->rev_size && strcmp(dup->rev[0].date, imp->module->rev[0].date))) {
            /* - modules are not the same
             * - one of modules has no revision (except they both has no revision)
             * - revisions of the modules are not the same */
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "import");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Importing multiple revisions of module \"%s\".", value);
            return -1;
        }
    }

    return 0;
}

/* Propagate imports and includes into the main module */
int
lyp_propagate_submodule(struct lys_module *module, struct lys_include *inc)
{
    uint8_t i, j;
    size_t size;
    struct lys_include *aux_inc;
    struct lys_import *aux_imp;
    struct lys_import *impiter;
    struct ly_set *set;

    set = ly_set_new();

    /* propagate imports into the main module */
    for (i = 0; i < inc->submodule->imp_size; i++) {
        for (j = 0; j < module->imp_size; j++) {
            if (inc->submodule->imp[i].module == module->imp[j].module &&
                    !strcmp(inc->submodule->imp[i].rev, module->imp[j].rev)) {
                /* check prefix match */
                if (!ly_strequal(inc->submodule->imp[i].prefix, module->imp[j].prefix, 1)) {
                    LOGVAL(LYE_INID, LY_VLOG_NONE, NULL, inc->submodule->imp[i].prefix,
                           "non-matching prefixes of imported module in main module and submodule");
                    goto error;
                }
                break;
            }
        }
        if (j == module->imp_size) {
            /* new import */
            ly_set_add(set, &inc->submodule->imp[i], LY_SET_OPT_USEASLIST);
        }
    }
    if (set->number) {
        if (!(void*)module->imp) {
            /* no import array in main module */
            i = 0;
        } else {
            /* get array size by searching for stop block */
            for (i = 0; (void*)module->imp[i].module != (void*)0x1; i++);
        }
        size = (i + set->number) * sizeof *module->imp;
        aux_imp = realloc(module->imp, size + sizeof(void*));
        if (!aux_imp) {
            LOGMEM;
            goto error;
        }
        module->imp = aux_imp;
        memset(&module->imp[module->imp_size + set->number], 0, (i - module->imp_size) * sizeof *module->imp);
        module->imp[i + set->number].module = (void*)0x1; /* set stop block */

        for (i = 0; i < set->number; i++) {
            impiter = (struct lys_import *)set->set.g[i];

            /* check prefix uniqueness */
            if (dup_prefix_check(impiter->prefix, module)) {
                LOGVAL(LYE_DUPID, LY_VLOG_NONE, NULL, "prefix", impiter->prefix);
                goto error;
            }

            memcpy(&module->imp[module->imp_size], impiter, sizeof *module->imp);
            module->imp[module->imp_size].prefix = lydict_insert(module->ctx, impiter->prefix, 0);
            module->imp[module->imp_size].external = 1;
            module->imp_size++;
        }
    }
    ly_set_free(set);
    set = NULL;

    /* propagate the included submodule into the main module */
    for (i = 0; (void*)module->inc[i].submodule != (void*)0x1; i++); /* get array size by searching for stop block */
    size = (i + 1) * sizeof *module->inc;
    aux_inc = realloc(module->inc, size + sizeof(void*));
    if (!aux_inc) {
        LOGMEM;
        goto error;
    }
    module->inc = aux_inc;
    memset(&module->inc[module->inc_size + 1], 0, (i - module->inc_size) * sizeof *module->inc);
    module->inc[i + 1].submodule = (void*)0x1; /* set stop block */

    memcpy(&module->inc[module->inc_size], inc, sizeof *module->inc);
    module->inc[module->inc_size].external = 1;
    module->inc_size++;

    return EXIT_SUCCESS;

error:
    ly_set_free(set);
    return EXIT_FAILURE;
}

int
lyp_ctx_add_module(struct lys_module **module)
{
    struct ly_ctx *ctx;
    struct lys_module **newlist = NULL;
    struct lys_module *mod;
    int i, match_i = -1, to_implement;

    assert(module);
    mod = (*module);
    to_implement = 0;
    ctx = mod->ctx;

    /* add to the context's list of modules */
    if (ctx->models.used == ctx->models.size) {
        newlist = realloc(ctx->models.list, (2 * ctx->models.size) * sizeof *newlist);
        if (!newlist) {
            LOGMEM;
            return EXIT_FAILURE;
        }
        for (i = ctx->models.size; i < ctx->models.size * 2; i++) {
            newlist[i] = NULL;
        }
        ctx->models.size *= 2;
        ctx->models.list = newlist;
    }

    for (i = 0; ctx->models.list[i]; i++) {
        /* check name (name/revision) and namespace uniqueness */
        if (!strcmp(ctx->models.list[i]->name, mod->name)) {
            if (to_implement) {
                if (i == match_i) {
                    continue;
                }
                LOGERR(LY_EINVAL, "Module \"%s\" in another revision already implemented.", ctx->models.list[i]->name);
                return EXIT_FAILURE;
            } else if (!ctx->models.list[i]->rev_size && mod->rev_size) {
                LOGERR(LY_EINVAL, "Module \"%s\" without revision already in context.", ctx->models.list[i]->name);
                return EXIT_FAILURE;
            } else if (ctx->models.list[i]->rev_size && !mod->rev_size) {
                LOGERR(LY_EINVAL, "Module \"%s\" with revision already in context.", ctx->models.list[i]->name);
                return EXIT_FAILURE;
            } else if ((!mod->rev_size && !ctx->models.list[i]->rev_size)
                    || !strcmp(ctx->models.list[i]->rev[0].date, mod->rev[0].date)) {

                LOGVRB("Module \"%s\" already in context.", ctx->models.list[i]->name);
                to_implement = mod->implemented;
                match_i = i;
                if (to_implement && !ctx->models.list[i]->implemented) {
                    /* check first that it is okay to change it to implemented */
                    i = -1;
                    continue;
                }
                goto already_in_context;

            } else if (mod->implemented && ctx->models.list[i]->implemented) {
                LOGERR(LY_EINVAL, "Module \"%s\" in another revision already implemented.", ctx->models.list[i]->name);
                return EXIT_FAILURE;
            }
            /* else keep searching, for now the caller is just adding
             * another revision of an already present schema
             */
        } else if (!strcmp(ctx->models.list[i]->ns, mod->ns)) {
            LOGERR(LY_EINVAL, "Two different modules (\"%s\" and \"%s\") have the same namespace \"%s\".",
                   ctx->models.list[i]->name, mod->name, mod->ns);
            return EXIT_FAILURE;
        }
    }

    if (to_implement) {
        i = match_i;
        ctx->models.list[i]->implemented = 1;
        goto already_in_context;
    }
    ctx->models.list[i] = mod;
    ctx->models.used++;
    ctx->models.module_set_id++;
    return EXIT_SUCCESS;

already_in_context:
    lys_sub_module_remove_devs_augs(mod);
    lys_free(mod, NULL, 1);
    (*module) = ctx->models.list[i];
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
