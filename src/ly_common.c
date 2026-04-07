/**
 * @file ly_common.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief common internal definitions for libyang
 *
 * Copyright (c) 2018 - 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "ly_common.h"

#ifdef HAVE_REGEX_H
# include <regex.h>
#endif

#ifndef _WIN32
# ifdef HAVE_MMAP
#  include <sys/mman.h>
# endif
#else
# include <io.h>
#endif

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "compat.h"
#include "schema_compile_node.h"
#include "tree_data_internal.h"
#include "tree_schema_internal.h"
#include "version.h"
#include "xml.h"

/**< lock for creating and destroying both private & shared context data */
static pthread_rwlock_t ly_ctx_data_rwlock = PTHREAD_RWLOCK_INITIALIZER;

/**< sized array ([sized array](@ref sizedarrays)) of pointers to private context data (safe realloc).
 * The context is identified by the thread ID of the thread that created it and its address. */
static struct ly_ctx_private_data **ly_private_ctx_data;

/**< sized array ([sized array](@ref sizedarrays)) of pointers to shared context data (safe realloc).
 * The context is identified by the memory address of the context. */
static struct ly_ctx_shared_data **ly_shared_ctx_data;

LIBYANG_API_DEF uint32_t
ly_version_so_major(void)
{
    return LY_VERSION_MAJOR;
}

LIBYANG_API_DEF uint32_t
ly_version_so_minor(void)
{
    return LY_VERSION_MINOR;
}

LIBYANG_API_DEF uint32_t
ly_version_so_micro(void)
{
    return LY_VERSION_MICRO;
}

LIBYANG_API_DEF const char *
ly_version_so_str(void)
{
    return LY_VERSION;
}

LIBYANG_API_DEF uint32_t
ly_version_proj_major(void)
{
    return LY_PROJ_VERSION_MAJOR;
}

LIBYANG_API_DEF uint32_t
ly_version_proj_minor(void)
{
    return LY_PROJ_VERSION_MINOR;
}

LIBYANG_API_DEF uint32_t
ly_version_proj_micro(void)
{
    return LY_PROJ_VERSION_MICRO;
}

LIBYANG_API_DEF const char *
ly_version_proj_str(void)
{
    return LY_PROJ_VERSION;
}

/**
 * @brief Callback for comparing two leafref links records.
 */
ly_bool
ly_ctx_ht_leafref_links_equal_cb(void *val1_p, void *val2_p, ly_bool UNUSED(mod), void *UNUSED(cb_data))
{
    struct lyd_leafref_links_rec **rec1 = val1_p, **rec2 = val2_p;

    return (*rec1)->node == (*rec2)->node;
}

/**
 * @brief Callback for freeing two leafref links records.
 */
void
ly_ctx_ht_leafref_links_rec_free(void *val_p)
{
    struct lyd_leafref_links_rec **rec = val_p;

    lyd_free_leafref_links_rec(*rec);
    free(*rec);
}

/**
 * @brief Callback for comparing two pattern records.
 */
static ly_bool
ly_ctx_ht_pattern_equal_cb(void *val1_p, void *val2_p, ly_bool UNUSED(mod), void *UNUSED(cb_data))
{
    struct ly_pattern_ht_rec *val1 = val1_p;
    struct ly_pattern_ht_rec *val2 = val2_p;

    /* compare the pattern strings, if they match we can use the stored
     * serialized value to create the pcre2 code for the pattern */
    if (!strcmp(val1->pattern, val2->pattern) && (val1->format == val2->format)) {
        return 1;
    }

    return 0;
}

/**
 * @brief Remove private context data from the sized array and free its contents.
 *
 * @param[in] private_data Private context data to free.
 */
static void
ly_ctx_private_data_remove_and_free(struct ly_ctx_private_data *private_data)
{
    LY_ARRAY_COUNT_TYPE u;

    if (!private_data) {
        return;
    }

    /* free members */
    ly_err_free(private_data->errs);
    free(private_data);

    /* find */
    LY_ARRAY_FOR(ly_private_ctx_data, u) {
        if (ly_private_ctx_data[u] == private_data) {
            break;
        }
    }
    assert(u < LY_ARRAY_COUNT(ly_private_ctx_data));

    /* remove */
    if (u < LY_ARRAY_COUNT(ly_private_ctx_data) - 1) {
        /* replace the private data with the last one if it even was added */
        ly_private_ctx_data[u] = ly_private_ctx_data[LY_ARRAY_COUNT(ly_private_ctx_data) - 1];
    }
    LY_ARRAY_DECREMENT_FREE(ly_private_ctx_data);
}

/**
 * @brief Create the private context data for a specific context.
 *
 * @param[in] ctx Context to create the private data for.
 * @param[out] private_data Optional created private context data.
 * @return LY_SUCCESS on success, LY_EMEM on memory allocation failure.
 */
static LY_ERR
ly_ctx_private_data_create(const struct ly_ctx *ctx, struct ly_ctx_private_data **private_data)
{
    pthread_t tid = pthread_self();
    struct ly_ctx_private_data **priv_data = NULL;
    LY_ERR rc = LY_SUCCESS;

    *private_data = NULL;

    /* create the private context data */
    LY_ARRAY_NEW_GOTO(ctx, ly_private_ctx_data, priv_data, rc, cleanup);
    *priv_data = calloc(1, sizeof **priv_data);
    LY_CHECK_ERR_GOTO(!*priv_data, LOGMEM(ctx); rc = LY_EMEM, cleanup);

    /* fill */
    (*priv_data)->tid = tid;
    (*priv_data)->ctx = ctx;
    (*priv_data)->errs = NULL;

    *private_data = *priv_data;

cleanup:
    if (rc && priv_data) {
        ly_ctx_private_data_remove_and_free(*priv_data);
    }
    return rc;
}

/**
 * @brief Get the private context data for a specific context.
 *
 * @param[in] ctx Context whose private data to get.
 * @param[in] own_data_only Whether to return only the private data of the current thread.
 * @return Private context data of @p ctx, NULL if not found.
 */
static struct ly_ctx_private_data *
_ly_ctx_private_data_get(const struct ly_ctx *ctx, ly_bool own_data_only)
{
    struct ly_ctx_private_data **iter;
    ly_bool found = 0;
    pthread_t tid = pthread_self();

    LY_ARRAY_FOR(ly_private_ctx_data, struct ly_ctx_private_data *, iter) {
        /* either own - ctx and tid match, or "context's" - thread does not matter */
        if (((*iter)->ctx == ctx) && (!own_data_only || pthread_equal((*iter)->tid, tid))) {
            found = 1;
            break;
        }
    }

    return found ? *iter : NULL;
}

struct ly_ctx_private_data *
ly_ctx_private_data_get_or_create(const struct ly_ctx *ctx)
{
    struct ly_ctx_private_data *private_data;
    LY_ERR r;

    while (ctx->parent_ctx) {
        /* find the right context */
        ctx = ctx->parent_ctx;
    }

    /* RD LOCK */
    pthread_rwlock_rdlock(&ly_ctx_data_rwlock);

    /* try to find existing data */
    private_data = _ly_ctx_private_data_get(ctx, 1);

    /* RD UNLOCK */
    pthread_rwlock_unlock(&ly_ctx_data_rwlock);

    if (private_data) {
        return private_data;
    }

    /* WR LOCK */
    pthread_rwlock_wrlock(&ly_ctx_data_rwlock);

    /* create it */
    r = ly_ctx_private_data_create(ctx, &private_data);
    if (r) {
        /* NULL to avoid infinite loop in LOGERR */
        LOGERR(NULL, r, "Failed to create context private data.");
        assert(0);
    }

    /* WR UNLOCK */
    pthread_rwlock_unlock(&ly_ctx_data_rwlock);

    return private_data;
}

/**
 * @brief Remove shared context data from the sized array and free its contents.
 *
 * @param[in] shared_data Shared context data to free.
 */
static void
ly_ctx_shared_data_remove_and_free(struct ly_ctx_shared_data *shared_data)
{
    LY_ARRAY_COUNT_TYPE u;

    if (!shared_data) {
        return;
    }

    /* all the patterns must have been removed already,
     * either while free compiled modules (standard behavior)
     * or when assigning a parent to a context, it's shared data will be used (schema mount) */
    assert(shared_data->pattern_ht->used == 0);
    lyht_free(shared_data->pattern_ht, NULL);

    /* free rest of the members */
    lydict_clean(shared_data->data_dict);
    free(shared_data->data_dict);
    lyht_free(shared_data->leafref_links_ht, ly_ctx_ht_leafref_links_rec_free);
    free(shared_data);

    /* find */
    LY_ARRAY_FOR(ly_shared_ctx_data, u) {
        if (ly_shared_ctx_data[u] == shared_data) {
            break;
        }
    }
    assert(u < LY_ARRAY_COUNT(ly_shared_ctx_data));

    /* remove */
    if (u < LY_ARRAY_COUNT(ly_shared_ctx_data) - 1) {
        /* replace the shared data with the last one */
        ly_shared_ctx_data[u] = ly_shared_ctx_data[LY_ARRAY_COUNT(ly_shared_ctx_data) - 1];
    }
    LY_ARRAY_DECREMENT_FREE(ly_shared_ctx_data);
}

/**
 * @brief Create the shared context data for a specific context.
 *
 * @param[in] ctx Context to create the shared data for.
 * @param[out] shared_data Optional created shared context data.
 * @return LY_SUCCESS on success, LY_EMEM on memory allocation failure.
 */
static LY_ERR
ly_ctx_shared_data_create(const struct ly_ctx *ctx, struct ly_ctx_shared_data **shared_data)
{
    LY_ERR rc = LY_SUCCESS;
    struct ly_ctx_shared_data **shrd_data = NULL;
    pthread_mutexattr_t attr;

    if (shared_data) {
        *shared_data = NULL;
    }

    /* create the shared context data */
    LY_ARRAY_NEW_GOTO(ctx, ly_shared_ctx_data, shrd_data, rc, cleanup);
    *shrd_data = calloc(1, sizeof **shrd_data);
    LY_CHECK_ERR_GOTO(!*shrd_data, LOGMEM(ctx); rc = LY_EMEM, cleanup);

    /* fill */
    (*shrd_data)->ctx = ctx;

    /* pattern hash table */
    (*shrd_data)->pattern_ht = lyht_new(LYHT_MIN_SIZE, sizeof(struct ly_pattern_ht_rec),
            ly_ctx_ht_pattern_equal_cb, NULL, 1);
    LY_CHECK_ERR_GOTO(!(*shrd_data)->pattern_ht, rc = LY_EMEM, cleanup);

    /* data dictionary */
    (*shrd_data)->data_dict = malloc(sizeof *(*shrd_data)->data_dict);
    LY_CHECK_ERR_GOTO(!(*shrd_data)->data_dict, rc = LY_EMEM, cleanup);
    lydict_init((*shrd_data)->data_dict);

    /* leafref set */
    if (ctx->opts & LY_CTX_LEAFREF_LINKING) {
        /**
         * storing the pointer instead of record itself is needed to avoid invalid memory reads. Hash table can reallocate
         * its memory completely during various manipulation function (e.g. remove, insert). In case of using pointers, the
         * pointer can be reallocated safely, while record itself remains untouched and can be accessed/modified freely
         * */
        (*shrd_data)->leafref_links_ht = lyht_new(1, sizeof(struct lyd_leafref_links_rec *),
                ly_ctx_ht_leafref_links_equal_cb, NULL, 1);
        LY_CHECK_ERR_GOTO(!(*shrd_data)->leafref_links_ht, rc = LY_EMEM, cleanup);
    }

    /* ext clb and leafref links locks */
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&(*shrd_data)->ext_clb_lock, &attr);
    pthread_mutexattr_destroy(&attr);
    pthread_mutex_init(&(*shrd_data)->leafref_links_lock, NULL);

    /* refcount */
    ATOMIC_STORE_RELAXED((*shrd_data)->refcount, 1);

    if (shared_data) {
        *shared_data = *shrd_data;
    }

cleanup:
    if (rc && shrd_data) {
        ly_ctx_shared_data_remove_and_free(*shrd_data);
    }
    return rc;
}

/**
 * @brief Get the shared context data for a specific context.
 *
 * @param[in] ctx Context whose shared data to get.
 * @return Found shared ctx data, NULL if not found.
 */
static struct ly_ctx_shared_data *
_ly_ctx_shared_data_get(const struct ly_ctx *ctx)
{
    struct ly_ctx_shared_data **iter;
    ly_bool found = 0;

    LY_ARRAY_FOR(ly_shared_ctx_data, struct ly_ctx_shared_data *, iter) {
        if ((*iter)->ctx == ctx) {
            found = 1;
            break;
        }
    }

    return found ? *iter : NULL;
}

struct ly_ctx_shared_data *
ly_ctx_shared_data_get(const struct ly_ctx *ctx)
{
    struct ly_ctx_shared_data *shared_data;

    while (ctx->parent_ctx) {
        /* find the right context */
        ctx = ctx->parent_ctx;
    }

    /* RD LOCK */
    pthread_rwlock_rdlock(&ly_ctx_data_rwlock);

    shared_data = _ly_ctx_shared_data_get(ctx);

    /* RD UNLOCK */
    pthread_rwlock_unlock(&ly_ctx_data_rwlock);

    if (!shared_data) {
        /* NULL to avoid infinite loop in LOGERR */
        LOGERR(NULL, LY_EINT, "Context shared data not found.");
        assert(0);
    }
    return shared_data;
}

struct ly_dict *
ly_ctx_data_dict_get(const struct ly_ctx *ctx)
{
    struct ly_ctx_shared_data *shared_data;

    shared_data = ly_ctx_shared_data_get(ctx);
    return shared_data ? shared_data->data_dict : NULL;
}

LY_ERR
ly_ctx_data_add(const struct ly_ctx *ctx)
{
    LY_ERR rc = LY_SUCCESS;
    struct ly_ctx_private_data *private_data;
    struct ly_ctx_shared_data *shared_data;

    /* WR LOCK */
    pthread_rwlock_wrlock(&ly_ctx_data_rwlock);

    /* check for duplicates in private context data, not allowed */
    private_data = _ly_ctx_private_data_get(ctx, 1);
    if (private_data) {
        /* ctx pointers can match only if the context is printed (they have the same memory address) */
        assert(private_data->ctx->opts & LY_CTX_INT_IMMUTABLE);

        /* use NULL as ctx to avoid RD lock while holding WR lock */
        LOGERR(NULL, LY_EEXIST, "Only one printed context per memory chunk and thread is allowed.");
        rc = LY_EEXIST;
        goto cleanup;
    }

    /* create the private context data, freed by the caller in case we fail later on */
    rc = ly_ctx_private_data_create(ctx, &private_data);
    if (rc) {
        /* NULL to avoid infinite loop in LOGERR */
        LOGERR(NULL, rc, "Failed to create context private data.");
        goto cleanup;
    }

    /* check for duplicates in shared context data */
    shared_data = _ly_ctx_shared_data_get(ctx);
    if (shared_data) {
        /* found, we can end */
        ATOMIC_INC_RELAXED(shared_data->refcount);
        goto cleanup;
    }

    /* create the shared context data */
    rc = ly_ctx_shared_data_create(ctx, NULL);
    if (rc) {
        /* NULL to avoid infinite loop in LOGERR */
        LOGERR(NULL, rc, "Failed to create context shared data.");
        goto cleanup;
    }

cleanup:
    /* WR UNLOCK */
    pthread_rwlock_unlock(&ly_ctx_data_rwlock);
    return rc;
}

void
ly_ctx_data_del(const struct ly_ctx *ctx)
{
    struct ly_ctx_private_data *private_data;
    struct ly_ctx_shared_data *shared_data;

    /* WR LOCK */
    pthread_rwlock_wrlock(&ly_ctx_data_rwlock);

    /* free private data of all the threads for this context */
    while ((private_data = _ly_ctx_private_data_get(ctx, 0))) {
        /* free the private data */
        ly_ctx_private_data_remove_and_free(private_data);
    }

    /* get the shared context data */
    shared_data = _ly_ctx_shared_data_get(ctx);
    if (!shared_data) {
        /* not found, nothing to do */
        goto cleanup;
    }
    assert(ATOMIC_LOAD_RELAXED(shared_data->refcount));

    /* decrease refcount */
    ATOMIC_DEC_RELAXED(shared_data->refcount);
    if (ATOMIC_LOAD_RELAXED(shared_data->refcount)) {
        goto cleanup;
    }

    /* free the shared members */
    ly_ctx_shared_data_remove_and_free(shared_data);

cleanup:
    /* WR UNLOCK */
    pthread_rwlock_unlock(&ly_ctx_data_rwlock);
}

void
ly_ctx_pattern_ht_erase(const struct ly_ctx *ctx)
{
    struct ly_ctx_shared_data *ctx_data;
    struct ly_ht_rec *rec;
    struct ly_pattern_ht_rec *pat_rec;
    uint32_t hlist_idx, rec_idx;

    ctx_data = ly_ctx_shared_data_get(ctx);

    /* free all the stored records */
    LYHT_ITER_ALL_RECS(ctx_data->pattern_ht, hlist_idx, rec_idx, rec) {
        pat_rec = (struct ly_pattern_ht_rec *)&rec->val;

        ly_pat_free(pat_rec->pat_comp, pat_rec->format);
    }

    /* we have removed all patterns (so it is empty), we can not free the ht here though, to avoid
     * double free, but just trick it to look empty */
    ctx_data->pattern_ht->used = 0;
}

LY_ERR
ly_ctx_shared_data_pattern_get(const struct ly_ctx *ctx, const char *pattern, ly_bool format, const void **pat_comp)
{
    LY_ERR rc = LY_SUCCESS;
    struct ly_ctx_shared_data *ctx_data;
    struct ly_pattern_ht_rec rec = {0}, *found_rec = NULL;
    uint32_t hash;
    void *pat_comp_tmp = NULL;
    struct ly_err_item *err = NULL;

    assert(ctx && pattern);

    if (pat_comp) {
        *pat_comp = NULL;
    }

    /* get the context shared data */
    ctx_data = ly_ctx_shared_data_get(ctx);
    LY_CHECK_RET(!ctx_data, LY_EINT);

    /* try to find the pattern code in the pattern ht */
    hash = lyht_hash(pattern, strlen(pattern));
    rec.pattern = pattern;
    rec.format = format;
    if (!lyht_find(ctx_data->pattern_ht, &rec, hash, (void **)&found_rec)) {
        /* pat_comp cached */
        if (pat_comp) {
            *pat_comp = found_rec->pat_comp;
        }
        goto cleanup;
    }

    /* not found and it can be because:
     * 1) it's the first time the pattern is compiled in the context;
     * 2) we are using printed context (which compiles the patterns on the fly);
     * 3) the pattern was compiled for several types but then the types had to be recompiled (lysc_type_free()
     *    in lys_compile_type()) and we are no longer able to track the pattern code cache. */
    LY_CHECK_GOTO(rc = ly_pat_compile(pattern, format, &pat_comp_tmp, &err), cleanup);

    /* store the compiled pattern code in the hash table */
    rec.pat_comp = pat_comp_tmp;
    LY_CHECK_GOTO(rc = lyht_insert_no_check(ctx_data->pattern_ht, &rec, hash, NULL), cleanup);

    if (pat_comp) {
        *pat_comp = pat_comp_tmp;
    }
    pat_comp_tmp = NULL;

cleanup:
    ly_pat_free(pat_comp_tmp, format);
    if (err) {
        /* log with the schema path */
        ly_vlog(ctx, err->apptag, NULL, err->vecode, "%s", err->msg);
        ly_err_free(err);
    }
    return rc;
}

void
ly_ctx_shared_data_pattern_del(const struct ly_ctx *ctx, const char *pattern, ly_bool format)
{
    struct ly_ctx_shared_data *ctx_data;
    struct ly_pattern_ht_rec rec = {0}, *found_rec = NULL;
    uint32_t hash;

    assert(ctx && pattern);

    /* get the context shared data */
    ctx_data = ly_ctx_shared_data_get(ctx);
    LY_CHECK_ERR_RET(!ctx_data, LOGINT(ctx), );

    /* try to find the pattern code in the pattern ht */
    hash = lyht_hash(pattern, strlen(pattern));
    rec.pattern = pattern;
    rec.format = format;

    if (lyht_find(ctx_data->pattern_ht, &rec, hash, (void **)&found_rec)) {
        /* pattern code not cached, this may happen when using printed context,
         * because then the pcodes are obtained on demand */
        return;
    }

    /* found it, free */
    ly_pat_free(found_rec->pat_comp, found_rec->format);

    /* free the pattern HT record */
    if (lyht_remove(ctx_data->pattern_ht, &rec, hash)) {
        LOGINT(ctx);
    }
}

/**
 * @brief Compile a POSIX pattern.
 *
 * @param[in] pattern Pattern to compile.
 * @param[out] pat_comp Compiled pattern.
 * @param[out] err Generated error, if any.
 * @return LY_ERR value.
 */
static LY_ERR
ly_pat_compile_posix(const char *pattern, void **pat_comp, struct ly_err_item **err)
{
#ifdef HAVE_REGEX_H
    LY_ERR rc = LY_SUCCESS;
    int err_code;
    size_t err_len;
    char *err_msg = NULL;
    regex_t *preg_p = NULL;

    /* alloc memory */
    preg_p = calloc(1, sizeof *preg_p);
    if (!preg_p) {
        rc = ly_err_new(err, LY_EMEM, 0, NULL, NULL, LY_EMEM_MSG);
        goto cleanup;
    }

    /* try to compile the pattern */
    err_code = regcomp(preg_p, pattern, REG_EXTENDED);
    if (err_code) {
        err_len = regerror(err_code, NULL, NULL, 0);
        err_msg = malloc(err_len);
        if (err_msg) {
            regerror(err_code, NULL, err_msg, err_len);
        }

        rc = ly_err_new(err, LY_EVALID, 0, NULL, NULL, "Regular expression \"%s\" is not valid (%s).", pattern, err_msg);
        goto cleanup;
    }

    /* return compiled pattern */
    *pat_comp = preg_p;
    preg_p = NULL;

cleanup:
    if (preg_p) {
        regfree(preg_p);
        free(preg_p);
    }
    free(err_msg);
    return rc;
#else
    (void)pattern;
    (void)pat_comp;

    return ly_err_new(err, LY_EINVAL, 0, NULL, NULL, "POSIX patterns are not supported.");
#endif
}

/**
 * @brief Transform characters block in an XML Schema pattern into Perl character ranges.
 *
 * @param[in] pattern Pattern to compile.
 * @param[in,out] regex Pattern (regex) to modify.
 * @param[out] err Generated error, if any.
 * @return LY_ERR value.
 */
static LY_ERR
ly_pat_compile_xmlschema_chblocks_xmlschema2perl(const char *pattern, char **regex, struct ly_err_item **err)
{
#define URANGE_LEN 19
    char *ublock2urange[][2] = {
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
        {"Specials", "[\\x{FEFF}|\\x{FFF0}-\\x{FFFD}]"},
        {NULL, NULL}
    };

    size_t idx, idx2, start, end, ublock;
    char *perl_regex, *ptr;

    perl_regex = *regex;

    /* substitute Unicode Character Blocks with exact Character Ranges */
    while ((ptr = strstr(perl_regex, "\\p{Is"))) {
        start = ptr - perl_regex;

        ptr = strchr(ptr, '}');
        if (!ptr) {
            return ly_err_new(err, LY_EVALID, 0, NULL, NULL, "Regular expression \"%s\" is not valid (\"%s\": %s).",
                    pattern, perl_regex + start + 2, "unterminated character property");
        }
        end = (ptr - perl_regex) + 1;

        /* need more space */
        if (end - start < URANGE_LEN) {
            perl_regex = ly_realloc(perl_regex, strlen(perl_regex) + (URANGE_LEN - (end - start)) + 1);
            *regex = perl_regex;
            if (!perl_regex) {
                return ly_err_new(err, LY_EMEM, 0, NULL, NULL, LY_EMEM_MSG);
            }
        }

        /* find our range */
        for (idx = 0; ublock2urange[idx][0]; ++idx) {
            if (!strncmp(perl_regex + start + ly_strlen_const("\\p{Is"),
                    ublock2urange[idx][0], strlen(ublock2urange[idx][0]))) {
                break;
            }
        }
        if (!ublock2urange[idx][0]) {
            return ly_err_new(err, LY_EVALID, 0, NULL, NULL, "Regular expression \"%s\" is not valid (\"%s\": %s).",
                    pattern, perl_regex + start + 5, "unknown block name");
        }
        ublock = idx;

        /* make the space in the string and replace the block (but we cannot include brackets if it was already enclosed in them) */
        for (idx2 = 0, idx = 0; idx2 < start; ++idx2) {
            if ((perl_regex[idx2] == '[') && (!idx2 || (perl_regex[idx2 - 1] != '\\'))) {
                ++idx;
            }
            if ((perl_regex[idx2] == ']') && (!idx2 || (perl_regex[idx2 - 1] != '\\'))) {
                assert(idx);
                --idx;
            }
        }
        if (idx) {
            /* skip brackets */
            memmove(perl_regex + start + (URANGE_LEN - 2), perl_regex + end, strlen(perl_regex + end) + 1);
            memcpy(perl_regex + start, ublock2urange[ublock][1] + 1, URANGE_LEN - 2);
        } else {
            memmove(perl_regex + start + URANGE_LEN, perl_regex + end, strlen(perl_regex + end) + 1);
            memcpy(perl_regex + start, ublock2urange[ublock][1], URANGE_LEN);
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Compile an XML Schema pattern.
 *
 * @param[in] pattern Pattern to compile.
 * @param[out] pat_comp Compiled pattern.
 * @param[out] err Generated error, if any.
 * @return LY_ERR value.
 */
static LY_ERR
ly_pat_compile_xmlschema(const char *pattern, void **pat_comp, struct ly_err_item **err)
{
    LY_ERR rc = LY_SUCCESS;
    size_t idx, size, brack;
    char *perl_regex = NULL;
    int err_code, compile_opts;
    const char *orig_ptr;
    PCRE2_SIZE err_offset;
    pcre2_code *code_local;
    ly_bool escaped;

    /* adjust the expression to a Perl equivalent
     * http://www.w3.org/TR/2004/REC-xmlschema-2-20041028/#regexs */

    /* allocate space for the transformed pattern */
    size = strlen(pattern) + 1;
    compile_opts = PCRE2_UTF | PCRE2_UCP | PCRE2_DOLLAR_ENDONLY | PCRE2_NO_AUTO_CAPTURE | PCRE2_ANCHORED;
#ifdef PCRE2_ENDANCHORED
    compile_opts |= PCRE2_ENDANCHORED;
#else
    /* add space for trailing $ anchor */
    size++;
#endif

    perl_regex = malloc(size);
    if (!perl_regex) {
        rc = ly_err_new(err, LY_EMEM, 0, NULL, NULL, LY_EMEM_MSG);
        goto cleanup;
    }
    perl_regex[0] = '\0';

    /* we need to replace all "$" and "^" (that are not in "[]") with "\$" and "\^" */
    brack = 0;
    idx = 0;
    escaped = 0;
    orig_ptr = pattern;
    while (orig_ptr[0]) {
        switch (orig_ptr[0]) {
        case '$':
        case '^':
            if (!brack) {
                /* make space for the extra character */
                ++size;
                perl_regex = ly_realloc(perl_regex, size);
                if (!perl_regex) {
                    rc = ly_err_new(err, LY_EMEM, 0, NULL, NULL, LY_EMEM_MSG);
                    goto cleanup;
                }

                /* print escape slash */
                perl_regex[idx] = '\\';
                ++idx;
            }
            break;
        case '\\':
            /*  escape character found or backslash is escaped */
            escaped = !escaped;
            /* copy backslash and continue with the next character */
            perl_regex[idx] = orig_ptr[0];
            ++idx;
            ++orig_ptr;
            continue;
        case '[':
            if (!escaped) {
                ++brack;
            }
            break;
        case ']':
            if (!brack && !escaped) {
                /* If ']' does not terminate a character class expression, then pcre2_compile() implicitly escapes the
                * ']' character. But this seems to be against the regular expressions rules declared in
                * "XML schema: Datatypes" and therefore an error is returned. So for example if pattern is '\[a]' then
                * pcre2 match characters '[a]' literally but in YANG such pattern is not allowed.
                */
                rc = ly_err_new(err, LY_EVALID, 0, NULL, NULL, "Regular expression \"%s\" is not valid (\"%s\": %s).",
                        pattern, orig_ptr, "character group doesn't begin with '['");
                goto cleanup;
            } else if (!escaped) {
                --brack;
            }
            break;
        default:
            break;
        }

        /* copy char */
        perl_regex[idx] = orig_ptr[0];

        ++idx;
        ++orig_ptr;
        escaped = 0;
    }
#ifndef PCRE2_ENDANCHORED
    /* anchor match to end of subject */
    perl_regex[idx++] = '$';
#endif
    perl_regex[idx] = '\0';

    /* transform character blocks */
    if ((rc = ly_pat_compile_xmlschema_chblocks_xmlschema2perl(pattern, &perl_regex, err))) {
        goto cleanup;
    }

    /* must return 0, already checked during parsing */
    code_local = pcre2_compile((PCRE2_SPTR)perl_regex, PCRE2_ZERO_TERMINATED, compile_opts, &err_code, &err_offset, NULL);
    if (!code_local) {
        PCRE2_UCHAR err_msg[LY_PCRE2_MSG_LIMIT] = {0};

        pcre2_get_error_message(err_code, err_msg, LY_PCRE2_MSG_LIMIT);
        rc = ly_err_new(err, LY_EVALID, 0, NULL, NULL, "Regular expression \"%s\" is not valid (\"%s\": %s).",
                pattern, perl_regex + err_offset, err_msg);
        goto cleanup;
    }

    if (pat_comp) {
        *pat_comp = code_local;
    } else {
        pcre2_code_free(code_local);
    }

cleanup:
    free(perl_regex);
    return rc;

#undef URANGE_LEN
}

LY_ERR
ly_pat_compile(const char *pattern, ly_bool format, void **pat_comp, struct ly_err_item **err)
{
    if (format) {
        return ly_pat_compile_posix(pattern, pat_comp, err);
    } else {
        return ly_pat_compile_xmlschema(pattern, pat_comp, err);
    }
}

/**
 * @brief Match a string for a POSIX pattern.
 *
 * @param[in] pat_comp Compiled pattern, may not be set if @p pattern.
 * @param[in] pattern Pattern, may not be set if @p pat_comp.
 * @param[in] str String to match.
 * @param[in] str_len Length of @p str.
 * @param[out] err Generated error, if any.
 * @return LY_ENOT if @p str does not match.
 * @return LY_ERR value on error.
 */
static LY_ERR
ly_pat_match_posix(const void *pat_comp, const char *pattern, const char *str, size_t str_len, struct ly_err_item **err)
{
#ifdef HAVE_REGEX_H
    LY_ERR rc = LY_SUCCESS;
    int err_code;
    regex_t *preg_p = (void *)pat_comp;

    assert(pat_comp || pattern);

    if (!preg_p) {
        /* compile pattern first */
        rc = ly_pat_compile_posix(pattern, (void **)&preg_p, err);
        LY_CHECK_GOTO(rc, cleanup);
    }

# ifdef HAVE_REG_STARTEND
    regmatch_t pmatch = {0};

    /* match the string up to its length */
    pmatch.rm_so = 0;
    pmatch.rm_eo = str_len;
    err_code = regexec(preg_p, str, 1, &pmatch, REG_STARTEND);
# else
    /* must match the whole string */
    if (!str[str_len]) {
        /* zero-terminated */
        err_code = regexec(preg_p, str, 0, NULL, 0);
    } else {
        char *str_dup;

        str_dup = strndup(str, str_len);
        if (!str_dup) {
            rc = ly_err_new(err, LY_EMEM, 0, NULL, NULL, LY_EMEM_MSG);
            goto cleanup;
        }
        err_code = regexec(preg_p, str_dup, 0, NULL, 0);
        free(str_dup);
    }
# endif

    /* check the result */
    if (err_code == REG_NOMATCH) {
        rc = ly_err_new(err, LY_ENOT, 0, NULL, NULL, "Unsatisfied pattern - \"%.*s\" does not match \"%s\".",
                (int)str_len, str, pattern);
        goto cleanup;
    }

cleanup:
    if (!pat_comp) {
        ly_pat_free(preg_p, 1);
    }
    return rc;
#else
    (void)pat_comp;
    (void)pattern;
    (void)str;
    (void)str_len;

    return ly_err_new(err, LY_EINVAL, 0, NULL, NULL, "POSIX patterns are not supported.");
#endif
}

/**
 * @brief Match a string for an XML Schema pattern.
 *
 * @param[in] pat_comp Compiled pattern, may not be set if @p pattern.
 * @param[in] pattern Pattern, may not be set if @p pat_comp.
 * @param[in] str String to match.
 * @param[in] str_len Length of @p str.
 * @param[out] err Generated error, if any.
 * @return LY_ENOT if @p str does not match.
 * @return LY_ERR value on error.
 */
static LY_ERR
ly_pat_match_xmlschema(const void *pat_comp, const char *pattern, const char *str, size_t str_len, struct ly_err_item **err)
{
    LY_ERR rc = LY_SUCCESS;
    int r, match_opts = 0;
    pcre2_code *pcode = (void *)pat_comp;
    pcre2_match_data *match_data = NULL;

    if (!pat_comp) {
        /* compile pattern first */
        rc = ly_pat_compile_xmlschema(pattern, (void **)&pcode, err);
        LY_CHECK_GOTO(rc, cleanup);
    }

    /* match_data needs to be allocated each time because of possible multi-threaded evaluation */
    match_data = pcre2_match_data_create_from_pattern(pcode, NULL);
    if (!match_data) {
        rc = ly_err_new(err, LY_EMEM, 0, NULL, NULL, LY_EMEM_MSG);
        goto cleanup;
    }

    match_opts |= PCRE2_ANCHORED;
#ifdef PCRE2_ENDANCHORED
    /* PCRE2_ENDANCHORED was added in PCRE2 version 10.30 */
    match_opts |= PCRE2_ENDANCHORED;
#endif

    r = pcre2_match(pcode, (PCRE2_SPTR)str, str_len, 0, match_opts, match_data, NULL);
    pcre2_match_data_free(match_data);

    if ((r != PCRE2_ERROR_NOMATCH) && (r < 0)) {
        /* error */
        PCRE2_UCHAR pcre2_errmsg[LY_PCRE2_MSG_LIMIT] = {0};

        pcre2_get_error_message(r, pcre2_errmsg, LY_PCRE2_MSG_LIMIT);
        rc = ly_err_new(err, LY_ESYS, 0, NULL, NULL, "%s", (const char *)pcre2_errmsg);
        goto cleanup;
    }

    if (r == PCRE2_ERROR_NOMATCH) {
        rc = ly_err_new(err, LY_ENOT, 0, NULL, NULL, "Unsatisfied pattern - \"%.*s\" does not match \"%s\".",
                (int)str_len, str, pattern);
        goto cleanup;
    }

cleanup:
    if (!pat_comp) {
        ly_pat_free(pcode, 0);
    }
    return rc;
}

LY_ERR
ly_pat_match(const void *pat_comp, const char *pattern, ly_bool format, const char *str, size_t str_len,
        struct ly_err_item **err)
{
    if (format) {
        return ly_pat_match_posix(pat_comp, pattern, str, str_len, err);
    } else {
        return ly_pat_match_xmlschema(pat_comp, pattern, str, str_len, err);
    }
}

void
ly_pat_free(void *pat_comp, ly_bool format)
{
    if (!pat_comp) {
        return;
    }

    if (format) {
#ifdef HAVE_REGEX_H
        regfree(pat_comp);
        free(pat_comp);
#endif
    } else {
        pcre2_code_free(pat_comp);
    }
}

void *
ly_realloc(void *ptr, size_t size)
{
    void *new_mem;

    /* may cause double-free if NULL is returned */
    assert(size);

    new_mem = realloc(ptr, size);
    if (!new_mem && size) {
        free(ptr);
    }

    return new_mem;
}

char *
ly_strnchr(const char *s, int c, size_t len)
{
    for ( ; len && (*s != (char)c); ++s, --len) {}
    return len ? (char *)s : NULL;
}

int
ly_strncmp(const char *refstr, const char *str, size_t str_len)
{
    int rc = strncmp(refstr, str, str_len);

    if (!rc && (refstr[str_len] == '\0')) {
        return 0;
    } else {
        return rc ? rc : 1;
    }
}

LY_ERR
ly_strntou8(const char *nptr, size_t len, uint8_t *ret)
{
    uint8_t num = 0, dig;
    uint16_t dec_pow;

    if (len > 3) {
        /* overflow for sure */
        return LY_EDENIED;
    }

    dec_pow = 1;
    for ( ; len && isdigit(nptr[len - 1]); --len) {
        dig = nptr[len - 1] - 48;

        if (LY_OVERFLOW_MUL(UINT8_MAX, dig, dec_pow)) {
            return LY_EDENIED;
        }
        dig *= dec_pow;

        if (LY_OVERFLOW_ADD(UINT8_MAX, num, dig)) {
            return LY_EDENIED;
        }
        num += dig;

        dec_pow *= 10;
    }

    if (len) {
        return LY_EVALID;
    }
    *ret = num;
    return LY_SUCCESS;
}

LY_ERR
ly_value_prefix_next(const char *str_begin, const char *str_end, uint32_t *len, ly_bool *is_prefix, const char **str_next)
{
    const char *stop, *prefix;
    uint32_t bytes_read, c;
    ly_bool prefix_found;
    LY_ERR ret = LY_SUCCESS;

    assert(len && is_prefix && str_next);

#define IS_AT_END(PTR, STR_END) (STR_END ? PTR == STR_END : !(*PTR))

    *str_next = NULL;
    *is_prefix = 0;
    *len = 0;

    if (!str_begin || !(*str_begin) || (str_begin == str_end)) {
        return ret;
    }

    stop = str_begin;
    prefix = NULL;
    prefix_found = 0;

    do {
        /* look for the beginning of the YANG value */
        do {
            LY_CHECK_RET(ly_getutf8(&stop, &c, &bytes_read));
        } while (!is_xmlqnamestartchar(c) && !IS_AT_END(stop, str_end));

        if (IS_AT_END(stop, str_end)) {
            break;
        }

        /* maybe the prefix was found */
        prefix = stop - bytes_read;

        /* look for the the end of the prefix */
        do {
            LY_CHECK_RET(ly_getutf8(&stop, &c, &bytes_read));
        } while (is_xmlqnamechar(c) && !IS_AT_END(stop, str_end));

        prefix_found = c == ':' ? 1 : 0;

        /* if it wasn't the prefix, keep looking */
    } while (!IS_AT_END(stop, str_end) && !prefix_found);

    if ((str_begin == prefix) && prefix_found) {
        /* prefix found at the beginning of the input string */
        *is_prefix = 1;
        *str_next = IS_AT_END(stop, str_end) ? NULL : stop;
        *len = (stop - bytes_read) - str_begin;
    } else if ((str_begin != prefix) && (prefix_found)) {
        /* there is a some string before prefix */
        *str_next = prefix;
        *len = prefix - str_begin;
    } else {
        /* no prefix found */
        *len = stop - str_begin;
    }

#undef IS_AT_END

    return ret;
}

LY_ERR
ly_getutf8(const char **input, uint32_t *utf8_char, uint32_t *bytes_read)
{
    uint32_t c, aux, len;

    c = (*input)[0];

    if (!(c & 0x80)) {
        /* one byte character */
        len = 1;

        if ((c < 0x20) && (c != 0x9) && (c != 0xa) && (c != 0xd)) {
            goto error;
        }
    } else if ((c & 0xe0) == 0xc0) {
        /* two bytes character */
        len = 2;

        aux = (*input)[1];
        if ((aux & 0xc0) != 0x80) {
            goto error;
        }
        c = ((c & 0x1f) << 6) | (aux & 0x3f);

        if (c < 0x80) {
            goto error;
        }
    } else if ((c & 0xf0) == 0xe0) {
        /* three bytes character */
        len = 3;

        c &= 0x0f;
        for (uint64_t i = 1; i <= 2; i++) {
            aux = (*input)[i];
            if ((aux & 0xc0) != 0x80) {
                goto error;
            }

            c = (c << 6) | (aux & 0x3f);
        }

        if ((c < 0x800) || ((c > 0xd7ff) && (c < 0xe000)) || (c > 0xfffd)) {
            goto error;
        }
    } else if ((c & 0xf8) == 0xf0) {
        /* four bytes character */
        len = 4;

        c &= 0x07;
        for (uint64_t i = 1; i <= 3; i++) {
            aux = (*input)[i];
            if ((aux & 0xc0) != 0x80) {
                goto error;
            }

            c = (c << 6) | (aux & 0x3f);
        }

        if ((c < 0x1000) || (c > 0x10ffff)) {
            goto error;
        }
    } else {
        goto error;
    }

    (*utf8_char) = c;
    (*input) += len;
    if (bytes_read) {
        (*bytes_read) = len;
    }
    return LY_SUCCESS;

error:
    if (bytes_read) {
        (*bytes_read) = 0;
    }
    return LY_EINVAL;
}

/**
 * @brief Check whether an UTF-8 string is equal to a hex string after a bitwise and.
 *
 * (input & 0x[arg1][arg3][arg5]...) == 0x[arg2][arg4][arg6]...
 *
 * @param[in] input UTF-8 string.
 * @param[in] bytes Number of bytes to compare.
 * @param[in] ... 2x @p bytes number of bytes to perform bitwise and and equality operations.
 * @return Result of the operation.
 */
static int
ly_utf8_and_equal(const char *input, int bytes, ...)
{
    va_list ap;
    int i, and, byte;

    va_start(ap, bytes);
    for (i = 0; i < bytes; ++i) {
        and = va_arg(ap, int);
        byte = va_arg(ap, int);

        /* compare each byte */
        if (((uint8_t)input[i] & and) != (uint8_t)byte) {
            va_end(ap);
            return 0;
        }
    }
    va_end(ap);

    return 1;
}

/**
 * @brief Check whether an UTF-8 string is smaller than a hex string.
 *
 * input < 0x[arg1][arg2]...
 *
 * @param[in] input UTF-8 string.
 * @param[in] bytes Number of bytes to compare.
 * @param[in] ... @p bytes number of bytes to compare with.
 * @return Result of the operation.
 */
static int
ly_utf8_less(const char *input, int bytes, ...)
{
    va_list ap;
    int i, byte;

    va_start(ap, bytes);
    for (i = 0; i < bytes; ++i) {
        byte = va_arg(ap, int);

        /* compare until bytes differ */
        if ((uint8_t)input[i] > (uint8_t)byte) {
            va_end(ap);
            return 0;
        } else if ((uint8_t)input[i] < (uint8_t)byte) {
            va_end(ap);
            return 1;
        }
    }
    va_end(ap);

    /* equals */
    return 0;
}

/**
 * @brief Check whether an UTF-8 string is greater than a hex string.
 *
 * input > 0x[arg1][arg2]...
 *
 * @param[in] input UTF-8 string.
 * @param[in] bytes Number of bytes to compare.
 * @param[in] ... @p bytes number of bytes to compare with.
 * @return Result of the operation.
 */
static int
ly_utf8_greater(const char *input, int bytes, ...)
{
    va_list ap;
    int i, byte;

    va_start(ap, bytes);
    for (i = 0; i < bytes; ++i) {
        byte = va_arg(ap, int);

        /* compare until bytes differ */
        if ((uint8_t)input[i] > (uint8_t)byte) {
            va_end(ap);
            return 1;
        } else if ((uint8_t)input[i] < (uint8_t)byte) {
            va_end(ap);
            return 0;
        }
    }
    va_end(ap);

    /* equals */
    return 0;
}

LY_ERR
ly_checkutf8(const char *input, uint32_t in_len, uint32_t *utf8_len)
{
    uint32_t len;

    if (!(input[0] & 0x80)) {
        /* one byte character */
        len = 1;

        if (ly_utf8_less(input, 1, 0x20) && (input[0] != 0x9) && (input[0] != 0xa) && (input[0] != 0xd)) {
            /* invalid control characters */
            return LY_EINVAL;
        }
    } else if (((input[0] & 0xe0) == 0xc0) && (in_len > 1)) {
        /* two bytes character */
        len = 2;

        /* (input < 0xC280) || (input > 0xDFBF) || ((input & 0xE0C0) != 0xC080) */
        if (ly_utf8_less(input, 2, 0xC2, 0x80) || ly_utf8_greater(input, 2, 0xDF, 0xBF) ||
                !ly_utf8_and_equal(input, 2, 0xE0, 0xC0, 0xC0, 0x80)) {
            return LY_EINVAL;
        }
    } else if (((input[0] & 0xf0) == 0xe0) && (in_len > 2)) {
        /* three bytes character */
        len = 3;

        /* (input >= 0xEDA080) && (input <= 0xEDBFBF) */
        if (!ly_utf8_less(input, 3, 0xED, 0xA0, 0x80) && !ly_utf8_greater(input, 3, 0xED, 0xBF, 0xBF)) {
            /* reject UTF-16 surrogates */
            return LY_EINVAL;
        }

        /* (input < 0xE0A080) || (input > 0xEFBFBF) || ((input & 0xF0C0C0) != 0xE08080) */
        if (ly_utf8_less(input, 3, 0xE0, 0xA0, 0x80) || ly_utf8_greater(input, 3, 0xEF, 0xBF, 0xBF) ||
                !ly_utf8_and_equal(input, 3, 0xF0, 0xE0, 0xC0, 0x80, 0xC0, 0x80)) {
            return LY_EINVAL;
        }
    } else if (((input[0] & 0xf8) == 0xf0) && (in_len > 3)) {
        /* four bytes character */
        len = 4;

        /* (input < 0xF0908080) || (input > 0xF48FBFBF) || ((input & 0xF8C0C0C0) != 0xF0808080) */
        if (ly_utf8_less(input, 4, 0xF0, 0x90, 0x80, 0x80) || ly_utf8_greater(input, 4, 0xF4, 0x8F, 0xBF, 0xBF) ||
                !ly_utf8_and_equal(input, 4, 0xF8, 0xF0, 0xC0, 0x80, 0xC0, 0x80, 0xC0, 0x80)) {
            return LY_EINVAL;
        }
    } else {
        return LY_EINVAL;
    }

    *utf8_len = len;
    return LY_SUCCESS;
}

LY_ERR
ly_pututf8(char *dst, uint32_t value, uint32_t *bytes_written)
{
    if (value < 0x80) {
        /* one byte character */
        if ((value < 0x20) &&
                (value != 0x09) &&
                (value != 0x0a) &&
                (value != 0x0d)) {
            /* valid UTF8 but not YANG string character */
            return LY_EINVAL;
        }

        dst[0] = value;
        (*bytes_written) = 1;
    } else if (value < 0x800) {
        /* two bytes character */
        dst[0] = 0xc0 | (value >> 6);
        dst[1] = 0x80 | (value & 0x3f);
        (*bytes_written) = 2;
    } else if (value < 0xfffe) {
        /* three bytes character */
        if (((value & 0xf800) == 0xd800) ||
                ((value >= 0xfdd0) && (value <= 0xfdef))) {
            /* exclude surrogate blocks %xD800-DFFF */
            /* exclude noncharacters %xFDD0-FDEF */
            return LY_EINVAL;
        }

        dst[0] = 0xe0 | (value >> 12);
        dst[1] = 0x80 | ((value >> 6) & 0x3f);
        dst[2] = 0x80 | (value & 0x3f);

        (*bytes_written) = 3;
    } else if (value < 0x10fffe) {
        if ((value & 0xffe) == 0xffe) {
            /* exclude noncharacters %xFFFE-FFFF, %x1FFFE-1FFFF, %x2FFFE-2FFFF, %x3FFFE-3FFFF, %x4FFFE-4FFFF,
             * %x5FFFE-5FFFF, %x6FFFE-6FFFF, %x7FFFE-7FFFF, %x8FFFE-8FFFF, %x9FFFE-9FFFF, %xAFFFE-AFFFF,
             * %xBFFFE-BFFFF, %xCFFFE-CFFFF, %xDFFFE-DFFFF, %xEFFFE-EFFFF, %xFFFFE-FFFFF, %x10FFFE-10FFFF */
            return LY_EINVAL;
        }
        /* four bytes character */
        dst[0] = 0xf0 | (value >> 18);
        dst[1] = 0x80 | ((value >> 12) & 0x3f);
        dst[2] = 0x80 | ((value >> 6) & 0x3f);
        dst[3] = 0x80 | (value & 0x3f);

        (*bytes_written) = 4;
    } else {
        return LY_EINVAL;
    }
    return LY_SUCCESS;
}

/**
 * @brief Static table of the UTF8 characters lengths according to their first byte.
 */
static const unsigned char utf8_char_length_table[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
};

uint32_t
ly_utf8len(const char *str, uint32_t bytes)
{
    uint32_t len = 0;
    const char *ptr = str;

    while (((uint32_t)(ptr - str) < bytes) && *ptr) {
        ++len;
        ptr += utf8_char_length_table[((unsigned char)(*ptr))];
    }
    return len;
}

int
LY_VCODE_INSTREXP_len(const char *str)
{
    int len = 0;

    if (!str) {
        return len;
    } else if (!str[0]) {
        return 1;
    }
    for (len = 1; len < LY_VCODE_INSTREXP_MAXLEN && str[len]; ++len) {}
    return len;
}

#ifdef HAVE_MMAP
LY_ERR
ly_mmap(struct ly_ctx *ctx, int fd, size_t *length, void **addr)
{
    struct stat sb;
    long pagesize;
    size_t m;

    assert(length);
    assert(addr);
    assert(fd >= 0);

    if (fstat(fd, &sb) == -1) {
        LOGERR(ctx, LY_ESYS, "Failed to stat the file descriptor (%s) for the mmap().", strerror(errno));
        return LY_ESYS;
    }
    if (!S_ISREG(sb.st_mode)) {
        LOGERR(ctx, LY_EINVAL, "File to mmap() is not a regular file.");
        return LY_ESYS;
    }
    if (!sb.st_size) {
        *addr = NULL;
        return LY_SUCCESS;
    }
    pagesize = sysconf(_SC_PAGESIZE);

    m = sb.st_size % pagesize;
    if (m && (pagesize - m >= 1)) {
        /* there will be enough space (at least 1 byte) after the file content mapping to provide zeroed NULL-termination byte */
        *length = sb.st_size + 1;
        *addr = mmap(NULL, *length, PROT_READ, MAP_PRIVATE, fd, 0);
    } else {
        /* there will not be enough bytes after the file content mapping for the additional bytes and some of them
         * would overflow into another page that would not be zerroed and any access into it would generate SIGBUS.
         * Therefore we have to do the following hack with double mapping. First, the required number of bytes
         * (including the additinal bytes) is required as anonymous and thus they will be really provided (actually more
         * because of using whole pages) and also initialized by zeros. Then, the file is mapped to the same address
         * where the anonymous mapping starts. */
        *length = sb.st_size + pagesize;
        *addr = mmap(NULL, *length, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        *addr = mmap(*addr, sb.st_size, PROT_READ, MAP_PRIVATE | MAP_FIXED, fd, 0);
    }
    if (*addr == MAP_FAILED) {
        LOGERR(ctx, LY_ESYS, "mmap() failed (%s).", strerror(errno));
        return LY_ESYS;
    }

    return LY_SUCCESS;
}

LY_ERR
ly_munmap(void *addr, size_t length)
{
    if (munmap(addr, length)) {
        return LY_ESYS;
    }
    return LY_SUCCESS;
}

#else

LY_ERR
ly_mmap(struct ly_ctx *ctx, int fd, size_t *length, void **addr)
{
    struct stat sb;
    size_t m;

    assert(length);
    assert(addr);
    assert(fd >= 0);

#if _WIN32
    if (_setmode(fd, _O_BINARY) == -1) {
        LOGERR(ctx, LY_ESYS, "Failed to switch the file descriptor to binary mode.", strerror(errno));
        return LY_ESYS;
    }
#endif

    if (fstat(fd, &sb) == -1) {
        LOGERR(ctx, LY_ESYS, "Failed to stat the file descriptor (%s) for the mmap().", strerror(errno));
        return LY_ESYS;
    }
    if (!S_ISREG(sb.st_mode)) {
        LOGERR(ctx, LY_EINVAL, "File to mmap() is not a regular file.");
        return LY_ESYS;
    }
    if (!sb.st_size) {
        *addr = NULL;
        return LY_SUCCESS;
    }
    /* On Windows, the mman-win32 mmap() emulation uses CreateFileMapping and MapViewOfFile, and these functions
     * do not allow mapping more than "length of file" bytes for PROT_READ. Remapping existing mappings is not allowed, either.
     * At that point the path of least resistance is just reading the file in as-is. */
    m = sb.st_size + 1;
    char *buf = calloc(m, 1);

    if (!buf) {
        LOGERR(ctx, LY_ESYS, "ly_mmap: malloc() failed (%s).", strerror(errno));
    }
    *addr = buf;
    *length = m;

    lseek(fd, 0, SEEK_SET);
    ssize_t to_read = m - 1;

    while (to_read > 0) {
        ssize_t n = read(fd, buf, to_read);

        if (n == 0) {
            return LY_SUCCESS;
        } else if (n < 0) {
            if (errno == EINTR) {
                continue; // can I get this on Windows?
            }
            LOGERR(ctx, LY_ESYS, "ly_mmap: read() failed (%s).", strerror(errno));
        }
        to_read -= n;
        buf += n;
    }
    return LY_SUCCESS;
}

LY_ERR
ly_munmap(void *addr, size_t length)
{
    (void)length;
    free(addr);
    return LY_SUCCESS;
}

#endif

LY_ERR
ly_strcat(char **dest, const char *format, ...)
{
    va_list fp;
    char *addition = NULL;
    size_t len;

    va_start(fp, format);
    len = vasprintf(&addition, format, fp);
    len += (*dest ? strlen(*dest) : 0) + 1;

    if (*dest) {
        *dest = ly_realloc(*dest, len);
        if (!*dest) {
            va_end(fp);
            return LY_EMEM;
        }
        *dest = strcat(*dest, addition);
        free(addition);
    } else {
        *dest = addition;
    }

    va_end(fp);
    return LY_SUCCESS;
}

LY_ERR
ly_parse_int(const char *val_str, uint32_t val_len, int64_t min, int64_t max, int base, int64_t *ret)
{
    LY_ERR rc = LY_SUCCESS;
    char *ptr, *str;
    int64_t i;

    LY_CHECK_ARG_RET(NULL, val_str, val_str[0], val_len, LY_EINVAL);

    /* duplicate the value */
    str = strndup(val_str, val_len);
    LY_CHECK_RET(!str, LY_EMEM);

    /* parse the value to avoid accessing following bytes */
    errno = 0;
    i = strtoll(str, &ptr, base);
    if (errno || (ptr == str)) {
        /* invalid string */
        rc = LY_EVALID;
    } else if ((i < min) || (i > max)) {
        /* invalid number */
        rc = LY_EDENIED;
    } else if (*ptr) {
        while (isspace(*ptr)) {
            ++ptr;
        }
        if (*ptr) {
            /* invalid characters after some number */
            rc = LY_EVALID;
        }
    }

    /* cleanup */
    free(str);
    if (!rc) {
        *ret = i;
    }
    return rc;
}

LY_ERR
ly_parse_uint(const char *val_str, uint32_t val_len, uint64_t max, int base, uint64_t *ret)
{
    LY_ERR rc = LY_SUCCESS;
    char *ptr, *str;
    uint64_t u;

    LY_CHECK_ARG_RET(NULL, val_str, val_str[0], val_len, LY_EINVAL);

    /* duplicate the value to avoid accessing following bytes */
    str = strndup(val_str, val_len);
    LY_CHECK_RET(!str, LY_EMEM);

    /* parse the value */
    errno = 0;
    u = strtoull(str, &ptr, base);
    if (errno || (ptr == str)) {
        /* invalid string */
        rc = LY_EVALID;
    } else if ((u > max) || (u && (str[0] == '-'))) {
        /* invalid number */
        rc = LY_EDENIED;
    } else if (*ptr) {
        while (isspace(*ptr)) {
            ++ptr;
        }
        if (*ptr) {
            /* invalid characters after some number */
            rc = LY_EVALID;
        }
    }

    /* cleanup */
    free(str);
    if (!rc) {
        *ret = u;
    }
    return rc;
}

/**
 * @brief Parse an identifier.
 *
 * ;; An identifier MUST NOT start with (('X'|'x') ('M'|'m') ('L'|'l'))
 * identifier          = (ALPHA / "_")
 *                       *(ALPHA / DIGIT / "_" / "-" / ".")
 *
 * @param[in,out] id Identifier to parse. When returned, it points to the first character which is not part of the identifier.
 * @return LY_ERR value: LY_SUCCESS or LY_EINVAL in case of invalid starting character.
 */
static LY_ERR
lys_parse_id(const char **id)
{
    assert(id && *id);

    if (!is_yangidentstartchar(**id)) {
        return LY_EINVAL;
    }
    ++(*id);

    while (is_yangidentchar(**id)) {
        ++(*id);
    }
    return LY_SUCCESS;
}

LY_ERR
ly_parse_nodeid(const char **id, const char **prefix, uint32_t *prefix_len, const char **name, uint32_t *name_len)
{
    assert(id && *id);
    assert(prefix && prefix_len);
    assert(name && name_len);

    *prefix = *id;
    *prefix_len = 0;
    *name = NULL;
    *name_len = 0;

    LY_CHECK_RET(lys_parse_id(id));
    if (**id == ':') {
        /* there is prefix */
        *prefix_len = *id - *prefix;
        ++(*id);
        *name = *id;

        LY_CHECK_RET(lys_parse_id(id));
        *name_len = *id - *name;
    } else {
        /* there is no prefix, so what we have as prefix now is actually the name */
        *name = *prefix;
        *name_len = *id - *name;
        *prefix = NULL;
    }

    return LY_SUCCESS;
}

LY_ERR
ly_parse_instance_predicate(const char **pred, uint32_t limit, LYD_FORMAT format, const char **prefix, uint32_t *prefix_len,
        const char **id, uint32_t *id_len, const char **value, uint32_t *value_len, const char **errmsg)
{
    LY_ERR ret = LY_EVALID;
    const char *in = *pred;
    uint32_t offset = 1;
    uint8_t expr = 0; /* 0 - position predicate; 1 - leaf-list-predicate; 2 - key-predicate */
    char quot;

    assert(in[0] == '[');

    *prefix = *id = *value = NULL;
    *prefix_len = *id_len = *value_len = 0;

    /* leading *WSP */
    for ( ; isspace(in[offset]); offset++) {}

    if (isdigit(in[offset])) {
        /* pos: "[" *WSP positive-integer-value *WSP "]" */
        if (in[offset] == '0') {
            /* zero */
            *errmsg = "The position predicate cannot be zero.";
            goto error;
        }

        /* positive-integer-value */
        *value = &in[offset++];
        for ( ; isdigit(in[offset]); offset++) {}
        *value_len = &in[offset] - *value;

    } else if (in[offset] == '.') {
        /* leaf-list-predicate: "[" *WSP "." *WSP "=" *WSP quoted-string *WSP "]" */
        *id = &in[offset];
        *id_len = 1;
        offset++;
        expr = 1;
    } else if (in[offset] == '-') {
        /* typically negative value */
        *errmsg = "Invalid instance predicate format (negative position or invalid node-identifier).";
        goto error;
    } else {
        /* key-predicate: "[" *WSP node-identifier *WSP "=" *WSP quoted-string *WSP "]" */
        in = &in[offset];
        if (ly_parse_nodeid(&in, prefix, prefix_len, id, id_len)) {
            *errmsg = "Invalid node-identifier.";
            goto error;
        }
        if ((format == LYD_XML) && !(*prefix)) {
            /* all node names MUST be qualified with explicit namespace prefix */
            *errmsg = "Missing prefix of a node name.";
            goto error;
        }
        offset = in - *pred;
        in = *pred;
        expr = 2;
    }

    if (expr) {
        /*  *WSP "=" *WSP quoted-string *WSP "]" */
        for ( ; isspace(in[offset]); offset++) {}

        if (in[offset] != '=') {
            if (expr == 1) {
                *errmsg = "Unexpected character instead of \'=\' in leaf-list-predicate.";
            } else { /* 2 */
                *errmsg = "Unexpected character instead of \'=\' in key-predicate.";
            }
            goto error;
        }
        offset++;
        for ( ; isspace(in[offset]); offset++) {}

        /* quoted-string */
        quot = in[offset++];
        if ((quot != '\'') && (quot != '\"')) {
            *errmsg = "String value is not quoted.";
            goto error;
        }
        *value = &in[offset];
        for ( ; offset < limit && (in[offset] != quot || (offset && in[offset - 1] == '\\')); offset++) {}
        if (in[offset] == quot) {
            *value_len = &in[offset] - *value;
            offset++;
        } else {
            *errmsg = "Value is not terminated quoted-string.";
            goto error;
        }
    }

    /* *WSP "]" */
    for ( ; isspace(in[offset]); offset++) {}
    if (in[offset] != ']') {
        if (expr == 0) {
            *errmsg = "Predicate (pos) is not terminated by \']\' character.";
        } else if (expr == 1) {
            *errmsg = "Predicate (leaf-list-predicate) is not terminated by \']\' character.";
        } else { /* 2 */
            *errmsg = "Predicate (key-predicate) is not terminated by \']\' character.";
        }
        goto error;
    }
    offset++;

    if (offset <= limit) {
        *pred = &in[offset];
        return LY_SUCCESS;
    }

    /* we read after the limit */
    *errmsg = "Predicate is incomplete.";
    *prefix = *id = *value = NULL;
    *prefix_len = *id_len = *value_len = 0;
    offset = limit;
    ret = LY_EINVAL;

error:
    *pred = &in[offset];
    return ret;
}
