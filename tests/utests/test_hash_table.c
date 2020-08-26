/*
 * @file hash_table.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from hash_table.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "hash_table.h"

struct ht_rec *lyht_get_rec(unsigned char *recs, uint16_t rec_size, uint32_t idx);

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

static void
logger(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) level; /* unused */
    (void) path; /* unused */

    strncpy(logbuf, msg, BUFSIZE - 1);
}

static int
logger_setup(void **state)
{
    (void) state; /* unused */
#if ENABLE_LOGGER_CHECKING
    ly_set_log_clb(logger, 0);
#endif
    return 0;
}

void
logbuf_clean(void)
{
    logbuf[0] = '\0';
}

#if ENABLE_LOGGER_CHECKING
#   define logbuf_assert(str) assert_string_equal(logbuf, str)
#else
#   define logbuf_assert(str)
#endif

static void
test_invalid_arguments(void **state)
{
    (void) state; /* unused */
    struct ly_ctx *ctx;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));

    assert_null(lydict_insert(NULL, NULL, 0));
    logbuf_assert("Invalid argument ctx (lydict_insert()).");

    assert_null(lydict_insert_zc(NULL, NULL));
    logbuf_assert("Invalid argument ctx (lydict_insert_zc()).");
    assert_null(lydict_insert_zc(ctx, NULL));
    logbuf_assert("Invalid argument value (lydict_insert_zc()).");

    ly_ctx_destroy(ctx, NULL);
}

static void
test_dict_hit(void **state)
{
    (void) state; /* unused */

    const char *str1, *str2;
    struct ly_ctx *ctx;

    assert_int_equal(LY_SUCCESS, ly_ctx_new(NULL, 0, &ctx));

    /* insert 2 strings, one of them repeatedly */
    str1 = lydict_insert(ctx, "test1", 0);
    assert_non_null(str1);
    /* via zerocopy we have to get the same pointer as provided */
    assert_non_null(str2 = strdup("test2"));
    assert_true(str2 == lydict_insert_zc(ctx, (char *)str2));
    /* here we get the same pointer as in case the string was inserted first time */
    str2 = lydict_insert(ctx, "test1", 0);
    assert_non_null(str2);
    assert_ptr_equal(str1, str2);

    /* remove strings, but the repeatedly inserted only once */
    lydict_remove(ctx, "test1");
    lydict_remove(ctx, "test2");

    /* destroy dictionary - should raise warning about data presence */
    ly_ctx_destroy(ctx, NULL);
    logbuf_assert("String \"test1\" not freed from the dictionary, refcount 1");

#ifndef NDEBUG
    /* cleanup */
    free((char*)str1);
#endif
}

static uint8_t
ht_equal_clb(void *val1, void *val2, uint8_t mod, void *cb_data)
{
    int *v1, *v2;
    (void)mod;
    (void)cb_data;

    v1 = (int *)val1;
    v2 = (int *)val2;

    return *v1 == *v2;
}

static void
test_ht_basic(void **state)
{
    (void) state; /* unused */

    uint32_t i;
    struct hash_table *ht;

    assert_non_null(ht = lyht_new(8, sizeof(int), ht_equal_clb, NULL, 0));

    i = 2;
    assert_int_equal(LY_ENOTFOUND, lyht_find(ht, &i, i, NULL));
    assert_int_equal(LY_SUCCESS, lyht_insert(ht, &i, i, NULL));
    assert_int_equal(LY_SUCCESS, lyht_find(ht, &i, i, NULL));
    assert_int_equal(LY_SUCCESS, lyht_remove(ht, &i, i));
    assert_int_equal(LY_ENOTFOUND, lyht_find(ht, &i, i, NULL));
    assert_int_equal(LY_ENOTFOUND, lyht_remove(ht, &i, i));
    logbuf_assert("Invalid argument hash (lyht_remove()).");

    lyht_free(ht);
}

static void
test_ht_resize(void **state)
{
    (void) state; /* unused */

    uint32_t i;
    struct ht_rec *rec;
    struct hash_table *ht;

    assert_non_null(ht = lyht_new(8, sizeof(int), ht_equal_clb, NULL, 1));
    assert_int_equal(8, ht->size);

    /* insert records into indexes 2-7 */
    for (i = 2; i < 8; ++i) {
        assert_int_equal(LY_SUCCESS, lyht_insert(ht, &i, i, NULL));
    }
    /* check that table resized */
    assert_int_equal(16, ht->size);

    /* check expected content of the table */
    for (i = 0; i < 16; ++i) {
        if (i >=2 && i < 8) {
            /* inserted data on indexes 2-7 */
            rec = lyht_get_rec(ht->recs, ht->rec_size, i);
            assert_int_equal(1, rec->hits);
            assert_int_equal(i, rec->hash);
        } else {
            /* nothing otherwise */
            rec = lyht_get_rec(ht->recs, ht->rec_size, i);
            assert_int_equal(0, rec->hits);
        }
    }

    /* removing not present data should fail */
    for (i = 0; i < 2; ++i) {
        logbuf_clean();
        assert_int_equal(LY_ENOTFOUND, lyht_remove(ht, &i, i));
        logbuf_assert("Invalid argument hash (lyht_remove()).");
    }
    /* removing present data, resize should happened
     * when we are below 25% of the table filled, so with 3 records left */
    for (; i < 5; ++i) {
        assert_int_equal(LY_SUCCESS, lyht_remove(ht, &i, i));
    }
    assert_int_equal(8, ht->size);

    /* remove the rest */
    for (; i < 8; ++i) {
        assert_int_equal(LY_SUCCESS, lyht_remove(ht, &i, i));
    }

    for (i = 0; i < 8; ++i) {
        assert_int_equal(LY_ENOTFOUND, lyht_find(ht, &i, i, NULL));
    }

    /* cleanup */
    lyht_free(ht);
}


static void
test_ht_collisions(void **state)
{
    (void) state; /* unused */
#define GET_REC_INT(rec) (*((uint32_t *)&(rec)->val))

    uint32_t i;
    struct ht_rec *rec;
    struct hash_table *ht;

    assert_non_null(ht = lyht_new(8, sizeof(int), ht_equal_clb, NULL, 1));

    for (i = 2; i < 6; ++i) {
        assert_int_equal(lyht_insert(ht, &i, 2, NULL), 0);
    }

    /* check all records */
    for (i = 0; i < 2; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }
    rec = lyht_get_rec(ht->recs, ht->rec_size, i);
    assert_int_equal(rec->hits, 4);
    assert_int_equal(GET_REC_INT(rec), i);
    ++i;
    for (; i < 6; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 1);
        assert_int_equal(GET_REC_INT(rec), i);
    }
    for (; i < 8; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }

    i = 4;
    assert_int_equal(lyht_remove(ht, &i, 2), 0);

    rec = lyht_get_rec(ht->recs, ht->rec_size, i);
    assert_int_equal(rec->hits, -1);

    i = 2;
    assert_int_equal(lyht_remove(ht, &i, 2), 0);

    /* check all records */
    for (i = 0; i < 2; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }
    rec = lyht_get_rec(ht->recs, ht->rec_size, i);
    assert_int_equal(rec->hits, 2);
    assert_int_equal(GET_REC_INT(rec), 5);
    ++i;
    rec = lyht_get_rec(ht->recs, ht->rec_size, i);
    assert_int_equal(rec->hits, 1);
    assert_int_equal(GET_REC_INT(rec), 3);
    ++i;
    for (; i < 6; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, -1);
    }
    for (; i < 8; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }

    for (i = 0; i < 3; ++i) {
        assert_int_equal(lyht_find(ht, &i, 2, NULL), LY_ENOTFOUND);
    }
    assert_int_equal(lyht_find(ht, &i, 2, NULL), LY_SUCCESS);
    ++i;
    assert_int_equal(lyht_find(ht, &i, 2, NULL), LY_ENOTFOUND);
    ++i;
    assert_int_equal(lyht_find(ht, &i, 2, NULL), LY_SUCCESS);
    ++i;
    for (; i < 8; ++i) {
        assert_int_equal(lyht_find(ht, &i, 2, NULL), LY_ENOTFOUND);
    }

    i = 3;
    assert_int_equal(lyht_remove(ht, &i, 2), 0);
    i = 5;
    assert_int_equal(lyht_remove(ht, &i, 2), 0);

    /* check all records */
    for (i = 0; i < 2; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }
    for (; i < 6; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, -1);
    }
    for (; i < 8; ++i) {
        rec = lyht_get_rec(ht->recs, ht->rec_size, i);
        assert_int_equal(rec->hits, 0);
    }

    lyht_free(ht);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_invalid_arguments, logger_setup),
        cmocka_unit_test_setup(test_dict_hit, logger_setup),
        cmocka_unit_test_setup(test_ht_basic, logger_setup),
        cmocka_unit_test_setup(test_ht_resize, logger_setup),
        cmocka_unit_test_setup(test_ht_collisions, logger_setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
