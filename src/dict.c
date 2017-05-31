/**
 * @file dict.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang dictionary for storing strings
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

#include "common.h"
#include "context.h"
#include "dict_private.h"

void
lydict_init(struct dict_table *dict)
{
    if (!dict) {
        ly_errno = LY_EINVAL;
        return;
    }

    dict->hash_mask = DICT_SIZE - 1;
    pthread_mutex_init(&dict->lock, NULL);
}

void
lydict_clean(struct dict_table *dict)
{
    int i;
    struct dict_rec *chain, *rec;

    if (!dict) {
        ly_errno = LY_EINVAL;
        return;
    }

    for (i = 0; i < DICT_SIZE; i++) {
        rec = &dict->recs[i];
        chain = rec->next;

        free(rec->value);
        while (chain) {
            rec = chain;
            chain = rec->next;

            free(rec->value);
            free(rec);
        }
    }

    pthread_mutex_destroy(&dict->lock);
}

/*
 * Bob Jenkin's one-at-a-time hash
 * http://www.burtleburtle.net/bob/hash/doobs.html
 *
 * Spooky hash is faster, but it works only for little endian architectures.
 */
static uint32_t
dict_hash(const char *key, size_t len)
{
    uint32_t hash, i;

    for (hash = i = 0; i < len; ++i) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

/*
 * Usage:
 * - init hash to 0
 * - repeatedly call dict_hash_multi(), provide hash from the last call
 * - call dict_hash_multi() with key_part = NULL to finish the hash
 */
uint32_t
dict_hash_multi(uint32_t hash, const char *key_part, size_t len)
{
    uint32_t i;

    if (key_part) {
        for (i = 0; i < len; ++i) {
            hash += key_part[i];
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }
    } else {
        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);
    }

    return hash;
}

API void
lydict_remove(struct ly_ctx *ctx, const char *value)
{
    size_t len;
    uint32_t index;
    struct dict_rec *record, *prev = NULL;

    if (!value || !ctx) {
        return;
    }

    len = strlen(value);

    pthread_mutex_lock(&ctx->dict.lock);

    if (!ctx->dict.used) {
        pthread_mutex_unlock(&ctx->dict.lock);
        return;
    }

    index = dict_hash(value, len) & ctx->dict.hash_mask;
    record = &ctx->dict.recs[index];

    while (record && record->value != value) {
        prev = record;
        record = record->next;
    }

    if (!record) {
        /* record not found */
        pthread_mutex_unlock(&ctx->dict.lock);
        return;
    }

    record->refcount--;
    if (!record->refcount) {
        free(record->value);
        if (record->next) {
            if (prev) {
                /* change in dynamically allocated chain */
                prev->next = record->next;
                free(record);
            } else {
                /* move dynamically allocated record into the static array */
                prev = record->next;    /* temporary storage */
                memcpy(record, record->next, sizeof *record);
                free(prev);
            }
        } else if (prev) {
            /* removing last record from the dynamically allocated chain */
            prev->next = NULL;
            free(record);
        } else {
            /* clean the static record content */
            memset(record, 0, sizeof *record);
        }
        ctx->dict.used--;
    }

    pthread_mutex_unlock(&ctx->dict.lock);
}

static char *
dict_insert(struct ly_ctx *ctx, char *value, size_t len, int zerocopy)
{
    uint32_t index;
    int match = 0;
    struct dict_rec *record, *new;

    index = dict_hash(value, len) & ctx->dict.hash_mask;
    record = &ctx->dict.recs[index];

    if (!record->value) {
        /* first record with this hash */
        if (zerocopy) {
            record->value = value;
        } else {
            record->value = malloc((len + 1) * sizeof *record->value);
            LY_CHECK_ERR_RETURN(!record->value, LOGMEM, NULL);
            memcpy(record->value, value, len);
            record->value[len] = '\0';
        }
        record->refcount = 1;
        if (len > DICT_REC_MAXLEN) {
            record->len = 0;
        } else {
            record->len = len;
        }
        record->next = NULL;

        ctx->dict.used++;

        LOGDBG(LY_LDGDICT, "inserting \"%s\"", record->value);
        return record->value;
    }

    /* collision, search if the value is already in dict */
    while (record) {
        if (record->len) {
            /* for strings shorter than DICT_REC_MAXLEN we are able to speed up
             * recognition of varying strings according to their lengths, and
             * for strings with the same length it is safe to use faster memcmp()
             * instead of strncmp() */
            if ((record->len == len) && !memcmp(value, record->value, len)) {
                match = 1;
            }
        } else {
            if (!strncmp(value, record->value, len) && record->value[len] == '\0') {
                match = 1;
            }
        }
        if (match) {
            /* record found */
            if (record->refcount == DICT_REC_MAXCOUNT) {
                LOGWRN("Refcount overflow detected, duplicating dictionary record");
                break;
            }
            record->refcount++;

            if (zerocopy) {
                free(value);
            }

            LOGDBG(LY_LDGDICT, "inserting (refcount) \"%s\"", record->value);
            return record->value;
        }

        if (!record->next) {
            /* not present, add as a new record in chain */
            break;
        }

        record = record->next;
    }

    /* create new record and add it behind the last record */
    new = malloc(sizeof *record);
    LY_CHECK_ERR_RETURN(!new, LOGMEM, NULL);
    if (zerocopy) {
        new->value = value;
    } else {
        new->value = malloc((len + 1) * sizeof *record->value);
        LY_CHECK_ERR_RETURN(!new->value, LOGMEM; free(new), NULL);
        memcpy(new->value, value, len);
        new->value[len] = '\0';
    }
    new->refcount = 1;
    if (len > DICT_REC_MAXLEN) {
        new->len = 0;
    } else {
        new->len = len;
    }
    new->next = record->next; /* in case of refcount overflow, we are not at the end of chain */
    record->next = new;

    ctx->dict.used++;

    LOGDBG(LY_LDGDICT, "inserting \"%s\" with collision ", new->value);
    return new->value;
}

API const char *
lydict_insert(struct ly_ctx *ctx, const char *value, size_t len)
{
    const char *result;

    if (value && !len) {
        len = strlen(value);
    }

    if (!value) {
        return NULL;
    }

    pthread_mutex_lock(&ctx->dict.lock);
    result = dict_insert(ctx, (char *)value, len, 0);
    pthread_mutex_unlock(&ctx->dict.lock);

    return result;
}

API const char *
lydict_insert_zc(struct ly_ctx *ctx, char *value)
{
    const char *result;

    if (!value) {
        return NULL;
    }

    pthread_mutex_lock(&ctx->dict.lock);
    result = dict_insert(ctx, value, strlen(value), 1);
    pthread_mutex_unlock(&ctx->dict.lock);

    return result;
}
