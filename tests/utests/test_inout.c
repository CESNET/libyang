/**
 * @file test_inout.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for input and output handlers functions
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _POSIX_C_SOURCE 200112L

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "log.h"
#include "printer.h"
#include "parser.h"


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
test_input_mem(void **state)
{
    struct ly_in *in = NULL;
    char *str1 = "a", *str2 = "b";

    *state = test_input_mem;

    assert_int_equal(LY_EINVAL, ly_in_new_memory(NULL, NULL));
    assert_int_equal(LY_EINVAL, ly_in_new_memory(str1, NULL));
    assert_null(ly_in_memory(NULL, NULL));

    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str1, &in));
    assert_int_equal(LY_IN_MEMORY, ly_in_type(in));
    assert_ptr_equal(str1, ly_in_memory(in, str2));
    assert_ptr_equal(str2, ly_in_memory(in, NULL));
    assert_ptr_equal(str2, ly_in_memory(in, NULL));
    ly_in_free(in, 0);

    /* cleanup */
    *state = NULL;
}

static void
test_input_fd(void **state)
{
    struct ly_in *in = NULL;
    int fd1, fd2;
    struct stat statbuf;

    *state = test_input_fd;

    assert_int_equal(LY_EINVAL, ly_in_new_fd(-1, NULL));
    assert_int_equal(-1, ly_in_fd(NULL, -1));

    assert_int_not_equal(-1, fd1 = open(__FILE__, O_RDONLY));
    assert_int_not_equal(-1, fd2 = open(__FILE__, O_RDONLY));

    assert_int_equal(LY_EINVAL, ly_in_new_fd(fd1, NULL));

    assert_int_equal(LY_SUCCESS, ly_in_new_fd(fd1, &in));
    assert_int_equal(LY_IN_FD, ly_in_type(in));
    assert_ptr_equal(fd1, ly_in_fd(in, fd2));
    assert_ptr_equal(fd2, ly_in_fd(in, -1));
    assert_ptr_equal(fd2, ly_in_fd(in, -1));
    ly_in_free(in, 1);
    /* fd1 is still open */
    assert_int_equal(0, fstat(fd1, &statbuf));
    close(fd1);
    /* but fd2 was closed by ly_in_free() */
    errno = 0;
    assert_int_equal(-1, fstat(fd2, &statbuf));
    assert_int_equal(errno, EBADF);

    /* cleanup */
    *state = NULL;
}

static void
test_input_file(void **state)
{
    struct ly_in *in = NULL;
    FILE *f1 = NULL, *f2 = NULL;

    *state = test_input_file;

    assert_int_equal(LY_EINVAL, ly_in_new_file(NULL, NULL));
    assert_null(ly_in_file(NULL, NULL));

    assert_int_not_equal(-1, f1 = fopen(__FILE__, "r"));
    assert_int_not_equal(-1, f2 = fopen(__FILE__, "r"));

    assert_int_equal(LY_EINVAL, ly_in_new_file(f1, NULL));

    assert_int_equal(LY_SUCCESS, ly_in_new_file(f1, &in));
    assert_int_equal(LY_IN_FILE, ly_in_type(in));
    assert_ptr_equal(f1, ly_in_file(in, f2));
    assert_ptr_equal(f2, ly_in_file(in, NULL));
    assert_ptr_equal(f2, ly_in_file(in, NULL));
    ly_in_free(in, 1);
    /* f1 is still open */
    assert_int_not_equal(-1, fileno(f1));
    fclose(f1);
    /* but f2 was closed by ly_in_free() */

    /* cleanup */
    *state = NULL;
}

static void
test_input_filepath(void **state)
{
    struct ly_in *in = NULL;
    const char *path1 = __FILE__, *path2 = __FILE__;

    *state = test_input_filepath;

    assert_int_equal(LY_EINVAL, ly_in_new_filepath(NULL, 0, NULL));
    assert_int_equal(LY_EINVAL, ly_in_new_filepath(path1, 0, NULL));
    assert_ptr_equal(((void *)-1), ly_in_filepath(NULL, NULL, 0));

    assert_int_equal(LY_SUCCESS, ly_in_new_filepath(path1, 0, &in));
    assert_int_equal(LY_IN_FILEPATH, ly_in_type(in));
    assert_ptr_equal(NULL, ly_in_filepath(in, path2, 0));
    assert_string_equal(path2, ly_in_filepath(in, NULL, 0));
    ly_in_free(in, 0);

    /* cleanup */
    *state = NULL;
}

static void
test_output_mem(void **state)
{
    struct ly_out *out = NULL;
    char *buf1 = NULL, *buf2 = NULL;

    *state = test_output_mem;

    /* manipulate with the handler */
    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&buf1, 0, &out));
    assert_int_equal(LY_OUT_MEMORY, ly_out_type(out));
    ly_write(out, "test", 4);
    assert_ptr_equal(buf1, ly_out_memory(out, &buf2, 0));
    assert_ptr_equal(buf2, ly_out_memory(out, NULL, 0));
    assert_ptr_equal(buf2, ly_out_memory(out, &buf1, strlen(buf1)));
    ly_out_free(out, NULL, 0);

    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&buf1, strlen(buf1), &out));
    ly_out_free(out, NULL, 1);

    /* writing data */

    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&buf1, 0, &out));
    assert_int_equal(LY_SUCCESS, ly_print(out, "test %s", "print"));
    assert_int_equal(10, ly_out_printed(out));
    assert_string_equal("test print", buf1);
    assert_int_equal(LY_SUCCESS, ly_out_reset(out));
    assert_int_equal(LY_SUCCESS, ly_write(out, "rewrite", 8));
    assert_int_equal(8, ly_out_printed(out));
    assert_string_equal("rewrite", buf1);
    ly_out_free(out, NULL, 1);

    /* cleanup */
    *state = NULL;
}

static void
test_output_fd(void **state)
{
    struct ly_out *out = NULL;
    int fd1, fd2;
    char buf[31] = {0};
    const char *filepath = "/tmp/libyang_test_output";

    *state = test_output_fd;

    assert_int_not_equal(-1, fd1 = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));
    assert_int_not_equal(-1, fd2 = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));

    /* manipulate with the handler */
    assert_int_equal(LY_SUCCESS, ly_out_new_fd(fd1, &out));
    assert_int_equal(LY_OUT_FD, ly_out_type(out));
    assert_ptr_equal(fd1, ly_out_fd(out, fd2));
    assert_ptr_equal(fd2, ly_out_fd(out, -1));
    assert_ptr_equal(fd2, ly_out_fd(out, fd1));
    ly_out_free(out, NULL, 0);
    assert_int_equal(0, close(fd2));
    assert_int_equal(LY_SUCCESS, ly_out_new_fd(fd1, &out));
    ly_out_free(out, NULL, 1);

    /* writing data */
    assert_int_not_equal(-1, fd1 = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));
    assert_int_not_equal(-1, fd2 = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));
    /* truncate file to start with no data */
    assert_int_equal(0, ftruncate(fd1, 0));

    assert_int_equal(LY_SUCCESS, ly_out_new_fd(fd1, &out));
    assert_int_equal(LY_SUCCESS, ly_print(out, "test %s", "print"));
    assert_int_equal(10, ly_out_printed(out));
    ly_print_flush(out);
    assert_int_equal(10, read(fd2, buf, 30));
    assert_string_equal("test print", buf);
    assert_int_equal(0, lseek(fd2, 0, SEEK_SET));
    assert_int_equal(LY_SUCCESS, ly_out_reset(out));

    assert_int_equal(LY_SUCCESS, ly_write(out, "rewrite", 8));
    assert_int_equal(8, ly_out_printed(out));
    ly_print_flush(out);
    assert_int_equal(8, read(fd2, buf, 30));
    assert_string_equal("rewrite", buf);

    close(fd2);
    ly_out_free(out, NULL, 1);

    /* cleanup */
    *state = NULL;
}

static void
test_output_file(void **state)
{
    struct ly_out *out = NULL;
    FILE *f1, *f2;
    char buf[31] = {0};
    const char *filepath = "/tmp/libyang_test_output";

    *state = test_output_file;

    assert_int_not_equal(-1, f1 = fopen(filepath, "w"));
    assert_int_not_equal(-1, f2 = fopen(filepath, "w"));

    /* manipulate with the handler */
    assert_int_equal(LY_SUCCESS, ly_out_new_file(f1, &out));
    assert_int_equal(LY_OUT_FILE, ly_out_type(out));
    assert_ptr_equal(f1, ly_out_file(out, f2));
    assert_ptr_equal(f2, ly_out_file(out, NULL));
    assert_ptr_equal(f2, ly_out_file(out, f1));
    ly_out_free(out, NULL, 0);
    assert_int_equal(0, fclose(f2));
    assert_int_equal(LY_SUCCESS, ly_out_new_file(f1, &out));
    ly_out_free(out, NULL, 1);

    /* writing data */
    assert_int_not_equal(-1, f1 = fopen(filepath, "w"));
    assert_int_not_equal(-1, f2 = fopen(filepath, "r"));

    assert_int_equal(LY_SUCCESS, ly_out_new_file(f1, &out));
    assert_int_equal(LY_SUCCESS, ly_print(out, "test %s", "print"));
    assert_int_equal(10, ly_out_printed(out));
    ly_print_flush(out);
    assert_non_null(fgets(buf, 31, f2));
    assert_string_equal("test print", buf);
    assert_int_equal(0, fseek(f2, 0, SEEK_SET));
    assert_int_equal(LY_SUCCESS, ly_out_reset(out));

    assert_int_equal(LY_SUCCESS, ly_write(out, "rewrite", 8));
    assert_int_equal(8, ly_out_printed(out));
    ly_print_flush(out);
    assert_non_null(fgets(buf, 31, f2));
    assert_string_equal("rewrite", buf);

    fclose(f2);
    ly_out_free(out, NULL, 1);

    /* cleanup */
    *state = NULL;
}

static void
test_output_filepath(void **state)
{
    struct ly_out *out = NULL;
    FILE *f1;
    char buf[31] = {0};
    const char *fp1 = "/tmp/libyang_test_output";
    const char *fp2 = "/tmp/libyang_test_output2";

    *state = test_output_filepath;

    /* manipulate with the handler */
    assert_int_equal(LY_SUCCESS, ly_out_new_filepath(fp1, &out));
    assert_int_equal(LY_OUT_FILEPATH, ly_out_type(out));
    assert_ptr_equal(NULL, ly_out_filepath(out, fp2));
    assert_string_equal(fp2, ly_out_filepath(out, NULL));
    assert_ptr_equal(NULL, ly_out_filepath(out, fp1));
    ly_out_free(out, NULL, 0);
    assert_int_equal(LY_SUCCESS, ly_out_new_filepath(fp1, &out));
    ly_out_free(out, NULL, 1);

    /* writing data */
    assert_int_not_equal(-1, f1 = fopen(fp1, "r"));

    assert_int_equal(LY_SUCCESS, ly_out_new_filepath(fp1, &out));
    assert_int_equal(LY_SUCCESS, ly_print(out, "test %s", "print"));
    assert_int_equal(10, ly_out_printed(out));
    ly_print_flush(out);
    assert_non_null(fgets(buf, 31, f1));
    assert_string_equal("test print", buf);
    assert_int_equal(0, fseek(f1, 0, SEEK_SET));
    assert_int_equal(LY_SUCCESS, ly_out_reset(out));

    assert_int_equal(LY_SUCCESS, ly_write(out, "rewrite", 8));
    assert_int_equal(8, ly_out_printed(out));
    ly_print_flush(out);
    assert_non_null(fgets(buf, 31, f1));
    assert_string_equal("rewrite", buf);

    fclose(f1);
    ly_out_free(out, NULL, 1);

    /* cleanup */
    *state = NULL;
}

static void
test_output_clb(void **state)
{
    struct ly_out *out = NULL;
    int fd1, fd2;
    char buf[31] = {0};
    const char *filepath = "/tmp/libyang_test_output";

    *state = test_output_clb;

    assert_int_not_equal(-1, fd1 = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));
    assert_int_not_equal(-1, fd2 = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));

    /* manipulate with the handler */
    assert_int_equal(LY_SUCCESS, ly_out_new_clb((ssize_t (*)(void *, const void *, size_t))write, (void*)(intptr_t)fd1, &out));
    assert_int_equal(LY_OUT_CALLBACK, ly_out_type(out));
    assert_ptr_equal(fd1, ly_out_clb_arg(out, (void*)(intptr_t)fd2));
    assert_ptr_equal(fd2, ly_out_clb_arg(out, NULL));
    assert_ptr_equal(fd2, ly_out_clb_arg(out, (void*)(intptr_t)fd1));
    assert_ptr_equal(write, ly_out_clb(out, (ssize_t (*)(void *, const void *, size_t))write));
    ly_out_free(out, NULL, 0);
    assert_int_equal(0, close(fd2));
    assert_int_equal(LY_SUCCESS, ly_out_new_clb((ssize_t (*)(void *, const void *, size_t))write, (void*)(intptr_t)fd1, &out));
    ly_out_free(out, (void (*)(void *))close, 0);

    /* writing data */
    assert_int_not_equal(-1, fd1 = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));
    assert_int_not_equal(-1, fd2 = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));
    /* truncate file to start with no data */
    assert_int_equal(0, ftruncate(fd1, 0));

    assert_int_equal(LY_SUCCESS, ly_out_new_clb((ssize_t (*)(void *, const void *, size_t))write, (void*)(intptr_t)fd1, &out));
    assert_int_equal(LY_SUCCESS, ly_print(out, "test %s", "print"));
    assert_int_equal(10, ly_out_printed(out));
    assert_int_equal(10, read(fd2, buf, 30));
    assert_string_equal("test print", buf);

    close(fd2);
    ly_out_free(out, (void (*)(void *))close, 0);

    /* cleanup */
    *state = NULL;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_input_mem, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_input_fd, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_input_file, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_input_filepath, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_output_mem, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_output_fd, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_output_file, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_output_filepath, logger_setup, logger_teardown),
        cmocka_unit_test_setup_teardown(test_output_clb, logger_setup, logger_teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
