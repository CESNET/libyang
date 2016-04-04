/**
 * \file test_ietf.c
 * \author Radek Krejci <rkrejci@cesnet.cz>
 * \brief libyang tests - loading standard IETF modules
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <dirent.h>
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
#include "../../src/context.h"
#include "../config.h"

#define SCHEMA_FOLDER_YIN TESTS_DIR"/schema/yin/ietf"
#define SCHEMA_FOLDER_YANG TESTS_DIR"/schema/yang/ietf"

static int
setup_ctx(void **state, int format)
{
    //ly_verb(LY_LLVRB);
    if (format == LYS_IN_YANG){
        (*state) = ly_ctx_new(SCHEMA_FOLDER_YANG);
    } else {
        (*state) = ly_ctx_new(SCHEMA_FOLDER_YIN);
    }
    if (!(*state)) {
        return -1;
    }

    return 0;
}

static int
setup_ctx_yin(void **state)
{
    return setup_ctx(state, LYS_IN_YIN);
}

static int
setup_ctx_yang(void **state)
{
    return setup_ctx(state, LYS_IN_YANG);
}

static int
teardown_ctx(void **state)
{
    ly_ctx_destroy((struct ly_ctx *)(*state), NULL);
    (*state) = NULL;

    return 0;
}

static void
test_modules(void **state)
{
    struct ly_ctx *ctx = *state;
    char *extension;

    char files[19][32] = { "iana-crypt-hash", "iana-if-type", "ietf-inet-types@2010-09-24",
                           "ietf-inet-types", "ietf-interfaces", "ietf-ipfix-psamp", "ietf-ip",
                           "ietf-netconf-acm", "ietf-netconf-monitoring", "ietf-netconf-notifications",
                           "ietf-netconf-partial-lock", "ietf-netconf-with-defaults", "ietf-netconf",
                           "ietf-snmp", "ietf-system", "ietf-x509-cert-to-name", "ietf-yang-smiv2",
                           "ietf-yang-types@2010-09-24", "ietf-yang-types"
                         };
    int i, format;

    if (!strcmp(ctx->models.search_path, SCHEMA_FOLDER_YIN)) {
        extension = ".yin";
        format = LYS_IN_YIN;
    }  else {
        extension = ".yang";
        format = LYS_IN_YANG;
    }

    if (chdir(ctx->models.search_path)) {
        fprintf(stderr, "unable to open \"%s\" folder.\n", ctx->models.search_path);
        fail();
    }

    for (i = 0; i < 19; i++) {
        strcat(files[i], extension);
        fprintf(stdout, "Loading \"%s\" module ... ", files[i]);
        if (!lys_parse_path(ctx, files[i], format)) {
            fprintf(stdout, "failed\n");
            fail();
        }
        fprintf(stdout, "ok\n");
    }
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_modules, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_modules, setup_ctx_yang, teardown_ctx)
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
