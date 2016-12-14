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
#include <inttypes.h>
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
lyp_is_rpc_action(struct lys_node *node)
{
    assert(node);

    while (lys_parent(node)) {
        node = lys_parent(node);
        if (node->nodetype == LYS_ACTION) {
            break;
        }
    }

    if (node->nodetype & (LYS_RPC | LYS_ACTION)) {
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
lys_read_import(struct ly_ctx *ctx, int fd, LYS_INFORMAT format, const char *revision, int implement)
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
        module = yin_read_module(ctx, addr, revision, implement);
        break;
    case LYS_IN_YANG:
        module = yang_read_module(ctx, addr, sb.st_size + 2, revision, implement);
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
                int implement, struct unres_schema *unres)
{
    size_t len, flen, match_len = 0, dir_len;
    int fd, i;
    char *wd, *wn = NULL;
    DIR *dir = NULL;
    struct dirent *file;
    char *match_name = NULL, *dot, *rev, *filename;
    LYS_INFORMAT format, match_format = 0;
    struct lys_module *result = NULL;
    unsigned int u;
    struct ly_set *dirs;
    struct stat st;

    /* start to fill the dir fifo with the context's search path (if set)
     * and the current working directory */
    dirs = ly_set_new();
    if (!dirs) {
        LOGMEM;
        return NULL;
    }

    len = strlen(name);
    wd = get_current_dir_name();
    if (!wd) {
        LOGMEM;
        goto cleanup;
    } else if (ly_set_add(dirs, wd, 0) == -1) {
        goto cleanup;
    }
    if (ctx->models.search_path) {
        wd = strdup(ctx->models.search_path);
        if (!wd) {
            LOGMEM;
            goto cleanup;
        } else if (ly_set_add(dirs, wd, 0) == -1) {
            goto cleanup;
        }
    }
    wd = NULL;

    /* start searching */
    while (dirs->number) {
        free(wd);
        free(wn); wn = NULL;

        dirs->number--;
        wd = (char *)dirs->set.g[dirs->number];
        dirs->set.g[dirs->number] = NULL;
        LOGVRB("Searching for \"%s\" in %s.", name, wd);

        if (dir) {
            closedir(dir);
        }
        dir = opendir(wd);
        dir_len = strlen(wd);
        if (!dir) {
            LOGWRN("Unable to open directory \"%s\" for searching (sub)modules (%s).", wd, strerror(errno));
        } else {
            while ((file = readdir(dir))) {
                if (!strcmp(".", file->d_name) || !strcmp("..", file->d_name)) {
                    /* skip . and .. */
                    continue;
                }
                free(wn);
                if (asprintf(&wn, "%s/%s", wd, file->d_name) == -1) {
                    LOGMEM;
                    goto cleanup;
                }
                if (stat(wn, &st) == -1) {
                    LOGWRN("Unable to get information about \"%s\" file in \"%s\" when searching for (sub)modules (%s)",
                           file->d_name, wd, strerror(errno));
                    continue;
                }
                if (S_ISDIR(st.st_mode) && dirs->number) {
                    /* we have another subdirectory in searchpath to explore,
                     * subdirectories are not taken into account in current working dir (dirs->set.g[0]) */
                    if (ly_set_add(dirs, wn, 0) == -1) {
                        goto cleanup;
                    }
                    /* continue with the next item in current directory */
                    wn = NULL;
                    continue;
                } else if (!S_ISREG(st.st_mode)) {
                    /* not a regular file (note that we see the target of symlinks instead of symlinks */
                    continue;
                }

                /* here we know that the item is a file which can contain a module */
                if (strncmp(name, file->d_name, len) ||
                        (file->d_name[len] != '.' && file->d_name[len] != '@')) {
                    /* different filename than the module we search for */
                    continue;
                }

                /* get type according to filename suffix */
                flen = strlen(file->d_name);
                if (!strcmp(&file->d_name[flen - 4], ".yin")) {
                    format = LYS_IN_YIN;
                } else if (!strcmp(&file->d_name[flen - 5], ".yang")) {
                    format = LYS_IN_YANG;
                } else {
                    /* not supportde suffix/file format */
                    continue;
                }

                if (revision) {
                    /* we look for the specific revision, try to get it from the filename */
                    if (file->d_name[len] == '@') {
                        /* check revision from the filename */
                        if (strncmp(revision, &file->d_name[len + 1], strlen(revision))) {
                            /* another revision */
                            continue;
                        } else {
                            /* exact revision */
                            free(match_name);
                            match_name = wn;
                            wn = NULL;
                            match_len = dir_len + 1 + len;
                            match_format = format;
                            goto matched;
                        }
                    } else {
                        /* continue trying to find exact revision match, use this only if not found */
                        free(match_name);
                        match_name = wn;
                        wn = NULL;
                        match_len = dir_len + 1 +len;
                        match_format = format;
                        continue;
                    }
                } else {
                    /* remember the revision and try to find the newest one */
                    if (match_name) {
                        if (file->d_name[len] != '@' || lyp_check_date(&file->d_name[len + 1])) {
                            continue;
                        } else if (match_name[match_len] == '@' &&
                                (strncmp(&match_name[match_len + 1], &file->d_name[len + 1], LY_REV_SIZE - 1) >= 0)) {
                            continue;
                        }
                        free(match_name);
                    }

                    match_name = wn;
                    wn = NULL;
                    match_len = dir_len + 1 + len;
                    match_format = format;
                    continue;
                }
            }
        }
    }

    if (!match_name) {
        if (!module && !revision) {
            /* otherwise the module would be already taken from the context */
            result = (struct lys_module *)ly_ctx_get_module(ctx, name, revision);
        }
        if (!result) {
            LOGERR(LY_ESYS, "Data model \"%s\" not found.", name, ctx->models.search_path, wd);
        }
        goto cleanup;
    }

matched:
    LOGVRB("Loading schema from \"%s\" file.", match_name);

    /* cut the format for now */
    dot = strrchr(match_name, '.');
    dot[1] = '\0';

    /* check that the same file was not already loaded - it make sense only in case of loading the newest revision,
     * search also in disabled module - if the matching module is disabled, it will be enabled instead of loading it */
    if (!revision) {
        for (i = 0; i < ctx->models.used; ++i) {
            if (ctx->models.list[i]->filepath && !strcmp(name, ctx->models.list[i]->name)
                    && !strncmp(match_name, ctx->models.list[i]->filepath, strlen(match_name))) {
                result = ctx->models.list[i];
                if (implement && !result->implemented) {
                    /* make it implemented now */
                    if (lys_set_implemented(result)) {
                        result = NULL;
                    }
                }
                if (result->disabled) {
                    lys_set_enabled(result);
                }

                goto cleanup;
            }
        }
    }

    /* add the format back */
    dot[1] = 'y';

    /* open the file */
    fd = open(match_name, O_RDONLY);
    if (fd < 0) {
        LOGERR(LY_ESYS, "Unable to open data model file \"%s\" (%s).",
               match_name, strerror(errno));
        goto cleanup;
    }

    if (module) {
        result = (struct lys_module *)lys_submodule_read(module, fd, match_format, unres);
    } else {
        result = lys_read_import(ctx, fd, match_format, revision, implement);
    }
    close(fd);

    if (!result) {
        goto cleanup;
    }

    /* check that name and revision match filename */
    filename = strrchr(match_name, '/');
    if (!filename) {
        filename = match_name;
    } else {
        filename++;
    }
    rev = strchr(filename, '@');
    /* name */
    len = strlen(result->name);
    if (strncmp(filename, result->name, len) ||
            ((rev && rev != &filename[len]) || (!rev && dot != &filename[len]))) {
        LOGWRN("File name \"%s\" does not match module name \"%s\".", filename, result->name);
    }
    if (rev) {
        len = dot - ++rev;
        if (!result->rev_size || len != 10 || strncmp(result->rev[0].date, rev, len)) {
            LOGWRN("File name \"%s\" does not match module revision \"%s\".", filename,
                   result->rev_size ? result->rev[0].date : "none");
        }
    }

    result->filepath = lydict_insert_zc(ctx, match_name);
    match_name = NULL;
    /* success */

cleanup:
    free(wn);
    free(wd);
    if (dir) {
        closedir(dir);
    }
    free(match_name);
    for (u = 0; u < dirs->number; u++) {
        free(dirs->set.g[u]);
    }
    ly_set_free(dirs);

    return result;
}

/* logs directly
 * base: 0  - to accept decimal, octal, hexadecimal (in default value)
 *       10 - to accept only decimal (instance value)
 */
static int
parse_int(const char *val_str, int64_t min, int64_t max, int base, int64_t *ret, struct lyd_node *node)
{
    char *strptr;

    if (!val_str || !val_str[0]) {
        if (node) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, "", node->schema->name);
        } else {
            ly_errno = LY_EVALID;
            ly_vecode = LYVE_INVAL;
        }
        return EXIT_FAILURE;
    }

    /* convert to 64-bit integer, all the redundant characters are handled */
    errno = 0;
    strptr = NULL;

    /* parse the value */
    *ret = strtoll(val_str, &strptr, base);
    if (errno || (*ret < min) || (*ret > max)) {
        if (node) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, val_str, node->schema->name);
        } else {
            ly_errno = LY_EVALID;
            ly_vecode = LYVE_INVAL;
        }
        return EXIT_FAILURE;
    } else if (strptr && *strptr) {
        while (isspace(*strptr)) {
            ++strptr;
        }
        if (*strptr) {
            if (node) {
                LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, val_str, node->schema->name);
            } else {
                ly_errno = LY_EVALID;
                ly_vecode = LYVE_INVAL;
            }
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/* logs directly
 * base: 0  - to accept decimal, octal, hexadecimal (in default value)
 *       10 - to accept only decimal (instance value)
 */
static int
parse_uint(const char *val_str, uint64_t max, int base, uint64_t *ret, struct lyd_node *node)
{
    char *strptr;

    if (!val_str || !val_str[0]) {
        if (node) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, "", node->schema->name);
        } else {
            ly_errno = LY_EVALID;
            ly_vecode = LYVE_INVAL;
        }
        return EXIT_FAILURE;
    }

    errno = 0;
    strptr = NULL;
    *ret = strtoull(val_str, &strptr, base);
    if (errno || (*ret > max)) {
        if (node) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, val_str, node->schema->name);
        } else {
            ly_errno = LY_EVALID;
            ly_vecode = LYVE_INVAL;
        }
        return EXIT_FAILURE;
    } else if (strptr && *strptr) {
        while (isspace(*strptr)) {
            ++strptr;
        }
        if (*strptr) {
            if (node) {
                LOGVAL(LYE_INVAL, LY_VLOG_LYD, node, val_str, node->schema->name);
            } else {
                ly_errno = LY_EVALID;
                ly_vecode = LYVE_INVAL;
            }
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
validate_length_range(uint8_t kind, uint64_t unum, int64_t snum, int64_t fnum, uint8_t fnum_dig, struct lys_type *type,
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
                    || ((kind == 2) && (dec64cmp(fnum, fnum_dig, tmp_intv->value.fval.min, cur_type->info.dec64.dig) < 0))) {
                break;
            }

            if (((kind == 0) && (unum >= tmp_intv->value.uval.min) && (unum <= tmp_intv->value.uval.max))
                    || ((kind == 1) && (snum >= tmp_intv->value.sval.min) && (snum <= tmp_intv->value.sval.max))
                    || ((kind == 2) && (dec64cmp(fnum, fnum_dig, tmp_intv->value.fval.min, cur_type->info.dec64.dig) > -1)
                    && (dec64cmp(fnum, fnum_dig, tmp_intv->value.fval.max, cur_type->info.dec64.dig) < 1))) {
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

        LOGVAL(LYE_NOCONSTR, LY_VLOG_LYD, node, (val_str ? val_str : ""), restr ? restr->expr : "");
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
    int i, rc;
    pcre *precomp;

    assert(type->base == LY_TYPE_STRING);

    if (!val_str) {
        val_str = "";
    }

    if (type->der && validate_pattern(val_str, &type->der->type, node)) {
        return EXIT_FAILURE;
    }

    for (i = 0; i < type->info.str.pat_count; ++i) {
        if (lyp_check_pattern(&type->info.str.patterns[i].expr[1], &precomp)) {
            LOGINT;
            return EXIT_FAILURE;
        }

        rc = pcre_exec(precomp, NULL, val_str, strlen(val_str), 0, 0, NULL, 0);
        if ((rc && type->info.str.patterns[i].expr[0] == 0x06) || (!rc && type->info.str.patterns[i].expr[0] == 0x15)) {
            LOGVAL(LYE_NOCONSTR, LY_VLOG_LYD, node, val_str, &type->info.str.patterns[i].expr[1]);
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

static void
check_number(const char *str_num, const char **num_end, LY_DATA_TYPE base)
{
    if (!isdigit(str_num[0]) && (str_num[0] != '-') && (str_num[0] != '+')) {
        *num_end = str_num;
        return;
    }

    if ((str_num[0] == '-') || (str_num[0] == '+')) {
        ++str_num;
    }

    while (isdigit(str_num[0])) {
        ++str_num;
    }

    if ((base != LY_TYPE_DEC64) || (str_num[0] != '.') || !isdigit(str_num[1])) {
        *num_end = str_num;
        return;
    }

    ++str_num;
    while (isdigit(str_num[0])) {
        ++str_num;
    }

    *num_end = str_num;
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
    const char *c = expr, *tail;
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

            check_number(c, &tail, type->base);
            if (c == tail) {
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
        check_number(c, &tail, type->base);
        if (c == tail) {
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
            if (!strncmp(perl_regex + start + 5, lyp_ublock2urange[idx][0], strlen(lyp_ublock2urange[idx][0]))) {
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

/**
 * @brief Change the value into its canonical form. In libyang, additionally to the RFC,
 * all identities have their module as a prefix in their canonical form.
 *
 * @param[in] ctx
 * @param[in] type Type of the value.
 * @param[in,out] value Original and then canonical value.
 * @param[in] data1 If \p type is #LY_TYPE_BITS: (struct lys_type_bit **) type bit field,
 *                                #LY_TYPE_DEC64: (int64_t *) parsed digits of the number itself without floating point,
 *                                #LY_TYPE_IDENT: (const char *) local module name (identityref node module),
 *                                #LY_TYPE_INT*: (int64_t *) parsed int number itself,
 *                                #LY_TYPE_UINT*: (uint64_t *) parsed uint number itself,
 *                                otherwise ignored.
 * @param[in] data2 If \p type is #LY_TYPE_BITS: (int *) type bit field length,
 *                                #LY_TYPE_DEC64: (uint8_t *) number of fraction digits (position of the floating point),
 *                                otherwise ignored.
 * @return 1 if a conversion took place, 0 if the value was kept the same.
 */
static int
make_canonical(struct ly_ctx *ctx, int type, const char **value, void *data1, void *data2)
{
    char *buf = ly_buf(), *buf_backup = NULL, *str;
    struct lys_type_bit **bits = NULL;
    const char *module_name;
    int i, j, count, ret = 0;
    int64_t num;
    uint64_t unum;
    uint8_t c;

    /* prepare buffer for creating canonical representation */
    if (ly_buf_used && buf[0]) {
        buf_backup = strndup(buf, LY_BUF_SIZE - 1);
    }
    ly_buf_used++;

    switch (type) {
    case LY_TYPE_BITS:
        bits = (struct lys_type_bit **)data1;
        count = *((int *)data2);
        /* in canonical form, the bits are ordered by their position */
        buf[0] = '\0';
        for (i = 0; i < count; i++) {
            if (!bits[i]) {
                /* bit not set */
                continue;
            }
            if (buf[0]) {
                str = strdup(buf);
                sprintf(buf, "%s %s", str, bits[i]->name);
                free(str);
            } else {
                sprintf(buf, "%s", bits[i]->name);
            }
        }
        break;

    case LY_TYPE_IDENT:
        module_name = (const char *)data1;
        /* identity must always have a prefix */
        if (!strchr(*value, ':')) {
            sprintf(buf, "%s:%s", module_name, *value);
        } else {
            strcpy(buf, *value);
        }
        break;

    case LY_TYPE_DEC64:
        num = *((int64_t *)data1);
        c = *((uint8_t *)data2);
        if (num) {
            count = sprintf(buf, "%"PRId64" ", num);
            if ((count - 1) <= c) {
                /* we have 0. value, add a space for the leading zero */
                count = sprintf(buf, "0%"PRId64" ", num);
            }
            for (i = c, j = 1; i > 0 ; i--) {
                if (j && i > 1 && buf[count - 2] == '0') {
                    /* we have trailing zero to skip */
                    buf[count - 1] = '\0';
                } else {
                    j = 0;
                    buf[count - 1] = buf[count - 2];
                }
                count--;
            }
            buf[count - 1] = '.';
        } else {
            /* zero */
            sprintf(buf, "0.0");
        }
        break;

    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
    case LY_TYPE_INT64:
        num = *((int64_t *)data1);
        sprintf(buf, "%"PRId64, num);
        break;

    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_UINT64:
        unum = *((uint64_t *)data1);
        sprintf(buf, "%"PRIu64, unum);
        break;

    default:
        /* should not be even called - just do nothing */
        goto cleanup;
    }

    if (strcmp(buf, *value)) {
        lydict_remove(ctx, *value);
        *value = lydict_insert(ctx, buf, 0);
        ret = 1;
    }

cleanup:
    if (buf_backup) {
        /* return previous internal buffer content */
        strcpy(buf, buf_backup);
        free(buf_backup);
    }
    ly_buf_used--;

    return ret;
}


/*
 * xml  - optional for converting instance-identifier and identityref into JSON format
 * tree - optional for resolving instance-identifiers and leafrefs
 * leaf - mandatory to know the context (necessary e.g. for prefixes in idenitytref values)
 * store - flag for storing parsed data
 */
struct lys_type *
lyp_parse_value(struct lys_type *type, const char **value_, struct lyxml_elem *xml, struct lyd_node *tree,
                        struct lyd_node_leaf_list *leaf, int store, int resolvable, int dflt)
{
    struct lys_type *ret = NULL, *t;
    int c, i, j, len, found = 0, hidden;
    int64_t num;
    uint64_t unum;
    const char *ptr, *ptr2, *value = *value_;
    struct lys_type_bit **bits = NULL;
    struct lys_ident *ident;

    assert(leaf);

    if (store) {
        leaf->value_type = type->base;
    }

    switch(type->base) {
    case LY_TYPE_BINARY:
        /* get number of octets for length validation */
        unum = 0;
        if (value) {
            ptr = value;
            ptr2 = strchr(value, '\n');
            while (ptr2) {
                unum += ptr2 - ptr;
                ptr = ptr2 + 1;
                ptr2 = strchr(ptr, '\n');
            }
            unum += strlen(ptr);
        }

        if (unum & 3) {
            /* base64 length must be multiple of 4 chars */
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, value, leaf->schema->name);
            LOGVAL(LYE_SPEC, LY_VLOG_LYD, leaf, "Base64 encoded value length must be divisible by 4.");
            goto cleanup;
        }
        len = (unum / 4) * 3;
        /* check padding */
        if (unum) {
            if (ptr[strlen(ptr) - 1] == '=') {
                len--;
            }
            if (ptr[strlen(ptr) - 2] == '=') {
                len--;
            }
        }
        if (validate_length_range(0, len, 0, 0, 0, type, value, (struct lyd_node *)leaf)) {
            goto cleanup;
        }

        if (store) {
            /* store the result */
            leaf->value.binary = value;
        }
        break;

    case LY_TYPE_BITS:
        /* locate bits structure with the bits definitions
         * since YANG 1.1 allows restricted bits, it is the first
         * bits type with some explicit bit specification */
        for (; !type->info.bits.count; type = &type->der->type);

        if (value || store) {
            /* allocate the array of pointers to bits definition */
            bits = calloc(type->info.bits.count, sizeof *bits);
            if (!bits) {
                LOGMEM;
                goto cleanup;
            }
        }

        if (!value) {
            /* no bits set */
            if (store) {
                /* store empty array */
                leaf->value.bit = bits;
            }
            break;
        }

        c = 0;
        i = 0;
        while (value[c]) {
            /* skip leading whitespaces */
            while (isspace(value[c])) {
                c++;
            }

            /* get the length of the bit identifier */
            for (len = 0; value[c] && !isspace(value[c]); c++, len++);

            /* go back to the beginning of the identifier */
            c = c - len;

            /* find bit definition, identifiers appear ordered by their posititon */
            for (found = i = 0; i < type->info.bits.count; i++) {
                if (!strncmp(type->info.bits.bit[i].name, &value[c], len) && !type->info.bits.bit[i].name[len]) {
                    /* we have match, check if the value is enabled ... */
                    for (j = 0; j < type->info.bits.bit[i].iffeature_size; j++) {
                        if (!resolve_iffeature(&type->info.bits.bit[i].iffeature[i])) {
                            LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, value, leaf->schema->name);
                            LOGVAL(LYE_SPEC, LY_VLOG_LYD, leaf,
                                   "Bit \"%s\" is disabled by its if-feature condition.", type->info.bits.bit[i].name);

                            free(bits);
                            goto cleanup;
                        }
                    }
                    /* check that the value was not already set */
                    if (bits[i]) {
                        LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, value, leaf->schema->name);
                        LOGVAL(LYE_SPEC, LY_VLOG_LYD, leaf, "Bit \"%s\" used multiple times.",
                               type->info.bits.bit[i].name);
                        free(bits);
                        goto cleanup;
                    }
                    /* ... and then store the pointer */
                    bits[i] = &type->info.bits.bit[i];

                    /* stop searching */
                    found = 1;
                    break;
                }
            }

            if (!found) {
                /* referenced bit value does not exists */
                LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, value, leaf->schema->name);
                free(bits);
                goto cleanup;
            }
            c = c + len;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_BITS, value_, bits, &type->info.bits.count);

        if (store) {
            /* store the result */
            leaf->value.bit = bits;
        } else {
            free(bits);
        }
        break;

    case LY_TYPE_BOOL:
        if (value && !strcmp(value, "true")) {
            if (store) {
                leaf->value.bln = 1;
            }
        } else if (!value || strcmp(value, "false")) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, value ? value : "", leaf->schema->name);
            goto cleanup;
        }
        /* else stays 0 */
        break;

    case LY_TYPE_DEC64:
        if (!value || !value[0]) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, "", leaf->schema->name);
            goto cleanup;
        }

        ptr = value;
        if (parse_range_dec64(&ptr, type->info.dec64.dig, &num) || ptr[0]) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, value, leaf->schema->name);
            goto cleanup;
        }

        if (validate_length_range(2, 0, 0, num, type->info.dec64.dig, type, value, (struct lyd_node *)leaf)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_DEC64, value_, &num, &type->info.dec64.dig);

        if (store) {
            /* store the result */
            leaf->value.dec64 = num;
        }
        break;

    case LY_TYPE_EMPTY:
        if (value && value[0]) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, value, leaf->schema->name);
            goto cleanup;
        }
        break;

    case LY_TYPE_ENUM:
        /* locate enums structure with the enumeration definitions,
         * since YANG 1.1 allows restricted enums, it is the first
         * enum type with some explicit enum specification */
        for (; !type->info.enums.count; type = &type->der->type);

        /* find matching enumeration value */
        for (i = found = 0; i < type->info.enums.count; i++) {
            if (value && !strcmp(value, type->info.enums.enm[i].name)) {
                /* we have match, check if the value is enabled ... */
                for (j = 0; j < type->info.enums.enm[i].iffeature_size; j++) {
                    if (!resolve_iffeature(&type->info.enums.enm[i].iffeature[i])) {
                        LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, value, leaf->schema->name);
                        LOGVAL(LYE_SPEC, LY_VLOG_LYD, leaf, "Enum \"%s\" is disabled by its if-feature condition.",
                               value);
                        goto cleanup;
                    }
                }
                /* ... and store pointer to the definition */
                if (store) {
                    leaf->value.enm = &type->info.enums.enm[i];
                }
                found = 1;
                break;
            }
        }

        if (!found) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, value ? value : "", leaf->schema->name);
            goto cleanup;
        }
        break;

    case LY_TYPE_IDENT:
        if (!value) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, "", leaf->schema->name);
            goto cleanup;
        }

        if (xml) {
            /* first, convert value into the json format */
            value = transform_xml2json(type->parent->module->ctx, value, xml, 0);
            if (!value) {
                /* invalid identityref format */
                LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, *value_, leaf->schema->name);
                goto cleanup;
            }
        } else if (dflt) {
            /* turn logging off */
            hidden = *ly_vlog_hide_location();
            ly_vlog_hide(1);

            /* the value actually uses module's prefixes instead of the module names as in JSON format,
             * we have to convert it */
            value = transform_schema2json(leaf->schema->module, value);
            if (!value) {
                /* invalid identityref format or it was already transformed, so ignore the error here */
                value = lydict_insert(type->parent->module->ctx, *value_, 0);
                /* erase error information */
                ly_err_clean(1);
            }
            /* turn logging back on */
            if (!hidden) {
                ly_vlog_hide(0);
            }
        } else {
            value = lydict_insert(type->parent->module->ctx, *value_, 0);
        }
        /* value is now in the dictionary, whether it differs from *value_ or not */

        /* the value is always changed and includes prefix */
        if (dflt) {
            type->parent->flags |= LYS_DFLTJSON;
        }

        ident = resolve_identref(type, value, (struct lyd_node *)leaf);
        if (!ident) {
            goto cleanup;
        } else if (store) {
            /* store the result */
            leaf->value.ident = ident;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_IDENT, &value,
                           (void *)lyd_node_module((struct lyd_node *)leaf)->name, NULL);

        /* replace the old value with the new one (even if they may be the same) */
        lydict_remove(type->parent->module->ctx, *value_);
        *value_ = value;
        break;

    case LY_TYPE_INST:
        if (!value) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, "", leaf->schema->name);
            goto cleanup;
        }

        if (xml) {
            /* first, convert value into the json format */
            value = transform_xml2json(type->parent->module->ctx, value, xml, 0);
            if (!value) {
                /* invalid instance-identifier format */
                LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, *value_, leaf->schema->name);
                goto cleanup;
            }
        } else if (dflt) {
            /* turn logging off */
            hidden = *ly_vlog_hide_location();
            ly_vlog_hide(1);

            /* the value actually uses module's prefixes instead of the module names as in JSON format,
             * we have to convert it */
            value = transform_schema2json(leaf->schema->module, value);
            if (!value) {
                /* invalid identityref format or it was already transformed, so ignore the error here */
                value = *value_;
                /* erase error information */
                ly_err_clean(1);
            } else if (value == *value_) {
                /* we have actually created the same expression (prefixes are the same as the module names)
                 * so we have just increased dictionary's refcount - fix it */
                lydict_remove(type->parent->module->ctx, value);
            }
            /* turn logging back on */
            if (!hidden) {
                ly_vlog_hide(0);
            }
        }
        if (resolvable && tree && !resolve_instid(tree, value) && (ly_errno || type->info.inst.req)) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, *value_, leaf->schema->name);
            goto cleanup;
        } else if (!resolvable && store) {
            /* make the note that the data node is not resolvable instance-identifier,
             * because based on the data type the target is not necessary the part of the tree */
            leaf->value_type |= LY_TYPE_INST_UNRES;
        }

        if (value != *value_) {
            /* update the changed value */
            lydict_remove(type->parent->module->ctx, *value_);
            *value_ = value;

            /* we have to remember the conversion into JSON format to be able to print it in correct form */
            if (dflt) {
                type->parent->flags |= LYS_DFLTJSON;
            }
        }
        break;

    case LY_TYPE_LEAFREF:
        if (!value) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, "", leaf->schema->name);
            goto cleanup;
        }

        /* it is called not only to get the final type, but mainly to update value to canonical or JSON form
         * if needed */
        t = lyp_parse_value(&type->info.lref.target->type, value_, xml, tree, leaf, 0, resolvable, dflt);
        value = *value_; /* refresh possibly changed value */
        if (!t) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, value, leaf->schema->name);
            goto cleanup;
        }

        if (!resolvable && store) {
            /* the leafref will not be resolved because of the data tree type which make possible that the
             * target is not present in the data tree. Therefore, instead of leafref type, we store into the
             * leaf the target type of the leafref with the note that it is unresolved leafref */
            leaf->value_type = t->base | LY_TYPE_LEAFREF_UNRES;
        } else if (store) {
            /* if the leaf is resolvable, its type is kept as LY_TYPE_LEAFREF */
            leaf->value_type = LY_TYPE_LEAFREF;

            /* erase possible error from ly_parse_value() calling */
            ly_err_clean(1);

            /* if we have the complete tree, resolve the leafref */
            if (tree && resolve_leafref(leaf, type) && type->info.lref.req != -1) {
                /* failure */
                goto cleanup;
            }
        }

        type = t;
        break;

    case LY_TYPE_STRING:
        if (validate_length_range(0, (value ? strlen(value) : 0), 0, 0, 0, type, value, (struct lyd_node *)leaf)) {
            goto cleanup;
        }

        if (validate_pattern(value, type, (struct lyd_node *)leaf)) {
            goto cleanup;
        }

        if (store) {
            /* store the result */
            leaf->value.string = value;
        }
        break;

    case LY_TYPE_INT8:
        if (parse_int(value, __INT64_C(-128), __INT64_C(127), dflt ? 0 : 10, &num, (struct lyd_node *)leaf)
                || validate_length_range(1, 0, num, 0, 0, type, value, (struct lyd_node *)leaf)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_INT8, value_, &num, NULL);

        if (store) {
            /* store the result */
            leaf->value.int8 = (int8_t)num;
        }
        break;

    case LY_TYPE_INT16:
        if (parse_int(value, __INT64_C(-32768), __INT64_C(32767), dflt ? 0 : 10, &num, (struct lyd_node *)leaf)
                || validate_length_range(1, 0, num, 0, 0, type, value, (struct lyd_node *)leaf)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_INT16, value_, &num, NULL);

        if (store) {
            /* store the result */
            leaf->value.int16 = (int16_t)num;
        }
        break;

    case LY_TYPE_INT32:
        if (parse_int(value, __INT64_C(-2147483648), __INT64_C(2147483647), dflt ? 0 : 10, &num, (struct lyd_node *)leaf)
                || validate_length_range(1, 0, num, 0, 0, type, value, (struct lyd_node *)leaf)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_INT32, value_, &num, NULL);

        if (store) {
            /* store the result */
            leaf->value.int32 = (int32_t)num;
        }
        break;

    case LY_TYPE_INT64:
        if (parse_int(value, __INT64_C(-9223372036854775807) - __INT64_C(1), __INT64_C(9223372036854775807),
                      dflt ? 0 : 10, &num, (struct lyd_node *)leaf)
                || validate_length_range(1, 0, num, 0, 0, type, value, (struct lyd_node *)leaf)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_INT64, value_, &num, NULL);

        if (store) {
            /* store the result */
            leaf->value.int64 = num;
        }
        break;

    case LY_TYPE_UINT8:
        if (parse_uint(value, __UINT64_C(255), dflt ? 0 : 10, &unum, (struct lyd_node *)leaf)
                || validate_length_range(0, unum, 0, 0, 0, type, value, (struct lyd_node *)leaf)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_UINT8, value_, &unum, NULL);

        if (store) {
            /* store the result */
            leaf->value.uint8 = (uint8_t)unum;
        }
        break;

    case LY_TYPE_UINT16:
        if (parse_uint(value, __UINT64_C(65535), dflt ? 0 : 10, &unum, (struct lyd_node *)leaf)
                || validate_length_range(0, unum, 0, 0, 0, type, value, (struct lyd_node *)leaf)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_UINT16, value_, &unum, NULL);

        if (store) {
            /* store the result */
            leaf->value.uint16 = (uint16_t)unum;
        }
        break;

    case LY_TYPE_UINT32:
        if (parse_uint(value, __UINT64_C(4294967295), dflt ? 0 : 10, &unum, (struct lyd_node *)leaf)
                || validate_length_range(0, unum, 0, 0, 0, type, value, (struct lyd_node *)leaf)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_UINT32, value_, &unum, NULL);

        if (store) {
            /* store the result */
            leaf->value.uint32 = (uint32_t)unum;
        }
        break;

    case LY_TYPE_UINT64:
        if (parse_uint(value, __UINT64_C(18446744073709551615), dflt ? 0 : 10, &unum, (struct lyd_node *)leaf)
                || validate_length_range(0, unum, 0, 0, 0, type, value, (struct lyd_node *)leaf)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_UINT64, value_, &unum, NULL);

        if (store) {
            /* store the result */
            leaf->value.uint64 = unum;
        }
        break;

    case LY_TYPE_UNION:
        t = NULL;
        found = 0;

        /* turn logging off, we are going to try to validate the value with all the types in order */
        hidden = *ly_vlog_hide_location();
        ly_vlog_hide(1);

        while ((t = lyp_get_next_union_type(type, t, &found))) {
            found = 0;
            ret = lyp_parse_value(t, value_, xml, tree, leaf, store, resolvable, dflt);
            if (ret) {
                /* we have the result */
                type = ret;
                break;
            }
            /* erase information about errors - they are false or irrelevant
             * and will be replaced by a single error messages */
            ly_err_clean(1);

            if (store) {
                /* erase possible present and invalid value data */
                if (t->base == LY_TYPE_BITS) {
                    free(leaf->value.bit);
                }
                memset(&leaf->value, 0, sizeof leaf->value);
            }
        }

        /* turn logging back on */
        if (!hidden) {
            ly_vlog_hide(0);
        }

        if (!t) {
            /* not found */
            if (store) {
                leaf->value_type &= ~LY_DATA_TYPE_MASK;
            }
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, leaf, *value_ ? *value_ : "", leaf->schema->name);
            goto cleanup;
        }
        break;

    default:
        LOGINT;
        return NULL;
    }

    ret = type;

cleanup:

    return ret;
}

/* does not log, cannot fail */
struct lys_type *
lyp_get_next_union_type(struct lys_type *type, struct lys_type *prev_type, int *found)
{
    int i;
    struct lys_type *ret = NULL;

    while (!type->info.uni.count) {
        assert(type->der); /* at least the direct union type has to have type specified */
        type = &type->der->type;
    }

    for (i = 0; i < type->info.uni.count; ++i) {
        if (type->info.uni.types[i].base == LY_TYPE_UNION) {
            ret = lyp_get_next_union_type(&type->info.uni.types[i], prev_type, found);
            if (ret) {
                break;
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

    return ret;
}


/* does not log */
static int
dup_identity_check(const char *id, struct lys_ident *ident, uint32_t size)
{
    uint32_t i;

    for (i = 0; i < size; i++) {
        if (ly_strequal(id, ident[i].name, 1)) {
            /* name collision */
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int
dup_identities_check(const char *id, struct lys_module *module)
{
    struct lys_module *mainmod;
    int i;

    if (dup_identity_check(id, module->ident, module->ident_size)) {
        LOGVAL(LYE_DUPID, LY_VLOG_NONE, NULL, "identity", id);
        return EXIT_FAILURE;
    }

    /* check identity in submodules */
    mainmod = lys_main_module(module);
    for (i = 0; i < mainmod->inc_size && mainmod->inc[i].submodule; ++i)
    if (dup_identity_check(id, mainmod->inc[i].submodule->ident, mainmod->inc[i].submodule->ident_size)) {
        LOGVAL(LYE_DUPID, LY_VLOG_NONE, NULL, "identity", id);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* does not log */
int
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
static int
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
    struct lys_module *mainmod;

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
        mainmod = lys_main_module(module);

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
        for (i = 0; i < mainmod->inc_size && mainmod->inc[i].submodule; i++) {
            if (dup_typedef_check(id, mainmod->inc[i].submodule->tpdf, mainmod->inc[i].submodule->tpdf_size)) {
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
        break;
    case LY_IDENT_FEATURE:
        assert(module);
        mainmod = lys_main_module(module);

        /* check feature name uniqness*/
        /* check features in the current module */
        if (dup_feature_check(id, module)) {
            LOGVAL(LYE_DUPID, LY_VLOG_NONE, NULL, "feature", id);
            return EXIT_FAILURE;
        }

        /* and all its submodules */
        for (i = 0; i < mainmod->inc_size && mainmod->inc[i].submodule; i++) {
            if (dup_feature_check(id, (struct lys_module *)mainmod->inc[i].submodule)) {
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

/**
 * @return
 * NULL - success
 * root - not yet resolvable
 * other node - mandatory node under the root
 */
static const struct lys_node *
lyp_check_mandatory_(const struct lys_node *root)
{
    int mand_flag = 0;
    const struct lys_node *iter = NULL;

    while ((iter = lys_getnext(iter, root, NULL, LYS_GETNEXT_WITHCHOICE | LYS_GETNEXT_WITHUSES | LYS_GETNEXT_INTOUSES | LYS_GETNEXT_INTONPCONT))) {
        if (iter->nodetype == LYS_USES) {
            if (!((struct lys_node_uses *)iter)->grp) {
                /* not yet resolved uses */
                return root;
            } else {
                /* go into uses */
                continue;
            }
        }
        if (iter->nodetype == LYS_CHOICE) {
            /* skip it, it was already checked for direct mandatory node in default */
            continue;
        }
        if (iter->nodetype == LYS_LIST) {
            if (((struct lys_node_list *)iter)->min) {
                mand_flag = 1;
            }
        } else if (iter->nodetype == LYS_LEAFLIST) {
            if (((struct lys_node_leaflist *)iter)->min) {
                mand_flag = 1;
            }
        } else if (iter->flags & LYS_MAND_TRUE) {
            mand_flag = 1;
        }

        if (mand_flag) {
            return iter;
        }
    }

    return NULL;
}

/* logs directly */
int
lyp_check_mandatory_augment(struct lys_node_augment *aug, const struct lys_node *target)
{
    const struct lys_node *node;

    if (aug->when || target->nodetype == LYS_CHOICE) {
        /* - mandatory nodes in new cases are ok;
         * clarification from YANG 1.1 - augmentation can add mandatory nodes when it is
         * conditional with a when statement */
        return EXIT_SUCCESS;
    }

    if ((node = lyp_check_mandatory_((struct lys_node *)aug))) {
        if (node != (struct lys_node *)aug) {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "mandatory");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                   "Mandatory node \"%s\" appears in augment of \"%s\" without when condition.",
                   node->name, aug->target_name);
            return -1;
        }
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief check that a mandatory node is not directly under the default case.
 * @param[in] node choice with default node
 * @return EXIT_SUCCESS if the constraint is fulfilled, EXIT_FAILURE otherwise
 */
int
lyp_check_mandatory_choice(struct lys_node *node)
{
    const struct lys_node *mand, *dflt = ((struct lys_node_choice *)node)->dflt;

    if ((mand = lyp_check_mandatory_(dflt))) {
        if (mand != dflt) {
            LOGVAL(LYE_INSTMT, LY_VLOG_NONE, NULL, "mandatory");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                   "Mandatory node \"%s\" is directly under the default case \"%s\" of the \"%s\" choice.",
                   mand->name, dflt->name, node->name);
            return -1;
        }
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Check status for invalid combination.
 *
 * @param[in] flags1 Flags of the referencing node.
 * @param[in] mod1 Module of the referencing node,
 * @param[in] name1 Schema node name of the referencing node.
 * @param[in] flags2 Flags of the referenced node.
 * @param[in] mod2 Module of the referenced node,
 * @param[in] name2 Schema node name of the referenced node.
 * @return EXIT_SUCCES on success, EXIT_FAILURE on invalid reference.
 */
int
lyp_check_status(uint16_t flags1, struct lys_module *mod1, const char *name1,
                 uint16_t flags2, struct lys_module *mod2, const char *name2,
                 const struct lys_node *node)
{
    uint16_t flg1, flg2;

    flg1 = (flags1 & LYS_STATUS_MASK) ? (flags1 & LYS_STATUS_MASK) : LYS_STATUS_CURR;
    flg2 = (flags2 & LYS_STATUS_MASK) ? (flags2 & LYS_STATUS_MASK) : LYS_STATUS_CURR;

    if ((flg1 < flg2) && (lys_main_module(mod1) == lys_main_module(mod2))) {
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
        inc->submodule = (struct lys_submodule *)ly_ctx_load_sub_module(module->ctx, module, value,
                                                                        inc->rev[0] ? inc->rev : NULL, 1, unres);
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

    /* load module - in specific situations it tries to get the module from the context */
    imp->module = (struct lys_module *)ly_ctx_load_sub_module(module->ctx, NULL, value, imp->rev[0] ? imp->rev : NULL, 0, NULL);

    /* update the list of currently being parsed modules */
    lyp_check_circmod_pop(module);

    /* check the result */
    if (!imp->module) {
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

/* Propagate includes into the main module */
int
lyp_propagate_submodule(struct lys_module *module, struct lys_include *inc)
{
    uint8_t i;
    size_t size;
    struct lys_include *aux_inc;

    /* propagate the included submodule into the main module */
    for (i = 0; (void*)module->inc[i].submodule != (void*)0x1; i++); /* get array size by searching for stop block */
    size = (i + 1) * sizeof *module->inc;
    aux_inc = realloc(module->inc, size + sizeof(void*));
    if (!aux_inc) {
        LOGMEM;
        return EXIT_FAILURE;
    }
    module->inc = aux_inc;
    memset(&module->inc[module->inc_size + 1], 0, (i - module->inc_size) * sizeof *module->inc);
    module->inc[i + 1].submodule = (void*)0x1; /* set stop block */

    memcpy(&module->inc[module->inc_size], inc, sizeof *module->inc);
    module->inc[module->inc_size].external = 1;
    module->inc_size++;

    return EXIT_SUCCESS;
}

int
lyp_ctx_add_module(struct lys_module **module)
{
    struct ly_ctx *ctx;
    struct lys_module **newlist = NULL;
    struct lys_module *mod;
    int i, match_i = -1, to_implement;
    int ret = EXIT_SUCCESS;

    assert(module);
    mod = (*module);
    to_implement = 0;
    ctx = mod->ctx;

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
        if (lys_set_implemented(ctx->models.list[i])) {
            ret = EXIT_FAILURE;
        }
        goto already_in_context;
    }

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
    ctx->models.list[ctx->models.used++] = mod;
    ctx->models.module_set_id++;
    return EXIT_SUCCESS;

already_in_context:
    lys_sub_module_remove_devs_augs(mod);
    lys_free(mod, NULL, 1);
    (*module) = ctx->models.list[i];
    return ret;
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
 * Includes checking for valid characters (following RFC 7950, sec 9.4)
 */
unsigned int
pututf8(char *dst, int32_t value)
{
    if (value < 0x80) {
        /* one byte character */
        if (value < 0x20 &&
                value != 0x09 &&
                value != 0x0a &&
                value != 0x0d) {
            goto error;
        }

        dst[0] = value;
        return 1;
    } else if (value < 0x800) {
        /* two bytes character */
        dst[0] = 0xc0 | (value >> 6);
        dst[1] = 0x80 | (value & 0x3f);
        return 2;
    } else if (value < 0xfffe) {
        /* three bytes character */
        if (((value & 0xf800) == 0xd800) ||
                (value >= 0xfdd0 && value <= 0xfdef)) {
            /* exclude surrogate blocks %xD800-DFFF */
            /* exclude noncharacters %xFDD0-FDEF */
            goto error;
        }

        dst[0] = 0xe0 | (value >> 12);
        dst[1] = 0x80 | ((value >> 6) & 0x3f);
        dst[2] = 0x80 | (value & 0x3f);

        return 3;
    } else if (value < 0x10fffe) {
        if ((value & 0xffe) == 0xffe) {
            /* exclude noncharacters %xFFFE-FFFF, %x1FFFE-1FFFF, %x2FFFE-2FFFF, %x3FFFE-3FFFF, %x4FFFE-4FFFF,
             * %x5FFFE-5FFFF, %x6FFFE-6FFFF, %x7FFFE-7FFFF, %x8FFFE-8FFFF, %x9FFFE-9FFFF, %xAFFFE-AFFFF,
             * %xBFFFE-BFFFF, %xCFFFE-CFFFF, %xDFFFE-DFFFF, %xEFFFE-EFFFF, %xFFFFE-FFFFF, %x10FFFE-10FFFF */
            goto error;
        }
        /* four bytes character */
        dst[0] = 0xf0 | (value >> 18);
        dst[1] = 0x80 | ((value >> 12) & 0x3f);
        dst[2] = 0x80 | ((value >> 6) & 0x3f);
        dst[3] = 0x80 | (value & 0x3f);

        return 4;
    }
error:
    /* out of range */
    LOGVAL(LYE_XML_INCHAR, LY_VLOG_NONE, NULL, NULL);
    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid UTF-8 value 0x%08x", value);
    return 0;
}

unsigned int
copyutf8(char *dst, const char *src)
{
    uint32_t value;

    /* unicode characters */
    if (!(src[0] & 0x80)) {
        /* one byte character */
        if (src[0] < 0x20 &&
                src[0] != 0x09 &&
                src[0] != 0x0a &&
                src[0] != 0x0d) {
            LOGVAL(LYE_XML_INCHAR, LY_VLOG_NONE, NULL, src);
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid UTF-8 value 0x%02x", src[0]);
            return 0;
        }

        dst[0] = src[0];
        return 1;
    } else if (!(src[0] & 0x20)) {
        /* two bytes character */
        dst[0] = src[0];
        dst[1] = src[1];
        return 2;
    } else if (!(src[0] & 0x10)) {
        /* three bytes character */
        value = ((uint32_t)(src[0] & 0xf) << 12) | ((uint32_t)(src[1] & 0x3f) << 6) | (src[2] & 0x3f);
        if (((value & 0xf800) == 0xd800) ||
                (value >= 0xfdd0 && value <= 0xfdef) ||
                (value & 0xffe) == 0xffe) {
            /* exclude surrogate blocks %xD800-DFFF */
            /* exclude noncharacters %xFDD0-FDEF */
            /* exclude noncharacters %xFFFE-FFFF */
            LOGVAL(LYE_XML_INCHAR, LY_VLOG_NONE, NULL, src);
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid UTF-8 value 0x%08x", value);
            return 0;
        }

        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        return 3;
    } else if (!(src[0] & 0x08)) {
        /* four bytes character */
        value = ((uint32_t)(src[0] & 0x7) << 18) | ((uint32_t)(src[1] & 0x3f) << 12) | ((uint32_t)(src[2] & 0x3f) << 6) | (src[3] & 0x3f);
        if ((value & 0xffe) == 0xffe) {
            /* exclude noncharacters %x1FFFE-1FFFF, %x2FFFE-2FFFF, %x3FFFE-3FFFF, %x4FFFE-4FFFF,
             * %x5FFFE-5FFFF, %x6FFFE-6FFFF, %x7FFFE-7FFFF, %x8FFFE-8FFFF, %x9FFFE-9FFFF, %xAFFFE-AFFFF,
             * %xBFFFE-BFFFF, %xCFFFE-CFFFF, %xDFFFE-DFFFF, %xEFFFE-EFFFF, %xFFFFE-FFFFF, %x10FFFE-10FFFF */
            LOGVAL(LYE_XML_INCHAR, LY_VLOG_NONE, NULL, src);
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid UTF-8 value 0x%08x", value);
            return 0;
        }
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
        return 4;
    } else {
        LOGVAL(LYE_XML_INCHAR, LY_VLOG_NONE, NULL, src);
        LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Invalid UTF-8 leading byte 0x%02x", src[0]);
        return 0;
    }
}
