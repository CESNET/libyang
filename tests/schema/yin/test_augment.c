/**
 * \file test_augment.c
 * \author Michal Vasko <mvasko@cesnet.cz>
 * \brief libyang tests - augment targets
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
 *
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

#include <cmocka.h>

#include "../../../src/libyang.h"
#include "../../config.h"

#define SCHEMA_FOLDER TESTS_DIR"/schema/yin/files"
struct ly_ctx *ctx;

static int
setup_ctx(void **state)
{
    (void)state; /* unused */

    ctx = ly_ctx_new(NULL);
    if (!ctx) {
        return -1;
    }

    return 0;
}

static int
teardown_ctx(void **state)
{
    (void)state; /* unused */

    ly_ctx_destroy(ctx, NULL);

    return 0;
}

static void
test_target_include_submodule(void **state)
{
    (void)state; /* unused */

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER);

#define SCHEMA SCHEMA_FOLDER"/a.yin"
    if (!lys_parse_path(ctx, SCHEMA, LYS_IN_YIN)) {
        fail();
    }
#undef SCHEMA
}

static void
test_leafref(void **state)
{
    (void)state; /* unused */

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER);

#define SCHEMA SCHEMA_FOLDER"/b1.yin"
    if (!lys_parse_path(ctx, SCHEMA, LYS_IN_YIN)) {
        fail();
    }
#undef SCHEMA

#define SCHEMA SCHEMA_FOLDER"/b2.yin"
    if (!lys_parse_path(ctx, SCHEMA, LYS_IN_YIN)) {
        fail();
    }
#undef SCHEMA
}

static void
test_target_augment(void **state)
{
    (void)state; /* unused */

    ly_ctx_set_searchdir(ctx, SCHEMA_FOLDER);

#define SCHEMA SCHEMA_FOLDER"/c1.yin"
    if (!lys_parse_path(ctx, SCHEMA, LYS_IN_YIN)) {
        fail();
    }
#undef SCHEMA

#define SCHEMA SCHEMA_FOLDER"/c2.yin"
    if (!lys_parse_path(ctx, SCHEMA, LYS_IN_YIN)) {
        fail();
    }
#undef SCHEMA

#define SCHEMA SCHEMA_FOLDER"/c3.yin"
    if (!lys_parse_path(ctx, SCHEMA, LYS_IN_YIN)) {
        fail();
    }
#undef SCHEMA
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_target_include_submodule, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_leafref, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_target_augment, setup_ctx, teardown_ctx)
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
