/*
 * @file set.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from common.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "../../src/common.c"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "libyang.h"

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};

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

    ly_set_log_clb(logger, 0);

    return 0;
}

static void
test_utf8(void **state)
{
    (void) state; /* unused */

    char buf[5] = {0};
    const char *str = buf;
    unsigned int c;
    size_t len;

    /* test invalid UTF-8 characters in lyxml_getutf8
     * - https://en.wikipedia.org/wiki/UTF-8 */
    buf[0] = 0x04;
    assert_int_equal(LY_EINVAL, ly_getutf8(&str, &c, &len));
    buf[0] = 0x80;
    assert_int_equal(LY_EINVAL, ly_getutf8(&str, &c, &len));

    buf[0] = 0xc0;
    buf[1] = 0x00;
    assert_int_equal(LY_EINVAL, ly_getutf8(&str, &c, &len));
    buf[1] = 0x80;
    assert_int_equal(LY_EINVAL, ly_getutf8(&str, &c, &len));

    buf[0] = 0xe0;
    buf[1] = 0x00;
    buf[2] = 0x80;
    assert_int_equal(LY_EINVAL, ly_getutf8(&str, &c, &len));
    buf[1] = 0x80;
    assert_int_equal(LY_EINVAL, ly_getutf8(&str, &c, &len));

    buf[0] = 0xf0;
    buf[1] = 0x00;
    buf[2] = 0x80;
    buf[3] = 0x80;
    assert_int_equal(LY_EINVAL, ly_getutf8(&str, &c, &len));
    buf[1] = 0x80;
    assert_int_equal(LY_EINVAL, ly_getutf8(&str, &c, &len));
}

void *__real_realloc(void *ptr, size_t size);
void *__wrap_realloc(void *ptr, size_t size)
{
    int wrap = mock_type(int);

    if (wrap) {
        /* memory allocation failed */
        return NULL;
    } else {
        return __real_realloc(ptr, size);
    }
}

static void
test_lyrealloc(void **state)
{
    (void) state; /* unused */

    char *ptr;

    ptr = malloc(1);
    assert_non_null(ptr);

    /* realloc */
    will_return(__wrap_realloc, 0);
    ptr = ly_realloc(ptr, 2048);
    assert_non_null(ptr);
    ptr[2047] = 0; /* test write */

    /* realloc fails */
    will_return(__wrap_realloc, 1);
    ptr = ly_realloc(ptr, 2048);
    assert_null(ptr);

    /* ptr should be freed by ly_realloc() */
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_utf8, logger_setup),
        cmocka_unit_test(test_lyrealloc),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
