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
#define _UTEST_MAIN_
#include "utests.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "in.h"
#include "log.h"
#include "out.h"

static void
test_input_mem(void **UNUSED(state))
{
    struct ly_in *in = NULL;
    char *str1 = "a", *str2 = "b";

    assert_int_equal(LY_EINVAL, ly_in_new_memory(NULL, NULL));
    assert_int_equal(LY_EINVAL, ly_in_new_memory(str1, NULL));
    assert_null(ly_in_memory(NULL, NULL));

    assert_int_equal(LY_SUCCESS, ly_in_new_memory(str1, &in));
    assert_int_equal(LY_IN_MEMORY, ly_in_type(in));
    assert_ptr_equal(str1, ly_in_memory(in, str2));
    assert_ptr_equal(str2, ly_in_memory(in, NULL));
    assert_ptr_equal(str2, ly_in_memory(in, NULL));
    ly_in_free(in, 0);
}

static void
test_input_fd(void **UNUSED(state))
{
    struct ly_in *in = NULL;
    int fd1, fd2;
    struct stat statbuf;

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
}

static void
test_input_file(void **UNUSED(state))
{
    struct ly_in *in = NULL;
    FILE *f1 = NULL, *f2 = NULL;

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
}

static void
test_input_filepath(void **UNUSED(state))
{
    struct ly_in *in = NULL;
    const char *path1 = __FILE__, *path2 = __FILE__;

    assert_int_equal(LY_EINVAL, ly_in_new_filepath(NULL, 0, NULL));
    assert_int_equal(LY_EINVAL, ly_in_new_filepath(path1, 0, NULL));
    assert_ptr_equal(((void *)-1), ly_in_filepath(NULL, NULL, 0));

    assert_int_equal(LY_SUCCESS, ly_in_new_filepath(path1, 0, &in));
    assert_int_equal(LY_IN_FILEPATH, ly_in_type(in));
    assert_ptr_equal(NULL, ly_in_filepath(in, path2, 0));
    assert_string_equal(path2, ly_in_filepath(in, NULL, 0));
    ly_in_free(in, 0);
}

static void
test_output_mem(void **UNUSED(state))
{
    struct ly_out *out = NULL;
    char *buf1 = NULL, *buf2 = NULL;

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
}

static void
test_output_fd(void **UNUSED(state))
{
    struct ly_out *out = NULL;
    int fd1, fd2;
    char buf[31] = {0};
    const char *filepath = "/tmp/libyang_test_output";

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
}

static void
test_output_file(void **UNUSED(state))
{
    struct ly_out *out = NULL;
    FILE *f1, *f2;
    char buf[31] = {0};
    const char *filepath = "/tmp/libyang_test_output";

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
}

static void
test_output_filepath(void **UNUSED(state))
{
    struct ly_out *out = NULL;
    FILE *f1;
    char buf[31] = {0};
    const char *fp1 = "/tmp/libyang_test_output";
    const char *fp2 = "/tmp/libyang_test_output2";

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
}

static void
test_output_clb(void **UNUSED(state))
{
    struct ly_out *out = NULL;
    int fd1, fd2;
    char buf[31] = {0};
    const char *filepath = "/tmp/libyang_test_output";

    assert_int_not_equal(-1, fd1 = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));
    assert_int_not_equal(-1, fd2 = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));

    /* manipulate with the handler */
    assert_int_equal(LY_SUCCESS, ly_out_new_clb((void *)write, (void *)(intptr_t)fd1, &out));
    assert_int_equal(LY_OUT_CALLBACK, ly_out_type(out));
    assert_ptr_equal(fd1, ly_out_clb_arg(out, (void *)(intptr_t)fd2));
    assert_ptr_equal(fd2, ly_out_clb_arg(out, NULL));
    assert_ptr_equal(fd2, ly_out_clb_arg(out, (void *)(intptr_t)fd1));
    assert_ptr_equal(write, ly_out_clb(out, (void *)write));
    ly_out_free(out, NULL, 0);
    assert_int_equal(0, close(fd2));
    assert_int_equal(LY_SUCCESS, ly_out_new_clb((void *)write, (void *)(intptr_t)fd1, &out));
    ly_out_free(out, (void *)close, 0);

    /* writing data */
    assert_int_not_equal(-1, fd1 = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));
    assert_int_not_equal(-1, fd2 = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));
    /* truncate file to start with no data */
    assert_int_equal(0, ftruncate(fd1, 0));

    assert_int_equal(LY_SUCCESS, ly_out_new_clb((void *)write, (void *)(intptr_t)fd1, &out));
    assert_int_equal(LY_SUCCESS, ly_print(out, "test %s", "print"));
    assert_int_equal(10, ly_out_printed(out));
    assert_int_equal(10, read(fd2, buf, 30));
    assert_string_equal("test print", buf);

    close(fd2);
    ly_out_free(out, (void *)close, 0);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_input_mem),
        UTEST(test_input_fd),
        UTEST(test_input_file),
        UTEST(test_input_filepath),
        UTEST(test_output_mem),
        UTEST(test_output_fd),
        UTEST(test_output_file),
        UTEST(test_output_filepath),
        UTEST(test_output_clb),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
