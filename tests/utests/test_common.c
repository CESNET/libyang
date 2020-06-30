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

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "common.h"

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};
int store = -1; /* negative for infinite logging, positive for limited logging */

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

#if ENABLE_LOGGER_CHECKING
static void
logger(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) level; /* unused */
    if (store) {
        if (path && path[0]) {
            snprintf(logbuf, BUFSIZE - 1, "%s %s", msg, path);
        } else {
            strncpy(logbuf, msg, BUFSIZE - 1);
        }
        if (store > 0) {
            --store;
        }
    }
}
#endif

static int
logger_setup(void **state)
{
    (void) state; /* unused */

    ly_set_log_clb(logger, 0);

    return 0;
}

static int
logger_teardown(void **state)
{
    (void) state; /* unused */
#if ENABLE_LOGGER_CHECKING
    if (*state) {
        fprintf(stderr, "%s\n", logbuf);
    }
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

#ifndef __APPLE__
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
#endif /* not __APPLE__ */

static void
test_parse_int(void **state)
{
    *state = test_parse_int;
    const char *str;
    int64_t i = 500;

    str = "10";
    assert_int_equal(LY_SUCCESS, ly_parse_int(str, strlen(str), -10, 10, 10, &i));
    assert_int_equal(i, 10);

    /* leading zeros are allowed, trailing whitespaces are allowed */
    str = "000\n\t  ";
    assert_int_equal(LY_SUCCESS, ly_parse_int(str, strlen(str), -10, 10, 10, &i));
    assert_int_equal(i, 0);

    /* negative value */
    str = "-10";
    assert_int_equal(LY_SUCCESS, ly_parse_int(str, strlen(str), -10, 10, 10, &i));
    assert_int_equal(i, -10);

    /* non-NULL terminated string */
    str = "+5sometext";
    assert_int_equal(LY_SUCCESS, ly_parse_int(str, 2, -10, 10, 10, &i));
    assert_int_equal(i, 5);

    /* out of bounds value */
    str = "11";
    assert_int_equal(LY_EDENIED, ly_parse_int(str, strlen(str), -10, 10, 10, &i));
    str = "-11";
    assert_int_equal(LY_EDENIED, ly_parse_int(str, strlen(str), -10, 10, 10, &i));

    /* NaN */
    str = "zero";
    assert_int_equal(LY_EVALID, ly_parse_int(str, strlen(str), -10, 10, 10, &i));

    /* mixing number with text */
    str = "10zero";
    assert_int_equal(LY_EVALID, ly_parse_int(str, strlen(str), -10, 10, 10, &i));

    str = "10  zero";
    assert_int_equal(LY_EVALID, ly_parse_int(str, strlen(str), -10, 10, 10, &i));

    *state = NULL;
}

static void
test_parse_uint(void **state)
{
    *state = test_parse_int;
    const char *str;
    uint64_t u = 500;

    str = "10";
    assert_int_equal(LY_SUCCESS, ly_parse_uint(str, strlen(str), 10, 10, &u));
    assert_int_equal(u, 10);

    /* leading zeros are allowed, trailing whitespaces are allowed */
    str = "000\n\t  ";
    assert_int_equal(LY_SUCCESS, ly_parse_uint(str, strlen(str), 10, 10, &u));
    assert_int_equal(u, 0);
    /* non-NULL terminated string */
    str = "+5sometext";
    assert_int_equal(LY_SUCCESS, ly_parse_uint(str, 2, 10, 10, &u));
    assert_int_equal(u, 5);

    /* out of bounds value */
    str = "11";
    assert_int_equal(LY_EDENIED, ly_parse_uint(str, strlen(str), 10, 10, &u));
    str = "-1";
    assert_int_equal(LY_EDENIED, ly_parse_uint(str, strlen(str), (uint64_t)-1, 10, &u));

    /* NaN */
    str = "zero";
    assert_int_equal(LY_EVALID, ly_parse_uint(str, strlen(str), 10, 10, &u));

    /* mixing number with text */
    str = "10zero";
    assert_int_equal(LY_EVALID, ly_parse_uint(str, strlen(str), 10, 10, &u));

    str = "10  zero";
    assert_int_equal(LY_EVALID, ly_parse_uint(str, strlen(str), 10, 10, &u));

    *state = NULL;
}

static void
test_parse_nodeid(void **state)
{
    *state = test_parse_nodeid;
    const char *str;
    const char *prefix, *name;
    size_t prefix_len, name_len;

    str = "123";
    assert_int_equal(LY_EINVAL, ly_parse_nodeid(&str, &prefix, &prefix_len, &name, &name_len));

    str = "a12_-.!";
    assert_int_equal(LY_SUCCESS, ly_parse_nodeid(&str, &prefix, &prefix_len, &name, &name_len));
    assert_null(prefix);
    assert_int_equal(0, prefix_len);
    assert_non_null(name);
    assert_int_equal(6, name_len);
    assert_int_equal(0, strncmp("a12_-.", name, name_len));
    assert_string_equal("!", str);

    str = "a12_-.:_b2 xxx";
    assert_int_equal(LY_SUCCESS, ly_parse_nodeid(&str, &prefix, &prefix_len, &name, &name_len));
    assert_non_null(prefix);
    assert_int_equal(6, prefix_len);
    assert_int_equal(0, strncmp("a12_-.", prefix, prefix_len));
    assert_non_null(name);
    assert_int_equal(3, name_len);
    assert_int_equal(0, strncmp("_b2", name, name_len));
    assert_string_equal(" xxx", str);

    *state = NULL;
}

static void
test_parse_instance_predicate(void **state)
{
    *state = test_parse_instance_predicate;
    const char *str, *errmsg;
    const char *prefix, *id, *value;
    size_t prefix_len, id_len, value_len;

    str = "[ex:name='fred']";
    assert_int_equal(LY_SUCCESS, ly_parse_instance_predicate(&str, strlen(str), LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(str, "");
    assert_string_equal(prefix, "ex:name='fred']");
    assert_int_equal(prefix_len, 2);
    assert_string_equal(id, "name='fred']");
    assert_int_equal(id_len, 4);
    assert_string_equal(value, "fred']");
    assert_int_equal(value_len, 4);

    str = "[ex:ip = \"[192.0.2.1]\"][ex:port='80']";
    assert_int_equal(LY_SUCCESS, ly_parse_instance_predicate(&str, strlen(str), LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(str, "[ex:port='80']");
    assert_string_equal(prefix, "ex:ip = \"[192.0.2.1]\"][ex:port='80']");
    assert_int_equal(prefix_len, 2);
    assert_string_equal(id, "ip = \"[192.0.2.1]\"][ex:port='80']");
    assert_int_equal(id_len, 2);
    assert_string_equal(value, "[192.0.2.1]\"][ex:port='80']");
    assert_int_equal(value_len, 11);

    str = "[. = 'blowfish-cbc']";
    assert_int_equal(LY_SUCCESS, ly_parse_instance_predicate(&str, strlen(str), LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(str, "");
    assert_null(prefix);
    assert_int_equal(prefix_len, 0);
    assert_string_equal(id, ". = 'blowfish-cbc']");
    assert_int_equal(id_len, 1);
    assert_string_equal(value, "blowfish-cbc']");
    assert_int_equal(value_len, 12);

    str = "[ 3 ]";
    assert_int_equal(LY_SUCCESS, ly_parse_instance_predicate(&str, strlen(str), LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(str, "");
    assert_null(prefix);
    assert_int_equal(prefix_len, 0);
    assert_null(id);
    assert_int_equal(id_len, 0);
    assert_string_equal(value, "3 ]");
    assert_int_equal(value_len, 1);

    /* invalid predicates */
    /* position must be positive integer */
    str = "[0]";
    assert_int_equal(LY_EVALID, ly_parse_instance_predicate(&str, strlen(str), LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(errmsg, "The position predicate cannot be zero.");
    str = "[-1]";
    assert_int_equal(LY_EVALID, ly_parse_instance_predicate(&str, strlen(str), LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(errmsg, "Invalid instance predicate format (negative position or invalid node-identifier).");

    /* invalid node-identifier */
    str = "[$node='value']";
    assert_int_equal(LY_EVALID, ly_parse_instance_predicate(&str, strlen(str), LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(errmsg, "Invalid node-identifier.");
    str = "[.node='value']";
    assert_int_equal(LY_EVALID, ly_parse_instance_predicate(&str, strlen(str), LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(errmsg, "Unexpected character instead of '=' in leaf-list-predicate.");
    str = "[13node='value']";
    assert_int_equal(LY_EVALID, ly_parse_instance_predicate(&str, strlen(str), LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(errmsg, "Predicate (pos) is not terminated by \']\' character.");

    str = "[ex:node]";
    assert_int_equal(LY_EVALID, ly_parse_instance_predicate(&str, strlen(str), LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(errmsg, "Unexpected character instead of '=' in key-predicate.");

    str = "[ex:node=  value]";
    assert_int_equal(LY_EVALID, ly_parse_instance_predicate(&str, strlen(str), LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(errmsg, "String value is not quoted.");

    str = "[ex:node='value\"]";
    assert_int_equal(LY_EVALID, ly_parse_instance_predicate(&str, strlen(str), LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(errmsg, "Value is not terminated quoted-string.");

    str = "[ex:node='value  ]";
    assert_int_equal(LY_EVALID, ly_parse_instance_predicate(&str, strlen(str), LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(errmsg, "Value is not terminated quoted-string.");

    str = "[ex:node=\"value\"[3]";
    assert_int_equal(LY_EVALID, ly_parse_instance_predicate(&str, strlen(str), LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(errmsg, "Predicate (key-predicate) is not terminated by \']\' character.");
    str = "[.=\"value\"[3]";
    assert_int_equal(LY_EVALID, ly_parse_instance_predicate(&str, strlen(str), LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(errmsg, "Predicate (leaf-list-predicate) is not terminated by \']\' character.");

    /* the limit of the string is too short, it ends one character earlier */
    str = "[ex:node='value']";
    assert_int_equal(LY_EINVAL, ly_parse_instance_predicate(&str, strlen(str) - 1, LYD_XML, &prefix, &prefix_len, &id, &id_len, &value, &value_len, &errmsg));
    assert_string_equal(errmsg, "Predicate is incomplete.");

    *state = NULL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_utf8, logger_setup, logger_teardown),
#ifndef __APPLE__
        cmocka_unit_test(test_lyrealloc),
#endif
        cmocka_unit_test_setup_teardown(test_parse_int, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_parse_uint, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_parse_nodeid, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_parse_instance_predicate, logger_setup, logger_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
