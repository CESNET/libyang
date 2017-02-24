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

const char *ly_stmt_str[] = {
    [LY_STMT_UNKNOWN] = "",
    [LY_STMT_ARGUMENT] = "argument",
    [LY_STMT_BASE] = "base",
    [LY_STMT_BELONGSTO] = "belongs-to",
    [LY_STMT_CONTACT] = "contact",
    [LY_STMT_DEFAULT] = "default",
    [LY_STMT_DESCRIPTION] = "description",
    [LY_STMT_ERRTAG] = "error-app-tag",
    [LY_STMT_ERRMSG] = "error-message",
    [LY_STMT_KEY] = "key",
    [LY_STMT_NAMESPACE] = "namespace",
    [LY_STMT_ORGANIZATION] = "organization",
    [LY_STMT_PATH] = "path",
    [LY_STMT_PREFIX] = "prefix",
    [LY_STMT_PRESENCE] = "presence",
    [LY_STMT_REFERENCE] = "reference",
    [LY_STMT_REVISIONDATE] = "revision-date",
    [LY_STMT_UNITS] = "units",
    [LY_STMT_VALUE] = "value",
    [LY_STMT_VERSION] = "yang-version",
    [LY_STMT_MODIFIER] = "modifier",
    [LY_STMT_REQINSTANCE] = "require-instance",
    [LY_STMT_YINELEM] = "yin-element",
    [LY_STMT_CONFIG] = "config",
    [LY_STMT_MANDATORY] = "mandatory",
    [LY_STMT_ORDEREDBY] = "ordered-by",
    [LY_STMT_STATUS] = "status",
    [LY_STMT_DIGITS] = "fraction-digits",
    [LY_STMT_MAX] = "max-elements",
    [LY_STMT_MIN] = "min-elements",
    [LY_STMT_POSITION] = "position",
    [LY_STMT_UNIQUE] = "unique",
    [LY_STMT_MODULE] = "module",
    [LY_STMT_SUBMODULE] = "submodule",
    [LY_STMT_ACTION] = "action",
    [LY_STMT_ANYDATA] = "anydata",
    [LY_STMT_ANYXML] = "anyxml",
    [LY_STMT_CASE] = "case",
    [LY_STMT_CHOICE] = "choice",
    [LY_STMT_CONTAINER] = "container",
    [LY_STMT_GROUPING] = "grouping",
    [LY_STMT_INPUT] = "input",
    [LY_STMT_LEAF] = "leaf",
    [LY_STMT_LEAFLIST] = "leaf-list",
    [LY_STMT_LIST] = "list",
    [LY_STMT_NOTIFICATION] = "notification",
    [LY_STMT_OUTPUT] = "output",
    [LY_STMT_RPC] = "rpc",
    [LY_STMT_USES] = "uses",
    [LY_STMT_TYPEDEF] = "typedef",
    [LY_STMT_TYPE] = "type",
    [LY_STMT_BIT] = "bit",
    [LY_STMT_ENUM] = "enum",
    [LY_STMT_REFINE] = "refine",
    [LY_STMT_AUGMENT] = "augment",
    [LY_STMT_DEVIATE] = "deviate",
    [LY_STMT_DEVIATION] = "deviation",
    [LY_STMT_EXTENSION] = "extension",
    [LY_STMT_FEATURE] = "feature",
    [LY_STMT_IDENTITY] = "identity",
    [LY_STMT_IFFEATURE] = "if-feature",
    [LY_STMT_IMPORT] = "import",
    [LY_STMT_INCLUDE] = "include",
    [LY_STMT_LENGTH] = "length",
    [LY_STMT_MUST] = "must",
    [LY_STMT_PATTERN] = "pattern",
    [LY_STMT_RANGE] = "range",
    [LY_STMT_WHEN] = "when",
    [LY_STMT_REVISION] = "revision"
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

void *
lyp_mmap(int fd, size_t addsize, size_t *length)
{
    struct stat sb;
    long pagesize;
    size_t m;
    void *addr;

    assert(fd >= 0);
    ly_errno = LY_SUCCESS;

    if (fstat(fd, &sb) == -1) {
        LOGERR(LY_ESYS, "Failed to stat the file descriptor (%s) for the mmap().", strerror(errno));
        return MAP_FAILED;
    }
    if (!S_ISREG(sb.st_mode)) {
        LOGERR(LY_EINVAL, "File to mmap() is not a regular file");
        return MAP_FAILED;
    }
    if (!sb.st_size) {
        return NULL;
    }
    pagesize = sysconf(_SC_PAGESIZE);
    ++addsize;                       /* at least one additional byte for terminating NULL byte */

    m = sb.st_size % pagesize;
    if (m && pagesize - m >= addsize) {
        /* there will be enough space after the file content mapping to provide zeroed additional bytes */
        *length = sb.st_size + addsize;
        addr = mmap(NULL, *length, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    } else {
        /* there will not be enough bytes after the file content mapping for the additional bytes and some of them
         * would overflow into another page that would not be zerroed and any access into it would generate SIGBUS.
         * Therefore we have to do the following hack with double mapping. First, the required number of bytes
         * (including the additinal bytes) is required as anonymous and thus they will be really provided (actually more
         * because of using whole pages) and also initialized by zeros. Then, the file is mapped to the same address
         * where the anonymous mapping starts. */
        *length = sb.st_size + pagesize;
        addr = mmap(NULL, *length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        addr = mmap(addr, sb.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, fd, 0);
    }
    if (addr == MAP_FAILED) {
        LOGERR(LY_ESYS, "mmap() failed - %s", strerror(errno));
    }
    return addr;
}

int
lyp_munmap(void *addr, size_t length)
{
    return munmap(addr, length);
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
    size_t length;
    char *addr;

    if (!ctx || fd < 0) {
        LOGERR(LY_EINVAL, "%s: Invalid parameter.", __func__);
        return NULL;
    }

    addr = lyp_mmap(fd, 1, &length);
    if (addr == MAP_FAILED) {
        LOGERR(LY_ESYS, "Mapping file descriptor into memory failed (%s()).", __func__);
        return NULL;
    } else if (!addr) {
        LOGERR(LY_EINVAL, "Empty schema file.");
        return NULL;
    }

    switch (format) {
    case LYS_IN_YIN:
        module = yin_read_module(ctx, addr, revision, implement);
        break;
    case LYS_IN_YANG:
        module = yang_read_module(ctx, addr, length, revision, implement);
        break;
    default:
        LOGERR(LY_EINVAL, "%s: Invalid format parameter.", __func__);
        break;
    }
    lyp_munmap(addr, length);

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
                } else if (result->disabled) {
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
        result = (struct lys_module *)lys_sub_parse_fd(module, fd, match_format, unres);
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
            LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL, restr->emsg);
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
                LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL, type->info.str.patterns[i].emsg);
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

static const char *
ident_val_add_module_prefix(const char *value, const struct lyxml_elem *xml, struct ly_ctx *ctx)
{
    const struct lyxml_ns *ns;
    const struct lys_module *mod;
    char *str;

    do {
        LY_TREE_FOR((struct lyxml_ns *)xml->attr, ns) {
            if ((ns->type == LYXML_ATTR_NS) && !ns->prefix) {
                /* match */
                break;
            }
        }
        if (!ns) {
            xml = xml->parent;
        }
    } while (!ns && xml);

    if (!ns) {
        /* no default namespace */
        LOGINT;
        return NULL;
    }

    /* find module */
    mod = ly_ctx_get_module_by_ns(ctx, ns->value, NULL);
    if (!mod) {
        LOGINT;
        return NULL;
    }

    if (asprintf(&str, "%s:%s", mod->name, value) == -1) {
        LOGMEM;
        return NULL;
    }
    lydict_remove(ctx, value);

    return lydict_insert_zc(ctx, str);
}

/*
 * xml  - optional for converting instance-identifier and identityref into JSON format
 * leaf - mandatory to know the context (necessary e.g. for prefixes in idenitytref values)
 * attr - alternative to leaf in case of parsing value in annotations (attributes)
 * store - flag for union resolution - we do not want to store the result, we are just learning the type
 * dflt - whether the value is a default value from the schema
 */
struct lys_type *
lyp_parse_value(struct lys_type *type, const char **value_, struct lyxml_elem *xml,
                struct lyd_node_leaf_list *leaf, struct lyd_attr *attr,
                int store, int dflt)
{
    struct lys_type *ret = NULL, *t;
    int c, i, j, len, found = 0, hidden;
    int64_t num;
    uint64_t unum;
    const char *ptr, *ptr2, *value = *value_, *itemname;
    struct lys_type_bit **bits = NULL;
    struct lys_ident *ident;
    struct lys_module *mod;
    lyd_val *val;
    uint16_t *val_type;
    struct lyd_node *contextnode;

    assert(leaf || attr);

    if (leaf) {
        assert(!attr);
        mod = leaf->schema->module;
        val = &leaf->value;
        val_type = &leaf->value_type;
        contextnode = (struct lyd_node *)leaf;
        itemname = leaf->schema->name;
    } else {
        assert(!leaf);
        mod = attr->annotation->module;
        val = &attr->value;
        val_type = &attr->value_type;
        contextnode = attr->parent;
        itemname = attr->name;
    }

    if (store && ((*val_type & LY_DATA_TYPE_MASK) == LY_TYPE_BITS)) {
        free(val->bit);
        val->bit = NULL;
    }

    switch (type->base) {
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
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, value, itemname);
            LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL, "Base64 encoded value length must be divisible by 4.");
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
        if (validate_length_range(0, len, 0, 0, 0, type, value, contextnode)) {
            goto cleanup;
        }

        if (store) {
            /* store the result */
            val->binary = value;
            *val_type = LY_TYPE_BINARY;
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
                val->bit = bits;
                *val_type = LY_TYPE_BITS;
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
                            LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, value, itemname);
                            LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL,
                                   "Bit \"%s\" is disabled by its if-feature condition.", type->info.bits.bit[i].name);

                            free(bits);
                            goto cleanup;
                        }
                    }
                    /* check that the value was not already set */
                    if (bits[i]) {
                        LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, value, itemname);
                        LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL, "Bit \"%s\" used multiple times.",
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
                LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, value, itemname);
                free(bits);
                goto cleanup;
            }
            c = c + len;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_BITS, value_, bits, &type->info.bits.count);

        if (store) {
            /* store the result */
            val->bit = bits;
            *val_type = LY_TYPE_BITS;
        } else {
            free(bits);
        }
        break;

    case LY_TYPE_BOOL:
        if (value && !strcmp(value, "true")) {
            if (store) {
                val->bln = 1;
            }
        } else if (!value || strcmp(value, "false")) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, value ? value : "", itemname);
            goto cleanup;
        } else {
            if (store) {
                val->bln = 0;
            }
        }

        if (store) {
            *val_type = LY_TYPE_BOOL;
        }
        break;

    case LY_TYPE_DEC64:
        if (!value || !value[0]) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, "", itemname);
            goto cleanup;
        }

        ptr = value;
        if (parse_range_dec64(&ptr, type->info.dec64.dig, &num) || ptr[0]) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, value, itemname);
            goto cleanup;
        }

        if (validate_length_range(2, 0, 0, num, type->info.dec64.dig, type, value, contextnode)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_DEC64, value_, &num, &type->info.dec64.dig);

        if (store) {
            /* store the result */
            val->dec64 = num;
            *val_type = LY_TYPE_DEC64;
        }
        break;

    case LY_TYPE_EMPTY:
        if (value && value[0]) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, value, itemname);
            goto cleanup;
        }

        if (store) {
            *val_type = LY_TYPE_EMPTY;
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
                        LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, value, itemname);
                        LOGVAL(LYE_SPEC, LY_VLOG_PREV, NULL, "Enum \"%s\" is disabled by its if-feature condition.",
                               value);
                        goto cleanup;
                    }
                }
                /* ... and store pointer to the definition */
                if (store) {
                    val->enm = &type->info.enums.enm[i];
                    *val_type = LY_TYPE_ENUM;
                }
                found = 1;
                break;
            }
        }

        if (!found) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, value ? value : "", itemname);
            goto cleanup;
        }
        break;

    case LY_TYPE_IDENT:
        if (!value) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, "", itemname);
            goto cleanup;
        }

        if (xml) {
            /* first, convert value into the json format */
            value = transform_xml2json(type->parent->module->ctx, value, xml, 0, 0);
            if (!value) {
                /* invalid identityref format */
                LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, *value_, itemname);
                goto cleanup;
            }

            /* the value has no prefix (default namespace), but the element's namespace has a prefix, find default namespace */
            if (!strchr(value, ':') && xml->ns->prefix) {
                value = ident_val_add_module_prefix(value, xml, type->parent->module->ctx);
                if (!value) {
                    goto cleanup;
                }
            }
        } else if (dflt) {
            /* turn logging off */
            hidden = *ly_vlog_hide_location();
            ly_vlog_hide(1);

            /* the value actually uses module's prefixes instead of the module names as in JSON format,
             * we have to convert it */
            value = transform_schema2json(mod, value);
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

        ident = resolve_identref(type, value, contextnode, mod);
        if (!ident) {
            goto cleanup;
        } else if (store) {
            /* store the result */
            val->ident = ident;
            *val_type = LY_TYPE_IDENT;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_IDENT, &value,
                       (void*)lys_main_module(mod)->name, NULL);

        /* replace the old value with the new one (even if they may be the same) */
        lydict_remove(type->parent->module->ctx, *value_);
        *value_ = value;
        break;

    case LY_TYPE_INST:
        if (!value) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, "", itemname);
            goto cleanup;
        }

        if (xml) {
            /* first, convert value into the json format */
            value = transform_xml2json(type->parent->module->ctx, value, xml, 1, 0);
            if (!value) {
                /* invalid instance-identifier format */
                LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, *value_, itemname);
                goto cleanup;
            }
        } else if (dflt) {
            /* turn logging off */
            hidden = *ly_vlog_hide_location();
            ly_vlog_hide(1);

            /* the value actually uses module's prefixes instead of the module names as in JSON format,
             * we have to convert it */
            value = transform_schema2json(mod, value);
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

        if (store) {
            /* note that the data node is an unresolved instance-identifier */
            val->instance = NULL;
            *val_type = LY_TYPE_INST | LY_TYPE_INST_UNRES;
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
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, "", itemname);
            goto cleanup;
        }

        /* it is called not only to get the final type, but mainly to update value to canonical or JSON form
         * if needed */
        t = lyp_parse_value(&type->info.lref.target->type, value_, xml, leaf, attr, store, dflt);
        value = *value_; /* refresh possibly changed value */
        if (!t) {
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, value, itemname);
            goto cleanup;
        }

        if (store) {
            /* make the note that the data node is an unresolved leafref (value union was already filled) */
            *val_type |= LY_TYPE_LEAFREF_UNRES;
        }

        type = t;
        break;

    case LY_TYPE_STRING:
        if (validate_length_range(0, (value ? strlen(value) : 0), 0, 0, 0, type, value, contextnode)) {
            goto cleanup;
        }

        if (validate_pattern(value, type, contextnode)) {
            goto cleanup;
        }

        if (store) {
            /* store the result */
            val->string = value;
            *val_type = LY_TYPE_STRING;
        }
        break;

    case LY_TYPE_INT8:
        if (parse_int(value, __INT64_C(-128), __INT64_C(127), dflt ? 0 : 10, &num, contextnode)
                || validate_length_range(1, 0, num, 0, 0, type, value, contextnode)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_INT8, value_, &num, NULL);

        if (store) {
            /* store the result */
            val->int8 = (int8_t)num;
            *val_type = LY_TYPE_INT8;
        }
        break;

    case LY_TYPE_INT16:
        if (parse_int(value, __INT64_C(-32768), __INT64_C(32767), dflt ? 0 : 10, &num, contextnode)
                || validate_length_range(1, 0, num, 0, 0, type, value, contextnode)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_INT16, value_, &num, NULL);

        if (store) {
            /* store the result */
            val->int16 = (int16_t)num;
            *val_type = LY_TYPE_INT16;
        }
        break;

    case LY_TYPE_INT32:
        if (parse_int(value, __INT64_C(-2147483648), __INT64_C(2147483647), dflt ? 0 : 10, &num, contextnode)
                || validate_length_range(1, 0, num, 0, 0, type, value, contextnode)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_INT32, value_, &num, NULL);

        if (store) {
            /* store the result */
            val->int32 = (int32_t)num;
            *val_type = LY_TYPE_INT32;
        }
        break;

    case LY_TYPE_INT64:
        if (parse_int(value, __INT64_C(-9223372036854775807) - __INT64_C(1), __INT64_C(9223372036854775807),
                      dflt ? 0 : 10, &num, contextnode)
                || validate_length_range(1, 0, num, 0, 0, type, value, contextnode)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_INT64, value_, &num, NULL);

        if (store) {
            /* store the result */
            val->int64 = num;
            *val_type = LY_TYPE_INT64;
        }
        break;

    case LY_TYPE_UINT8:
        if (parse_uint(value, __UINT64_C(255), dflt ? 0 : 10, &unum, contextnode)
                || validate_length_range(0, unum, 0, 0, 0, type, value, contextnode)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_UINT8, value_, &unum, NULL);

        if (store) {
            /* store the result */
            val->uint8 = (uint8_t)unum;
            *val_type = LY_TYPE_UINT8;
        }
        break;

    case LY_TYPE_UINT16:
        if (parse_uint(value, __UINT64_C(65535), dflt ? 0 : 10, &unum, contextnode)
                || validate_length_range(0, unum, 0, 0, 0, type, value, contextnode)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_UINT16, value_, &unum, NULL);

        if (store) {
            /* store the result */
            val->uint16 = (uint16_t)unum;
            *val_type = LY_TYPE_UINT16;
        }
        break;

    case LY_TYPE_UINT32:
        if (parse_uint(value, __UINT64_C(4294967295), dflt ? 0 : 10, &unum, contextnode)
                || validate_length_range(0, unum, 0, 0, 0, type, value, contextnode)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_UINT32, value_, &unum, NULL);

        if (store) {
            /* store the result */
            val->uint32 = (uint32_t)unum;
            *val_type = LY_TYPE_UINT32;
        }
        break;

    case LY_TYPE_UINT64:
        if (parse_uint(value, __UINT64_C(18446744073709551615), dflt ? 0 : 10, &unum, contextnode)
                || validate_length_range(0, unum, 0, 0, 0, type, value, contextnode)) {
            goto cleanup;
        }

        make_canonical(type->parent->module->ctx, LY_TYPE_UINT64, value_, &unum, NULL);

        if (store) {
            /* store the result */
            val->uint64 = unum;
            *val_type = LY_TYPE_UINT64;
        }
        break;

    case LY_TYPE_UNION:
        if (store) {
            /* unresolved union type */
            memset(val, 0, sizeof(lyd_val));
            *val_type = LY_TYPE_UNION;
        }

        if (type->info.uni.has_ptr_type) {
            /* we are not resolving anything here, only parsing, and in this case we cannot decide
             * the type without resolving it -> we return the union type (resolve it with resolve_union()) */
            if (xml) {
                /* in case it should resolve into a instance-identifier, we can only do the JSON conversion here */
                val->string = transform_xml2json(type->parent->module->ctx, value, xml, 1, 0);
                if (!val->string) {
                    /* invalid instance-identifier format */
                    LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, *value_, itemname);
                    goto cleanup;
                }
            }
            break;
        }

        t = NULL;
        found = 0;

        /* turn logging off, we are going to try to validate the value with all the types in order */
        hidden = *ly_vlog_hide_location();
        ly_vlog_hide(1);

        while ((t = lyp_get_next_union_type(type, t, &found))) {
            found = 0;
            ret = lyp_parse_value(t, value_, xml, leaf, attr, store, dflt);
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
                    free(val->bit);
                }
                memset(val, 0, sizeof(lyd_val));
            }
        }

        /* turn logging back on */
        if (!hidden) {
            ly_vlog_hide(0);
        }

        if (!t) {
            /* not found */
            if (store) {
                *val_type &= ~LY_DATA_TYPE_MASK;
            }
            LOGVAL(LYE_INVAL, LY_VLOG_LYD, contextnode, *value_ ? *value_ : "", itemname);
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
    for (i = 0; i < mainmod->inc_size && mainmod->inc[i].submodule; ++i) {
        if (dup_identity_check(id, mainmod->inc[i].submodule->ident, mainmod->inc[i].submodule->ident_size)) {
            LOGVAL(LYE_DUPID, LY_VLOG_NONE, NULL, "identity", id);
            return EXIT_FAILURE;
        }
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
    int i, j;
    int size;
    struct lys_tpdf *tpdf;
    struct lys_node *node;
    struct lys_module *mainmod;
    struct lys_submodule *submod;

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

        /* check feature name uniqueness*/
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

    case LY_IDENT_EXTENSION:
        assert(module);
        mainmod = lys_main_module(module);

        /* check extension name uniqueness in the main module ... */
        for (i = 0; i < mainmod->extensions_size; i++) {
            if (ly_strequal(id, mainmod->extensions[i].name, 1)) {
                LOGVAL(LYE_DUPID, LY_VLOG_NONE, NULL, "extension", id);
                return EXIT_FAILURE;
            }
        }

        /* ... and all its submodules */
        for (j = 0; j < mainmod->inc_size && mainmod->inc[j].submodule; j++) {
            submod = mainmod->inc[j].submodule; /* shortcut */
            for (i = 0; i < submod->extensions_size; i++) {
                if (ly_strequal(id, submod->extensions[i].name, 1)) {
                    LOGVAL(LYE_DUPID, LY_VLOG_NONE, NULL, "extension", id);
                    return EXIT_FAILURE;
                }
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

void
lyp_del_includedup(struct lys_module *mod)
{
    struct ly_modules_list *models = &mod->ctx->models;
    uint8_t i;

    assert(mod && !mod->type);

    if (mod->inc_size && models->parsed_submodules_count) {
        for (i = models->parsed_submodules_count - 1; models->parsed_submodules[i]->type; --i);
        assert(models->parsed_submodules[i] == mod);

        models->parsed_submodules_count = i;
        if (!models->parsed_submodules_count) {
            free(models->parsed_submodules);
            models->parsed_submodules = NULL;
        }
    }
}

static void
lyp_add_includedup(struct lys_module *sub_mod, struct lys_submodule *parsed_submod)
{
    struct ly_modules_list *models = &sub_mod->ctx->models;
    int16_t i;

    /* store main module if first include */
    if (models->parsed_submodules_count) {
        for (i = models->parsed_submodules_count - 1; models->parsed_submodules[i]->type; --i);
    } else {
        i = -1;
    }
    if ((i == -1) || (models->parsed_submodules[i] != lys_main_module(sub_mod))) {
        ++models->parsed_submodules_count;
        models->parsed_submodules = ly_realloc(models->parsed_submodules,
                                               models->parsed_submodules_count * sizeof *models->parsed_submodules);
        if (!models->parsed_submodules) {
            LOGMEM;
            return;
        }
        models->parsed_submodules[models->parsed_submodules_count - 1] = lys_main_module(sub_mod);
    }

    /* store parsed submodule */
    ++models->parsed_submodules_count;
    models->parsed_submodules = ly_realloc(models->parsed_submodules,
                                           models->parsed_submodules_count * sizeof *models->parsed_submodules);
    if (!models->parsed_submodules) {
        LOGMEM;
        return;
    }
    models->parsed_submodules[models->parsed_submodules_count - 1] = (struct lys_module *)parsed_submod;
}

/*
 * types: 0 - include, 1 - import
 */
static int
lyp_check_add_circmod(struct lys_module *module, const char *value, int type)
{
    LY_ECODE code = type ? LYE_CIRC_IMPORTS : LYE_CIRC_INCLUDES;
    struct ly_modules_list *models = &module->ctx->models;
    uint8_t i;

    /* include/import itself */
    if (ly_strequal(module->name, value, 1)) {
        LOGVAL(code, LY_VLOG_NONE, NULL, value);
        return -1;
    }

    /* currently parsed modules */
    for (i = 0; i < models->parsing_sub_modules_count; i++) {
        if (ly_strequal(models->parsing_sub_modules[i], value, 1)) {
            LOGVAL(code, LY_VLOG_NONE, NULL, value);
            return -1;
        }
    }
    /* storing - enlarge the list of modules being currently parsed */
    ++models->parsing_sub_modules_count;
    models->parsing_sub_modules = ly_realloc(models->parsing_sub_modules,
                                             models->parsing_sub_modules_count * sizeof *models->parsing_sub_modules);
    if (!models->parsing_sub_modules) {
        LOGMEM;
        return -1;
    }
    models->parsing_sub_modules[models->parsing_sub_modules_count - 1] = value;

    return 0;
}

/*
 * -1 - error - invalid duplicities)
 *  0 - success, no duplicity
 *  1 - success, valid duplicity found and stored in *sub
 */
static int
lyp_check_includedup(struct lys_module *mod, const char *name, struct lys_include *inc, struct lys_submodule **sub)
{
    struct lys_module **parsed_sub = mod->ctx->models.parsed_submodules;
    uint8_t i, parsed_sub_count = mod->ctx->models.parsed_submodules_count;

    assert(sub);

    for (i = 0; i < mod->inc_size; ++i) {
        if (ly_strequal(mod->inc[i].submodule->name, name, 1)) {
            /* the same module is already included in the same module - error */
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, name, "include");
            LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Submodule \"%s\" included twice in the same module \"%s\".",
                   name, mod->name);
            return -1;
        }
    }

    if (parsed_sub_count) {
        for (i = parsed_sub_count - 1; parsed_sub[i]->type; --i) {
            if (ly_strequal(parsed_sub[i]->name, name, 1)) {
                /* check revisions, including multiple revisions of a single module is error */
                if (inc->rev[0] && (!parsed_sub[i]->rev_size || strcmp(parsed_sub[i]->rev[0].date, inc->rev))) {
                    /* the already included submodule has
                     * - no revision, but here we require some
                     * - different revision than the one required here */
                    LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, name, "include");
                    LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL, "Including multiple revisions of submodule \"%s\".", name);
                    return -1;
                }

                /* the same module is already included in some other submodule, return it */
                (*sub) = (struct lys_submodule *)parsed_sub[i];
                return 1;
            }
        }

        /* if we are submodule, the last module must be our main */
        assert(!mod->type || (parsed_sub[i] == ((struct lys_submodule *)mod)->belongsto));
    }

    /* no duplicity found */
    return 0;
}

/* returns:
 *  0 - inc successfully filled
 * -1 - error
 */
int
lyp_check_include(struct lys_module *module, const char *value, struct lys_include *inc, struct unres_schema *unres)
{
    int i;

    /* check that the submodule was not included yet */
    i = lyp_check_includedup(module, value, inc, &inc->submodule);
    if (i == -1) {
        return -1;
    } else if (i == 1) {
        return 0;
    }
    /* submodule is not yet loaded */

    /* circular include check */
    if (lyp_check_add_circmod(module, value, 0)) {
        return -1;
    }

    /* try to load the submodule */
    inc->submodule = (struct lys_submodule *)ly_ctx_load_sub_module(module->ctx, module, value,
                                                                    inc->rev[0] ? inc->rev : NULL, 1, unres);

    /* update the list of currently being parsed modules */
    --module->ctx->models.parsing_sub_modules_count;
    if (!module->ctx->models.parsing_sub_modules_count) {
        free(module->ctx->models.parsing_sub_modules);
        module->ctx->models.parsing_sub_modules = NULL;
    }

    /* check the result */
    if (!inc->submodule) {
        if (!ly_vecode) {
            LOGVAL(LYE_INARG, LY_VLOG_NONE, NULL, value, "include");
        }
        LOGERR(LY_EVALID, "Including \"%s\" module into \"%s\" failed.", value, module->name);
        return -1;
    }

    /* store the submodule as successfully parsed */
    lyp_add_includedup(module, inc->submodule);

    return 0;
}

static int
lyp_check_include_missing_recursive(struct lys_module *main_module, struct lys_submodule *sub)
{
    uint8_t i, j;
    void *reallocated;

    for (i = 0; i < sub->inc_size; i++) {
        /* check that the include is also present in the main module */
        for (j = 0; j < main_module->inc_size; j++) {
            if (main_module->inc[j].submodule == sub->inc[i].submodule) {
                break;
            }
        }

        if (j == main_module->inc_size) {
            /* match not found */
            if (main_module->version >= 2) {
                LOGVAL(LYE_MISSSTMT, LY_VLOG_NONE, NULL, "include");
                LOGVAL(LYE_SPEC, LY_VLOG_NONE, NULL,
                       "The main module \"%s\" misses include of the \"%s\" submodule used in another submodule \"%s\".",
                       main_module->name, sub->inc[i].submodule->name, sub->name);
                /* now we should return error, but due to the issues with freeing the module, we actually have
                 * to go through the all includes and, as in case of 1.0, add them into the main module and fail
                 * at the end when all the includes are in the main module and we can free them */
            } else {
                /* not strictly an error in YANG 1.0 */
                LOGWRN("The main module \"%s\" misses include of the \"%s\" submodule used in another submodule \"%s\".",
                       main_module->name, sub->inc[i].submodule->name, sub->name);
                LOGWRN("To avoid further issues, adding submodule \"%s\" into the main module \"%s\".",
                       sub->inc[i].submodule->name, main_module->name);
                /* but since it is a good practise and because we expect all the includes in the main module
                 * when searching it and also when freeing the module, put it into it */
            }
            main_module->inc_size++;
            reallocated = realloc(main_module->inc, main_module->inc_size * sizeof *main_module->inc);
            if (!reallocated) {
                LOGMEM;
                return EXIT_FAILURE;
            }
            main_module->inc = reallocated;
            memset(&main_module->inc[main_module->inc_size - 1], 0, sizeof *main_module->inc);
            /* to avoid unexpected consequences, copy just a link to the submodule and the revision,
             * all other substatements of the include are ignored */
            memcpy(&main_module->inc[main_module->inc_size - 1].rev, sub->inc[i].rev, LY_REV_SIZE - 1);
            main_module->inc[main_module->inc_size - 1].submodule = sub->inc[i].submodule;
        }

        /* recursion */
        lyp_check_include_missing_recursive(main_module, sub->inc[i].submodule);
    }

    return EXIT_SUCCESS;
}

int
lyp_check_include_missing(struct lys_module *main_module)
{
    uint8_t i;

    ly_err_clean(1);

    /* in YANG 1.1, all the submodules must be in the main module, check it even for
     * 1.0 where it will be printed as warning and the include will be added into the main module */

    for (i = 0; i < main_module->inc_size; i++) {
        lyp_check_include_missing_recursive(main_module, main_module->inc[i].submodule);
    }

    if (ly_errno) {
        /* see comment in lyp_check_include_missing_recursive() */
        return EXIT_FAILURE;
    } else {
        /* everything ok */
        return EXIT_SUCCESS;
    }
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
    if (lyp_check_add_circmod(module, value, 1)) {
        return -1;
    }

    /* load module - in specific situations it tries to get the module from the context */
    imp->module = (struct lys_module *)ly_ctx_load_sub_module(module->ctx, NULL, value, imp->rev[0] ? imp->rev : NULL, 0, NULL);

    /* update the list of currently being parsed modules */
    --module->ctx->models.parsing_sub_modules_count;
    if (!module->ctx->models.parsing_sub_modules_count) {
        free(module->ctx->models.parsing_sub_modules);
        module->ctx->models.parsing_sub_modules = NULL;
    }

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

/*
 * put the newest revision to the first position
 */
void
lyp_sort_revisions(struct lys_module *module)
{
    uint8_t i, r;
    struct lys_revision rev;

    for (i = 1, r = 0; i < module->rev_size; i++) {
        if (strcmp(module->rev[i].date, module->rev[r].date) > 0) {
            r = i;
        }
    }

    if (r) {
        /* the newest revision is not on position 0, switch them */
        memcpy(&rev, &module->rev[0], sizeof rev);
        memcpy(&module->rev[0], &module->rev[r], sizeof rev);
        memcpy(&module->rev[r], &rev, sizeof rev);
    }
}

void
lyp_ext_instance_rm(struct ly_ctx *ctx, struct lys_ext_instance ***ext, uint8_t *size, uint8_t index)
{
    uint8_t i;

    lys_extension_instances_free(ctx, (*ext)[index]->ext, (*ext)[index]->ext_size);
    lydict_remove(ctx, (*ext)[index]->arg_value);
    free((*ext)[index]);

    /* move the rest of the array */
    for (i = index + 1; i < (*size); i++) {
        (*ext)[i - 1] = (*ext)[i];
    }
    /* clean the last cell in the array structure */
    (*ext)[(*size) - 1] = NULL;
    /* the array is not reallocated here, just change its size */
    (*size) = (*size) - 1;

    if (!(*size)) {
        /* ext array is empty */
        free((*ext));
        ext = NULL;
    }
}

static int
lyp_rfn_apply_ext_(struct lys_refine *rfn, struct lys_node *target, LYEXT_SUBSTMT substmt, struct lys_ext *extdef)
{
    struct ly_ctx *ctx;
    int m, n;
    struct lys_ext_instance *new;
    void *reallocated;

    ctx = target->module->ctx; /* shortcut */

    m = n = -1;
    while ((m = lys_ext_iter(rfn->ext, rfn->ext_size, m + 1, substmt)) != -1) {
        /* refine's substatement includes extensions, copy them to the target, replacing the previous
         * substatement's extensions if any. In case of refining the extension itself, we are going to
         * replace only the same extension (pointing to the same definition) */
        if (substmt == LYEXT_SUBSTMT_SELF && rfn->ext[m]->def != extdef) {
            continue;
        }

        /* get the index of the extension to replace in the target node */
        do {
            n = lys_ext_iter(target->ext, target->ext_size, n + 1, substmt);
        } while (n != -1 && substmt == LYEXT_SUBSTMT_SELF && target->ext[n]->def != extdef);

        /* TODO cover complex extension instances
           ((struct lys_ext_instance_complex*)(target->ext[n])->module = target->module;
         */
        if (n == -1) {
            /* nothing to replace, we are going to add it - reallocate */
            new = malloc(sizeof **target->ext);
            if (!new) {
                LOGMEM;
                return EXIT_FAILURE;
            }
            reallocated = realloc(target->ext, (target->ext_size + 1) * sizeof *target->ext);
            if (!reallocated) {
                LOGMEM;
                free(new);
                return EXIT_FAILURE;
            }
            target->ext = reallocated;
            target->ext_size++;

            /* init */
            n = target->ext_size - 1;
            target->ext[n] = new;
            target->ext[n]->parent = target;
            target->ext[n]->parent_type = LYEXT_PAR_NODE;
            target->ext[n]->flags = 0;
            target->ext[n]->insubstmt = substmt;
        } else {
            /* replacing - first remove the allocated data from target */
            lys_extension_instances_free(ctx, target->ext[n]->ext, target->ext[n]->ext_size);
            lydict_remove(ctx, target->ext[n]->arg_value);
        }
        /* common part for adding and replacing */
        target->ext[n]->def = rfn->ext[m]->def;
        /* parent and parent_type do not change */
        target->ext[n]->arg_value = lydict_insert(ctx, rfn->ext[m]->arg_value, 0);
        /* flags do not change */
        target->ext[n]->ext_size = rfn->ext[m]->ext_size;
        lys_ext_dup(target->module, rfn->ext[m]->ext, rfn->ext[m]->ext_size, target, LYEXT_PAR_NODE,
                    &target->ext[n]->ext, NULL);
        /* substmt does not change, but the index must be taken from the refine */
        target->ext[n]->insubstmt_index = rfn->ext[m]->insubstmt_index;
    }

    /* remove the rest of extensions belonging to the original substatement in the target node */
    while ((n = lys_ext_iter(target->ext, target->ext_size, n + 1, substmt)) != -1) {
        if (substmt == LYEXT_SUBSTMT_SELF && target->ext[n]->def != extdef) {
            /* keep this extension */
            continue;
        }

        /* remove the item */
        lyp_ext_instance_rm(ctx, &target->ext, &target->ext_size, n);
        --n;
    }

    return EXIT_SUCCESS;
}

/*
 * apply extension instances defined under refine's substatements.
 * It cannot be done immediately when applying the refine because there can be
 * still unresolved data (e.g. type) and mainly the targeted extension instances.
 */
int
lyp_rfn_apply_ext(struct lys_module *module)
{
    int i, k, a = 0;
    struct lys_node *root, *nextroot, *next, *node;
    struct lys_node *target;
    struct lys_node_uses *uses;
    struct lys_refine *rfn;
    struct ly_set *extset;

    /* refines in uses */
    LY_TREE_FOR_SAFE(module->data, nextroot, root) {
        /* go through the data tree of the module and all the defined augments */

        LY_TREE_DFS_BEGIN(root, next, node) {
            if (node->nodetype == LYS_USES) {
                uses = (struct lys_node_uses *)node;

                for (i = 0; i < uses->refine_size; i++) {
                    if (!uses->refine[i].ext_size) {
                        /* no extensions in refine */
                        continue;
                    }
                    rfn = &uses->refine[i]; /* shortcut */

                    /* get the target node */
                    resolve_descendant_schema_nodeid(rfn->target_name, uses->child,
                                                     LYS_NO_RPC_NOTIF_NODE | LYS_ACTION | LYS_NOTIF,
                                                     1, 0, (const struct lys_node **)&target);

                    /* extensions */
                    extset = ly_set_new();
                    k = -1;
                    while ((k = lys_ext_iter(rfn->ext, rfn->ext_size, k + 1, LYEXT_SUBSTMT_SELF)) != -1) {
                        ly_set_add(extset, rfn->ext[k]->def, 0);
                    }
                    for (k = 0; (unsigned int)k < extset->number; k++) {
                        if (lyp_rfn_apply_ext_(rfn, target, LYEXT_SUBSTMT_SELF, (struct lys_ext *)extset->set.g[k])) {
                            ly_set_free(extset);
                            return EXIT_FAILURE;
                        }
                    }
                    ly_set_free(extset);

                    /* description */
                    if (rfn->dsc && lyp_rfn_apply_ext_(rfn, target, LYEXT_SUBSTMT_DESCRIPTION, NULL)) {
                        return EXIT_FAILURE;
                    }
                    /* reference */
                    if (rfn->ref && lyp_rfn_apply_ext_(rfn, target, LYEXT_SUBSTMT_REFERENCE, NULL)) {
                        return EXIT_FAILURE;
                    }
                    /* config, in case of notification or rpc/action{notif, the config is not applicable
                     * (there is no config status) */
                    if ((rfn->flags & LYS_CONFIG_MASK) && (target->flags & LYS_CONFIG_MASK)) {
                        if (lyp_rfn_apply_ext_(rfn, target, LYEXT_SUBSTMT_CONFIG, NULL)) {
                            return EXIT_FAILURE;
                        }
                    }
                    /* default value */
                    if (rfn->dflt_size && lyp_rfn_apply_ext_(rfn, target, LYEXT_SUBSTMT_DEFAULT, NULL)) {
                        return EXIT_FAILURE;
                    }
                    /* mandatory */
                    if (rfn->flags & LYS_MAND_MASK) {
                        if (lyp_rfn_apply_ext_(rfn, target, LYEXT_SUBSTMT_MANDATORY, NULL)) {
                            return EXIT_FAILURE;
                        }
                    }
                    /* presence */
                    if ((target->nodetype & LYS_CONTAINER) && rfn->mod.presence) {
                        if (lyp_rfn_apply_ext_(rfn, target, LYEXT_SUBSTMT_PRESENCE, NULL)) {
                            return EXIT_FAILURE;
                        }
                    }
                    /* min/max */
                    if (rfn->flags & LYS_RFN_MINSET) {
                        if (lyp_rfn_apply_ext_(rfn, target, LYEXT_SUBSTMT_MIN, NULL)) {
                            return EXIT_FAILURE;
                        }
                    }
                    if (rfn->flags & LYS_RFN_MAXSET) {
                        if (lyp_rfn_apply_ext_(rfn, target, LYEXT_SUBSTMT_MAX, NULL)) {
                            return EXIT_FAILURE;
                        }
                    }
                    /* must and if-feature contain extensions on their own, not needed to be solved here */

                    if (target->ext_size) {
                        /* the allocated target's extension array can be now longer than needed in case
                         * there is less refine substatement's extensions than in original. Since we are
                         * going to reduce or keep the same memory, it is not necessary to test realloc's result */
                        target->ext = realloc(target->ext, target->ext_size * sizeof *target->ext);
                    }
                }
            }
            LY_TREE_DFS_END(root, next, node)
        }

        if (!nextroot && a < module->augment_size) {
            nextroot = module->augment[a].child;
            a++;
        }
    }

    return EXIT_SUCCESS;
}

/*
 * check mandatory substatements defined under extension instances.
 */
int
lyp_mand_check_ext(struct lys_ext_instance_complex *ext, const char *ext_name)
{
    void *p;
    int i;

    /* check for mandatory substatements */
    for (i = 0; ext->substmt[i].stmt; i++) {
        if (ext->substmt[i].cardinality == LY_STMT_CARD_OPT || ext->substmt[i].cardinality == LY_STMT_CARD_ANY) {
            /* not a mandatory */
            continue;
        } else if (ext->substmt[i].cardinality == LY_STMT_CARD_SOME) {
            goto array;
        }

        /*
         * LY_STMT_ORDEREDBY - not checked, has a default value which is the same as explicit system order
         * LY_STMT_MODIFIER, LY_STMT_STATUS, LY_STMT_MANDATORY, LY_STMT_CONFIG - checked, but mandatory requirement
         * does not make sense since there is also a default value specified
         */
        switch(ext->substmt[i].stmt) {
        case LY_STMT_ORDEREDBY:
            /* always ok */
            break;
        case LY_STMT_REQINSTANCE:
        case LY_STMT_DIGITS:
        case LY_STMT_MODIFIER:
            p = lys_ext_complex_get_substmt(ext->substmt[i].stmt, ext, NULL);
            if (!*(uint8_t*)p) {
                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, ly_stmt_str[ext->substmt[i].stmt], ext_name);
                goto error;
            }
            break;
        case LY_STMT_STATUS:
            p = lys_ext_complex_get_substmt(ext->substmt[i].stmt, ext, NULL);
            if (!(*(uint16_t*)p & LYS_STATUS_MASK)) {
                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, ly_stmt_str[ext->substmt[i].stmt], ext_name);
                goto error;
            }
            break;
        case LY_STMT_MANDATORY:
            p = lys_ext_complex_get_substmt(ext->substmt[i].stmt, ext, NULL);
            if (!(*(uint16_t*)p & LYS_MAND_MASK)) {
                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, ly_stmt_str[ext->substmt[i].stmt], ext_name);
                goto error;
            }
            break;
        case LY_STMT_CONFIG:
            p = lys_ext_complex_get_substmt(ext->substmt[i].stmt, ext, NULL);
            if (!(*(uint16_t*)p & LYS_CONFIG_MASK)) {
                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, ly_stmt_str[ext->substmt[i].stmt], ext_name);
                goto error;
            }
            break;
        default:
array:
            /* stored as a pointer */
            p = lys_ext_complex_get_substmt(ext->substmt[i].stmt, ext, NULL);
            if (!(*(void**)p)) {
                LOGVAL(LYE_MISSCHILDSTMT, LY_VLOG_NONE, NULL, ly_stmt_str[ext->substmt[i].stmt], ext_name);
                goto error;
            }
            break;
        }
    }

    return EXIT_SUCCESS;
error:
    return EXIT_FAILURE;
}

static int
lyp_deviate_del_ext(struct lys_node *target, struct lys_ext_instance *ext)
{
    int n = -1, found = 0;
    char *path;

    while ((n = lys_ext_iter(target->ext, target->ext_size, n + 1, ext->insubstmt)) != -1) {
        if (target->ext[n]->def != ext->def) {
            continue;
        }

        if (ext->def->argument) {
            /* check matching arguments */
            if (!ly_strequal(target->ext[n]->arg_value, ext->arg_value, 1)) {
                continue;
            }
        }

        /* we have the matching extension - remove it */
        ++found;
        lyp_ext_instance_rm(target->module->ctx, &target->ext, &target->ext_size, n);
        --n;
    }

    if (!found) {
        path = lys_path(target);
        LOGERR(LY_EVALID, "Extension deviation: extension \"%s\" to delete not found in \"%s\".", ext->def->name, path)
        free(path);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static int
lyp_deviate_apply_ext(struct lys_deviate *dev, struct lys_node *target, LYEXT_SUBSTMT substmt, struct lys_ext *extdef)
{
    struct ly_ctx *ctx;
    int m, n;
    struct lys_ext_instance *new;
    void *reallocated;

    /* LY_DEVIATE_ADD and LY_DEVIATE_RPL are very similar so they are implement the same way - in replacing,
     * there can be some extension instances in the target, in case of adding, there should not be any so we
     * will be just adding. */

    ctx = target->module->ctx; /* shortcut */
    m = n = -1;

    while ((m = lys_ext_iter(dev->ext, dev->ext_size, m + 1, substmt)) != -1) {
        /* deviate and its substatements include extensions, copy them to the target, replacing the previous
         * extensions if any. In case of deviating extension itself, we have to deviate only the same type
         * of the extension as specified in the deviation */
        if (substmt == LYEXT_SUBSTMT_SELF && dev->ext[m]->def != extdef) {
            continue;
        }

        if (substmt == LYEXT_SUBSTMT_SELF && dev->mod == LY_DEVIATE_ADD) {
            /* in case of adding extension, we will be replacing only the inherited extensions */
            do {
                n = lys_ext_iter(target->ext, target->ext_size, n + 1, substmt);
            } while (n != -1 && (target->ext[n]->def != extdef || !(target->ext[n]->flags & LYEXT_OPT_INHERIT)));
        } else {
            /* get the index of the extension to replace in the target node */
            do {
                n = lys_ext_iter(target->ext, target->ext_size, n + 1, substmt);
                /* if we are applying extension deviation, we have to deviate only the same type of the extension */
            } while (n != -1 && substmt == LYEXT_SUBSTMT_SELF && target->ext[n]->def != extdef);
        }

        /* TODO cover complex extension instances
           ((struct lys_ext_instance_complex*)(target->ext[n])->module = target->module;
         */
        if (n == -1) {
            /* nothing to replace, we are going to add it - reallocate */
            new = malloc(sizeof **target->ext);
            if (!new) {
                LOGMEM;
                return EXIT_FAILURE;
            }
            reallocated = realloc(target->ext, (target->ext_size + 1) * sizeof *target->ext);
            if (!reallocated) {
                LOGMEM;
                free(new);
                return EXIT_FAILURE;
            }
            target->ext = reallocated;
            target->ext_size++;

            n = target->ext_size - 1;
        } else {
            /* replacing - the original set of extensions is actually backuped together with the
             * node itself, so we are supposed only to free the allocated data here ... */
            lys_extension_instances_free(ctx, target->ext[n]->ext, target->ext[n]->ext_size);
            lydict_remove(ctx, target->ext[n]->arg_value);
            free(target->ext[n]);

            /* and prepare the new structure */
            new = malloc(sizeof **target->ext);
            if (!new) {
                LOGMEM;
                return EXIT_FAILURE;
            }
        }
        /* common part for adding and replacing - fill the newly created / replaceing cell */
        target->ext[n] = new;
        target->ext[n]->def = dev->ext[m]->def;
        target->ext[n]->arg_value = lydict_insert(ctx, dev->ext[m]->arg_value, 0);
        target->ext[n]->flags = 0;
        target->ext[n]->parent = target;
        target->ext[n]->parent_type = LYEXT_PAR_NODE;
        target->ext[n]->insubstmt = substmt;
        target->ext[n]->insubstmt_index = dev->ext[m]->insubstmt_index;
        target->ext[n]->ext_size = dev->ext[m]->ext_size;
        lys_ext_dup(target->module, dev->ext[m]->ext, dev->ext[m]->ext_size, target, LYEXT_PAR_NODE,
                    &target->ext[n]->ext, NULL);
    }

    /* remove the rest of extensions belonging to the original substatement in the target node,
     * due to possible reverting of the deviation effect, they are actually not removed, just moved
     * to the backup of the original node when the original node is backuped, here we just have to
     * free the replaced / deleted originals */
    while ((n = lys_ext_iter(target->ext, target->ext_size, n + 1, substmt)) != -1) {
        if (substmt == LYEXT_SUBSTMT_SELF) {
            /* if we are applying extension deviation, we are going to remove only
             * - the same type of the extension in case of replacing
             * - the same type of the extension which was inherited in case of adding
             * note - delete deviation is covered in lyp_deviate_del_ext */
            if (target->ext[n]->def != extdef ||
                    (dev->mod == LY_DEVIATE_ADD && !(target->ext[n]->flags & LYEXT_OPT_INHERIT))) {
                /* keep this extension */
                continue;
            }

        }

        /* remove the item */
        lyp_ext_instance_rm(ctx, &target->ext, &target->ext_size, n);
        --n;
    }

    return EXIT_SUCCESS;
}

/*
 * not-supported deviations are not processed since they affect the complete node, not just their substatements
 */
int
lyp_deviation_apply_ext(struct lys_module *module)
{
    int i, j, k;
    struct lys_deviate *dev;
    struct lys_node *target;
    struct ly_set *extset;

    for (i = 0; i < module->deviation_size; i++) {
        resolve_augment_schema_nodeid(module->deviation[i].target_name, NULL, module, 0,
                                      (const struct lys_node **)&target);
        if (!target) {
            /* LY_DEVIATE_NO */
            continue;
        }
        for (j = 0; j < module->deviation[i].deviate_size; j++) {
            dev = &module->deviation[i].deviate[j];
            if (!dev->ext_size) {
                /* no extensions in deviate and its substatement, nothing to do here */
                continue;
            }

            /* extensions */
            if (dev->mod == LY_DEVIATE_DEL) {
                k = -1;
                while ((k = lys_ext_iter(dev->ext, dev->ext_size, k + 1, LYEXT_SUBSTMT_SELF)) != -1) {
                    if (lyp_deviate_del_ext(target, dev->ext[k])) {
                        return EXIT_FAILURE;
                    }
                }

                /* In case of LY_DEVIATE_DEL, we are applying only extension deviation, removing
                 * of the substatement's extensions was already done when the substatement was applied.
                 * Extension deviation could not be applied by the parser since the extension could be unresolved,
                 * which is not the issue of the other substatements. */
                continue;
            } else {
                extset = ly_set_new();
                k = -1;
                while ((k = lys_ext_iter(dev->ext, dev->ext_size, k + 1, LYEXT_SUBSTMT_SELF)) != -1) {
                    ly_set_add(extset, dev->ext[k]->def, 0);
                }
                for (k = 0; (unsigned int)k < extset->number; k++) {
                    if (lyp_deviate_apply_ext(dev, target, LYEXT_SUBSTMT_SELF, (struct lys_ext *)extset->set.g[k])) {
                        ly_set_free(extset);
                        return EXIT_FAILURE;
                    }
                }
                ly_set_free(extset);
            }

            /* unique */
            if (dev->unique_size && lyp_deviate_apply_ext(dev, target, LYEXT_SUBSTMT_UNIQUE, NULL)) {
                return EXIT_FAILURE;
            }
            /* units */
            if (dev->units && lyp_deviate_apply_ext(dev, target, LYEXT_SUBSTMT_UNITS, NULL)) {
                return EXIT_FAILURE;
            }
            /* default */
            if (dev->dflt_size && lyp_deviate_apply_ext(dev, target, LYEXT_SUBSTMT_DEFAULT, NULL)) {
                return EXIT_FAILURE;
            }
            /* config */
            if ((dev->flags & LYS_CONFIG_MASK) && lyp_deviate_apply_ext(dev, target, LYEXT_SUBSTMT_CONFIG, NULL)) {
                return EXIT_FAILURE;
            }
            /* mandatory */
            if ((dev->flags & LYS_MAND_MASK) && lyp_deviate_apply_ext(dev, target, LYEXT_SUBSTMT_MANDATORY, NULL)) {
                return EXIT_FAILURE;
            }
            /* min/max */
            if (dev->min_set && lyp_deviate_apply_ext(dev, target, LYEXT_SUBSTMT_MIN, NULL)) {
                return EXIT_FAILURE;
            }
            if (dev->min_set && lyp_deviate_apply_ext(dev, target, LYEXT_SUBSTMT_MAX, NULL)) {
                return EXIT_FAILURE;
            }
            /* type and must contain extension instances in their structures */
        }
    }

    return EXIT_SUCCESS;
}

int
lyp_ctx_check_module(struct lys_module *module)
{
    struct ly_ctx *ctx;
    int i, match_i = -1, to_implement;
    const char *last_rev = NULL;

    assert(module);
    to_implement = 0;
    ctx = module->ctx;

    /* find latest revision */
    for (i = 0; i < module->rev_size; ++i) {
        if (!last_rev || (strcmp(last_rev, module->rev[i].date) < 0)) {
            last_rev = module->rev[i].date;
        }
    }

    for (i = 0; i < ctx->models.used; i++) {
        /* check name (name/revision) and namespace uniqueness */
        if (!strcmp(ctx->models.list[i]->name, module->name)) {
            if (to_implement) {
                if (i == match_i) {
                    continue;
                }
                LOGERR(LY_EINVAL, "Module \"%s\" in another revision already implemented.", ctx->models.list[i]->name);
                return -1;
            } else if (!ctx->models.list[i]->rev_size && module->rev_size) {
                LOGERR(LY_EINVAL, "Module \"%s\" without revision already in context.", ctx->models.list[i]->name);
                return -1;
            } else if (ctx->models.list[i]->rev_size && !module->rev_size) {
                LOGERR(LY_EINVAL, "Module \"%s\" with revision already in context.", ctx->models.list[i]->name);
                return -1;
            } else if ((!module->rev_size && !ctx->models.list[i]->rev_size)
                    || !strcmp(ctx->models.list[i]->rev[0].date, last_rev)) {

                LOGVRB("Module \"%s\" already in context.", ctx->models.list[i]->name);
                to_implement = module->implemented;
                match_i = i;
                if (to_implement && !ctx->models.list[i]->implemented) {
                    /* check first that it is okay to change it to implemented */
                    i = -1;
                    continue;
                }
                return 1;

            } else if (module->implemented && ctx->models.list[i]->implemented) {
                LOGERR(LY_EINVAL, "Module \"%s\" in another revision already implemented.", ctx->models.list[i]->name);
                return -1;
            }
            /* else keep searching, for now the caller is just adding
             * another revision of an already present schema
             */
        } else if (!strcmp(ctx->models.list[i]->ns, module->ns)) {
            LOGERR(LY_EINVAL, "Two different modules (\"%s\" and \"%s\") have the same namespace \"%s\".",
                   ctx->models.list[i]->name, module->name, module->ns);
            return -1;
        }
    }

    if (to_implement) {
        if (lys_set_implemented(ctx->models.list[match_i])) {
            return -1;
        }
        return 1;
    }

    return 0;
}

int
lyp_ctx_add_module(struct lys_module *module)
{
    struct lys_module **newlist = NULL;
    int i;

    assert(!lyp_ctx_check_module(module));

    /* add to the context's list of modules */
    if (module->ctx->models.used == module->ctx->models.size) {
        newlist = realloc(module->ctx->models.list, (2 * module->ctx->models.size) * sizeof *newlist);
        if (!newlist) {
            LOGMEM;
            return -1;
        }
        for (i = module->ctx->models.size; i < module->ctx->models.size * 2; i++) {
            newlist[i] = NULL;
        }
        module->ctx->models.size *= 2;
        module->ctx->models.list = newlist;
    }
    module->ctx->models.list[module->ctx->models.used++] = module;
    module->ctx->models.module_set_id++;

    return 0;
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
