/**
 * @file test_data_initialization.c
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Cmocka data test initialization.
 *
 * Copyright (c) 2015 Sartura d.o.o.
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
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include "../src/libyang.h"

struct ly_ctx *ctx;
struct lyd_node *root;
struct lys_module *model;

int generic_init(char *config_file, char *yang_file, char *yang_folder)
{
	LYS_INFORMAT yang_format;
	LYD_FORMAT in_format;
	char *schema = NULL;
	char *config = NULL;
	struct stat sb;
	int fd = 0;

	if (!config_file || !yang_file || !yang_folder)
		goto error;

	yang_format = LYS_IN_YIN;
	in_format = LYD_XML;

	ctx = ly_ctx_new(yang_folder);
	if (!ctx)
		goto error;

	fd = open(yang_file, O_RDONLY);
	if (fd == -1)
		goto error;
	if (fstat(fd, &sb) == -1)
		goto error;
	if (!S_ISREG(sb.st_mode))
		goto error;

	schema = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);

	fd = open(config_file, O_RDONLY);
	if (fd == -1)
		goto error;
	if (fstat(fd, &sb) == -1)
		goto error;
	if (!S_ISREG(sb.st_mode))
		goto error;

	config = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);

	model = lys_parse(ctx, schema, yang_format);
	if (!model)
		goto error;

	root = lyd_parse(ctx, config, in_format, LYD_OPT_STRICT);
	if (!root)
		goto error;

	return 0;

error:
	if (!schema)
		munmap(schema, sb.st_size);
	if (!config)
		munmap(config, sb.st_size);
	if (fd != -1)
		close(fd);

	return -1;
}

static int setup_f()
{
	char *config_file = "../../tests/config/hello.xml";
	char *yang_file = "../../tests/config/hello@2015-06-08.yin";
	char *yang_folder = "../../tests/config";
	int rc;

	rc = generic_init(config_file, yang_file, yang_folder);
	if (rc)
		return -1;

	return 0;
}

static int teardown_f()
{
	lyd_free(root);
	ly_ctx_destroy(ctx);

	return 0;
}

static void test_ctx_new_destroy()
{
	ctx = ly_ctx_new(NULL);
	if (!ctx)
		fail();

	ly_ctx_destroy(ctx);
}


static void test_container_name()
{
	struct lyd_node *node;
	const char *result = "";

	node = root;
	result = node->schema->name;

	assert_string_equal("hello", result);
}

static void test_leaf_name()
{
	struct lyd_node *node;
	const char *result;

	node = root;
	result = node->schema->child->name;

	assert_string_equal("foo", result);
}

static void test_leaf_list_parameters()
{
	struct lyd_node *node;
	struct lyd_node *tmp;
	const char *name_result;
	const char *str_result;
	int int_result;

	node = root;
	tmp = node->child->next;
	name_result = tmp->schema->name;
	int_result = ((struct lyd_node_leaf_list *) tmp)->value.int32;
	str_result = ((struct lyd_node_leaf_list *) tmp)->value_str;

	assert_int_equal(1234, int_result);
	assert_string_equal("1234", str_result);
	assert_string_equal("bar", name_result);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_ctx_new_destroy),
		cmocka_unit_test_setup_teardown(test_container_name, setup_f, teardown_f),
		cmocka_unit_test_setup_teardown(test_leaf_name, setup_f, teardown_f),
		cmocka_unit_test_setup_teardown(test_leaf_list_parameters, setup_f, teardown_f),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
