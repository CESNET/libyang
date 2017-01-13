/**
 * \file test_extensions.c
 * \author Radek Krejci <rkrejci@cesnet.cz>
 * \brief libyang tests - extensions
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cmocka.h>

#include "../../src/libyang.h"
#include "../config.h"

#define SCHEMA_FOLDER_YIN TESTS_DIR"/schema/yin/files"
#define SCHEMA_FOLDER_YANG TESTS_DIR"/schema/yang/files"

struct state {
    struct ly_ctx *ctx;
    int fd;
    char *str1;
    char *str2;
};

static int
setup_ctx(void **state, const char *searchdir)
{
    struct state *st;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(searchdir);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    st->fd = -1;

    return 0;

error:
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return -1;
}

static int
setup_ctx_yin(void **state)
{
    return setup_ctx(state, SCHEMA_FOLDER_YIN);
}

static int
setup_ctx_yang(void **state)
{
    return setup_ctx(state, SCHEMA_FOLDER_YANG);
}

static int
teardown_ctx(void **state)
{
    struct state *st = (*state);

    ly_ctx_destroy(st->ctx, NULL);
    if (st->fd >= 0) {
        close(st->fd);
    }
    free(st->str1);
    free(st->str2);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_fullset_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    struct stat s;

    mod = lys_parse_path(st->ctx, SCHEMA_FOLDER_YIN"/ext.yin", LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL);
    assert_ptr_not_equal(st->str1, NULL);

    st->fd = open(SCHEMA_FOLDER_YIN"/ext.yin", O_RDONLY);
    fstat(st->fd, &s);
    st->str2 = malloc(s.st_size + 1);
    assert_ptr_not_equal(st->str2, NULL);
    assert_int_equal(read(st->fd, st->str2, s.st_size), s.st_size);
    st->str2[s.st_size] = '\0';

    assert_string_equal(st->str1, st->str2);
}

static void
test_container_sub_yin(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    const char *yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    "<module name=\"ext\"\n"
                    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
                    "        xmlns:x=\"urn:ext\"\n"
                    "        xmlns:e=\"urn:ext-def\">\n"
                    "  <namespace uri=\"urn:ext\"/>\n"
                    "  <prefix value=\"x\"/>\n"
                    "  <import module=\"ext-def\">\n    <prefix value=\"e\"/>\n  </import>\n"
                    "  <container name=\"c\">\n"
                    "    <config value=\"false\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </config>\n"
                    "    <status value=\"current\">\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "    </status>\n"
                    "    <description>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>desc</text>\n"
                    "    </description>\n"
                    "    <reference>\n"
                    "      <e:a/>\n      <e:b x=\"one\"/>\n      <e:c>\n        <e:y>one</e:y>\n      </e:c>\n"
                    "      <text>ref</text>\n"
                    "    </reference>\n"
                    "  </container>\n</module>\n";

    mod = lys_parse_mem(st->ctx, yin, LYS_IN_YIN);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YIN, NULL);
    assert_ptr_not_equal(st->str1, NULL);
    assert_string_equal(st->str1, yin);
}

static void
test_fullset_yang(void **state)
{
    struct state *st = (*state);
    const struct lys_module *mod;
    struct stat s;

    mod = lys_parse_path(st->ctx, SCHEMA_FOLDER_YANG"/ext.yang", LYS_IN_YANG);
    assert_ptr_not_equal(mod, NULL);

    lys_print_mem(&st->str1, mod, LYS_OUT_YANG, NULL);
    assert_ptr_not_equal(st->str1, NULL);

    st->fd = open(SCHEMA_FOLDER_YANG"/ext.yang", O_RDONLY);
    fstat(st->fd, &s);
    st->str2 = malloc(s.st_size + 1);
    assert_ptr_not_equal(st->str2, NULL);
    assert_int_equal(read(st->fd, st->str2, s.st_size), s.st_size);
    st->str2[s.st_size] = '\0';

    assert_string_equal(st->str1, st->str2);
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_fullset_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_container_sub_yin, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_fullset_yang, setup_ctx_yang, teardown_ctx)
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
