/**
 * \file test_ietf.c
 * \author Radek Krejci <rkrejci@cesnet.cz>
 * \brief libyang tests - loading standard IETF modules
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
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

#include "../../../src/libyang.h"
#include "../../config.h"

#define SCHEMA_FOLDER TESTS_DIR"/schema/yin/ietf"

static int
setup_ctx(void **state)
{
    //ly_verb(LY_LLVRB);
    (*state) = ly_ctx_new(SCHEMA_FOLDER);
    if (!(*state)) {
        return -1;
    }

    return 0;
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
    DIR *dir;
    struct dirent *file;
    size_t flen;

    dir = opendir(SCHEMA_FOLDER);
    if (!dir) {
        fprintf(stderr, "unable to open \"%s\" folder.\n", SCHEMA_FOLDER);
        fail();
    }
    chdir(SCHEMA_FOLDER);

    while ((file = readdir(dir))) {
        flen = strlen(file->d_name);
        if (strcmp(&file->d_name[flen - 4], ".yin")) {
            /* skip non-YIN files */
            continue;
        }

        if (!strncmp(file->d_name, "ietf-snmp-", 10)) {
            /* skip ietf-snmp's submodules */
            continue;
        }

        fprintf(stdout, "Loading \"%s\" module ... ", file->d_name);
        if (!lys_parse_path(ctx, file->d_name, LYS_IN_YIN)) {
            fprintf(stdout, "failed\n");
            closedir(dir);
            fail();
        }
        fprintf(stdout, "ok\n");
    }
    closedir(dir);
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_modules, setup_ctx, teardown_ctx)
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}
