/**
 * @file test_data_initialization.c
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Cmocka data test initialization.
 *
 * Copyright (c) 2015 Sartura d.o.o.
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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include "../config.h"
#include "../../src/libyang.h"
#include "../../src/tree_data.h"
#include "../../src/tree_schema.h"

struct ly_ctx *ctx = NULL;
struct lyd_node *root = NULL;

const char *a_data_xml = "\
<x xmlns=\"urn:a\">\n\
  <bubba>test</bubba>\n\
  </x>\n";

const char *lys_module_a = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>           \
<module name=\"a\"                                    \
        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"   \
        xmlns:a=\"urn:a\">                            \
  <namespace uri=\"urn:a\"/>                          \
  <prefix value=\"a_mod\"/>                           \
  <include module=\"asub\"/>                          \
  <include module=\"atop\"/>                          \
  <feature name=\"foo\"/>                             \
  <grouping name=\"gg\">                              \
    <leaf name=\"bar-gggg\">                          \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </grouping>                                         \
  <container name=\"x\">                              \
    <leaf name=\"bar-leaf\">                          \
      <if-feature name=\"bar\"/>                      \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <uses name=\"gg\">                                \
      <if-feature name=\"bar\"/>                      \
    </uses>                                           \
    <leaf name=\"baz\">                               \
      <if-feature name=\"foo\"/>                      \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <leaf name=\"bubba\">                             \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </container>                                        \
  <augment target-node=\"/x\">                        \
    <if-feature name=\"bar\"/>                        \
    <container name=\"bar-y\"/>                       \
  </augment>                                          \
  <rpc name=\"bar-rpc\">                              \
    <if-feature name=\"bar\"/>                        \
  </rpc>                                              \
  <rpc name=\"foo-rpc\">                              \
    <if-feature name=\"foo\"/>                        \
  </rpc>                                              \
</module>                                             \
";

const char *result_xml = "<x xmlns=\"urn:a\"><bubba>test</bubba></x>";

const char *result_xml_format ="\
<x xmlns=\"urn:a\">\n\
  <bubba>test</bubba>\n\
</x>\n\
";

const char *result_json = "\
{\n\
  \"a:x\": {\n\
    \"bubba\": \"test\"\n\
  }\n\
}\n\
";

int
generic_init(char *config_file, char *yang_file, char *yang_folder)
{
    LYS_INFORMAT yang_format;
    LYD_FORMAT in_format;
    char *schema = NULL;
    char *config = NULL;
    struct stat sb_schema, sb_config;
    int fd = -1;

    if (!config_file || !yang_file || !yang_folder) {
        goto error;
    }

    yang_format = LYS_IN_YIN;
    in_format = LYD_XML;

    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        goto error;
    }

    fd = open(yang_file, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb_schema) == -1 || !S_ISREG(sb_schema.st_mode)) {
        goto error;
    }

    schema = mmap(NULL, sb_schema.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    fd = open(config_file, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb_config) == -1 || !S_ISREG(sb_config.st_mode)) {
        goto error;
    }

    config = mmap(NULL, sb_config.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    fd = -1;

    if (!lys_parse_mem(ctx, schema, yang_format)) {
        goto error;
    }

    root = lyd_parse_mem(ctx, config, in_format, LYD_OPT_STRICT);
    if (!root) {
        goto error;
    }

    /* cleanup */
    munmap(config, sb_config.st_size);
    munmap(schema, sb_schema.st_size);

    return 0;

error:
    if (schema) {
        munmap(schema, sb_schema.st_size);
    }
    if (config) {
        munmap(config, sb_config.st_size);
    }
    if (fd != -1) {
        close(fd);
    }

    return -1;
}

static int
setup_f(void **state)
{
    (void) state; /* unused */
    char *config_file = TESTS_DIR"/api/files/a.xml";
    char *yang_file = TESTS_DIR"/api/files/a.yin";
    char *yang_folder = TESTS_DIR"/api/files";
    int rc;

    rc = generic_init(config_file, yang_file, yang_folder);

    if (rc) {
        return -1;
    }

    return 0;
}

static int
setup_f_keys(void **state)
{
    (void) state; /* unused */
    char *config_file = TESTS_DIR"/api/files/keys.xml";
    char *yang_file = TESTS_DIR"/api/files/keys.yin";
    char *yang_folder = TESTS_DIR"/api/files";
    int rc;

    rc = generic_init(config_file, yang_file, yang_folder);

    if (rc) {
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    (void) state; /* unused */
    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    return 0;
}

static void
test_lyd_parse_mem(void **state)
{
    (void) state; /* unused */
    char *yang_folder = TESTS_DIR"/api/files";
    LYD_FORMAT in_format = LYD_XML;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct ly_ctx *ctx = NULL;
    struct lyd_node *root = NULL;

    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        goto error;
    }

    if (!lys_parse_mem(ctx, lys_module_a, yang_format)) {
        goto error;
    }

    root = lyd_parse_mem(ctx, a_data_xml, in_format, LYD_OPT_STRICT);
    if (!root) {
        goto error;
    }

    assert_string_equal("x", root->schema->name);

    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    return;
error:

    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    fail();
}

static void
test_lyd_parse_fd(void **state)
{
    (void) state; /* unused */
    char *yang_folder = TESTS_DIR"/api/files";
    char *config_file = TESTS_DIR"/api/files/a.xml";
    LYD_FORMAT in_format = LYD_XML;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct ly_ctx *ctx = NULL;
    struct lyd_node *root = NULL;
    struct stat sb;
    int fd;

    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        goto error;
    }

    if (!lys_parse_mem(ctx, lys_module_a, yang_format)) {
        goto error;
    }

    fd = open(config_file, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    root = lyd_parse_fd(ctx, fd, in_format, LYD_OPT_STRICT);
    if (!root) {
        goto error;
    }

    assert_string_equal("x", root->schema->name);

    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);
    if (fd > 0)
        close(fd);

    return;
error:

    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);
    if (fd > 0)
        close(fd);
    fail();
}

static void
test_lyd_parse_path(void **state)
{
    (void) state; /* unused */
    char *yang_folder = TESTS_DIR"/api/files";
    char *config_file = TESTS_DIR"/api/files/a.xml";
    LYD_FORMAT in_format = LYD_XML;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct ly_ctx *ctx = NULL;
    struct lyd_node *root = NULL;

    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        goto error;
    }

    if (!lys_parse_mem(ctx, lys_module_a, yang_format)) {
        goto error;
    }

    root = lyd_parse_path(ctx, config_file, in_format, LYD_OPT_STRICT);
    if (!root) {
        goto error;
    }

    assert_string_equal("x", root->schema->name);

    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    return;
error:

    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);
    fail();
}

static void
test_lyd_parse_xml(void **state)
{
    (void) state; /* unused */
    char *yang_folder = TESTS_DIR"/api/files";
    struct lyd_node *node;
    const struct lys_module *module;
    struct lyxml_elem *root_xml = NULL;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    struct ly_ctx *ctx = NULL;
    struct lyd_node *root = NULL;

    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        goto error;
    }

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        goto error;
    }

    root_xml = lyxml_parse_mem(ctx, a_data_xml, 1);

    node = lyd_parse_xml(ctx, &root_xml, LYD_OPT_DATA);
    if (!node) {
        goto error;
    }

    assert_string_equal("x", node->schema->name);

    if (node)
        lyd_free(node);
    if (root)
        lyd_free(root);
    if (root_xml)
        lyxml_free(ctx, root_xml);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    return;
error:

    if (node)
        lyd_free(node);
    if (root)
        lyd_free(root);
    if (root_xml)
        lyxml_free(ctx, root_xml);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);
    fail();
}

static void
test_lyd_new(void **state)
{
    (void) state; /* unused */
    char *yang_folder = TESTS_DIR"/api/files";
    struct lyd_node *new;
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    LYD_FORMAT in_format = LYD_XML;
    struct ly_ctx *ctx = NULL;
    struct lyd_node *root = NULL;

    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        goto error;
    }

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        goto error;
    }

    root = lyd_parse_mem(ctx, a_data_xml, in_format, LYD_OPT_STRICT);
    if (!root) {
        goto error;
    }

    new = lyd_new(root, module, "bar-y");
    if (!new) {
        goto error;
    }

    assert_string_equal("bar-y", new->schema->name);

    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    return;
error:

    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);
    fail();
}

static void
test_lyd_new_leaf(void **state)
{
    (void) state; /* unused */
    char *yang_folder = TESTS_DIR"/api/files";
    struct lyd_node *new;
    const struct lys_module *module;
    LYS_INFORMAT yang_format = LYS_IN_YIN;
    LYD_FORMAT in_format = LYD_XML;
    struct ly_ctx *ctx = NULL;
    struct lyd_node *root = NULL;
    struct lyd_node_leaf_list *result = NULL;

    ctx = ly_ctx_new(yang_folder);
    if (!ctx) {
        goto error;
    }

    module = lys_parse_mem(ctx, lys_module_a, yang_format);
    if (!module) {
        goto error;
    }

    root = lyd_parse_mem(ctx, a_data_xml, in_format, LYD_OPT_STRICT);
    if (!root) {
        goto error;
    }

    new = lyd_new_leaf(root, module, "bar-leaf", "test");
    if (!new) {
        goto error;
    }

    result = (struct lyd_node_leaf_list *) new;
    assert_string_equal("test", result->value_str);

    if (new)
        lyd_free(new);
    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    return;
error:

    if (new)
        lyd_free(new);
    if (root)
        lyd_free(root);
    if (ctx)
        ly_ctx_destroy(ctx, NULL);
    fail();
}

static void
test_lyd_change_leaf(void **state)
{
    (void) state; /* unused */
    struct lyd_node_leaf_list *result = NULL;
    int rc;

    result = (struct lyd_node_leaf_list *) root->child;
    assert_string_equal("test", result->value_str);

    rc = lyd_change_leaf(result, "new_test");
    if (rc) {
	fail();
    }

    assert_string_equal("new_test", result->value_str);

}

static void
test_lyd_dup(void **state)
{
    (void) state; /* unused */
    struct lyd_node *copy = NULL;

    copy = lyd_dup(root->child, 0);
    if (!copy) {
	fail();
    }

    assert_string_equal("bubba", copy->schema->name);

    free(copy);
}

static void
test_lyd_insert(void **state)
{
    (void) state; /* unused */
    struct lyd_node *new = NULL;
    struct lyd_node *node = root->child;
    struct lyd_node_leaf_list *result;
    int rc;

    result = (struct lyd_node_leaf_list *) root->child;
    assert_string_equal("test", result->value_str);
    if (root->child->next) {
        fail();
    }

    new = lyd_new_leaf(root, node->schema->module, "bar-leaf", "new_value");
    if (!new) {
        fail();
    }

    rc = lyd_insert(node, new);
    if (!rc) {
        fail();
    }

    result = (struct lyd_node_leaf_list *) root->child->next;
    assert_string_equal("new_value", result->value_str);
}

static void
test_lyd_insert_before(void **state)
{
    (void) state; /* unused */
    struct lyd_node *new = NULL;
    struct lyd_node *node = root->child;
    struct lyd_node_leaf_list *result;
    int rc;

    result = (struct lyd_node_leaf_list *) root->child;
    assert_string_equal("test", result->value_str);
    if (root->child->next) {
        fail();
    }

    new = lyd_new_leaf(root, node->schema->module, "bar-leaf", "foo");
    if (!new) {
        fail();
    }

    rc = lyd_insert(node, new);
    if (!rc) {
        fail();
    }

    new = lyd_new_leaf(root, node->schema->module, "baz", "bar");
    if (!new) {
        fail();
    }

    rc = lyd_insert_before(node->next, new);
    if (rc != EXIT_SUCCESS) {
        fail();
    }

    result = (struct lyd_node_leaf_list *) node->next;
    assert_string_equal("bar", result->value_str);

}

static void
test_lyd_insert_after(void **state)
{
    (void) state; /* unused */
    struct lyd_node *new = NULL;
    struct lyd_node *node = root->child;
    struct lyd_node_leaf_list *result;
    int rc;

    result = (struct lyd_node_leaf_list *) root->child;
    assert_string_equal("test", result->value_str);
    if (root->child->next) {
        fail();
    }

    new = lyd_new_leaf(root, node->schema->module, "bar-leaf", "foo");
    if (!new) {
        fail();
    }

    rc = lyd_insert(node, new);
    if (!rc) {
        fail();
    }

    new = lyd_new_leaf(root, node->schema->module, "baz", "bar");
    if (!new) {
        fail();
    }

    rc = lyd_insert_after(node->next, new);
    if (rc != EXIT_SUCCESS) {
        fail();
    }

    result = (struct lyd_node_leaf_list *) node->next->next;
    assert_string_equal("bar", result->value_str);

}

static void
test_lyd_get_node(void **state)
{
    (void) state; /* unused */
    struct ly_set *set = NULL;
    struct lyd_node_leaf_list *result;

    set = lyd_get_node(root->child, "/a:x/bubba");

    struct lyd_node *node = *set->dset;
    result = (struct lyd_node_leaf_list *) node;
    assert_string_equal("test", result->value_str);

    ly_set_free(set);
}

static void
test_lyd_get_node_2(void **state)
{
    (void) state; /* unused */
    struct ly_set *set = NULL;
    struct lyd_node_leaf_list *result;

    set = lyd_get_node2(root->child, root->child->schema);
    if (!set) {
        fail();
    }

    struct lyd_node *node = *set->dset;
    result = (struct lyd_node_leaf_list *) node;
    assert_string_equal("test", result->value_str);

    ly_set_free(set);
}

static void
test_lyd_get_list_keys(void **state)
{
    (void) state; /* unused */
    struct ly_set *set = NULL;
    struct lyd_node_leaf_list *result;

    set = lyd_get_list_keys(root);
    if (!set) {
        fail();
    }

    struct lyd_node *node = *set->dset;
    result = (struct lyd_node_leaf_list *) node;
    assert_string_equal("1", result->value_str);

    ly_set_free(set);
}

static void
test_lyd_validate(void **state)
{
    (void) state; /* unused */
    struct lyd_node *new = NULL;
    struct lyd_node *node = root->child;
    int rc;

    rc = lyd_validate(root, 0);
    if (rc) {
        fail();
    }

    if (root->child->next) {
        fail();
    }

    new = lyd_new_leaf(root, node->schema->module, "bar-leaf", "foo");
    if (!new) {
        fail();
    }

    rc = lyd_insert(node, new);
    if (!rc) {
        fail();
    }

    rc = lyd_validate(root, 0);
    if (!rc) {
        fail();
    }
}

static void
test_lyd_unlink(void **state)
{
    (void) state; /* unused */
    struct lyd_node *new = NULL;
    struct lyd_node *node = root->child;
    struct lyd_node_leaf_list *result;
    int rc;

    new = lyd_new_leaf(root, node->schema->module, "bar-leaf", "foo");
    if (!new) {
        fail();
    }

    rc = lyd_insert(node, new);
    if (!rc) {
        fail();
    }

    result = (struct lyd_node_leaf_list *) node->next;
    assert_string_equal("foo", result->value_str);

    rc = lyd_unlink(node->next);
    if (rc != EXIT_SUCCESS) {
        fail();
    }

    if (node->next) {
        fail();
    }

    free(new);
}

static void
test_lyd_free(void **state)
{
    (void) state; /* unused */
    struct lyd_node *copy = NULL;

    copy = lyd_dup(root->child, 0);
    if (!copy) {
	fail();
    }

    lyd_free(copy);
}

static void
test_lyd_free_withsiblings(void **state)
{
    (void) state; /* unused */
    struct lyd_node *copy = NULL;
    struct lyd_node *new = NULL;
    struct lyd_node *node = root->child;
    int rc;

    copy = lyd_dup(root->child, 0);
    if (!copy) {
	fail();
    }

    new = lyd_new_leaf(root, node->schema->module, "bar-leaf", "foo");
    if (!new) {
        fail();
    }

    rc = lyd_insert(copy, new);
    if (!rc) {
        fail();
    }

    lyd_free_withsiblings(copy);
}

static void
test_lyd_insert_attr(void **state)
{
    (void) state; /* unused */
    struct lyd_attr *attr = NULL;
    struct lyd_node *node = root->child;

    attr = lyd_insert_attr(node, "test", "test");
    if (!attr) {
        fail();
    }

    assert_string_equal("test", node->attr->name);
}

static void
test_lyd_free_attr(void **state)
{
    (void) state; /* unused */
    struct lyd_attr *attr = NULL;
    struct lyd_node *node = root->child;

    attr = lyd_insert_attr(node, "test", "test");

    if (!node->attr) {
        fail();
    }

    lyd_free_attr(ctx, node, attr, 0);

    if (node->attr) {
        fail();
    }

}

static void
test_lyd_print_mem_xml(void **state)
{
    (void) state; /* unused */
    char *result = NULL;
    int rc;

    rc = lyd_print_mem(&result, root, LYD_XML, 0);
    if (rc) {
        fail();
    }

    assert_string_equal(result_xml, result);
    free(result);
}

static void
test_lyd_print_mem_xml_format(void **state)
{
    (void) state; /* unused */
    char *result = NULL;
    int rc;

    rc = lyd_print_mem(&result, root, LYD_XML_FORMAT, 0);
    if (rc) {
        fail();
    }

    assert_string_equal(result_xml_format, result);
    free(result);
}

static void
test_lyd_print_mem_json(void **state)
{
    (void) state; /* unused */
    char *result = NULL;
    int rc;

    rc = lyd_print_mem(&result, root, LYD_JSON, 0);
    if (rc) {
        fail();
    }

    assert_string_equal(result_json, result);
    free(result);
}

static void
test_lyd_print_fd_xml(void **state)
{
    (void) state; /* unused */
    char *result = NULL;
    struct stat sb;
    char file_name[19];
    int rc;
    int fd;

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, "/tmp/libyang-XXXXXX", 21);

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }

    rc = lyd_print_fd(fd, root, LYD_XML, 0);
    if (rc) {
        goto error;
    }

    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_xml, result);

    close(fd);
    unlink(file_name);

    return;
error:
    if (fd > 0) {
        close(fd);
        unlink(file_name);
    }
    fail();
}

static void
test_lyd_print_fd_xml_format(void **state)
{
    (void) state; /* unused */
    char *result = NULL;
    struct stat sb;
    char file_name[19];
    int rc;
    int fd;

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, "/tmp/libyang-XXXXXX", 21);

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }

    rc = lyd_print_fd(fd, root, LYD_XML_FORMAT, 0);
    if (rc) {
        goto error;
    }

    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_xml_format, result);

    close(fd);
    unlink(file_name);

    return;
error:
    if (fd > 0) {
        close(fd);
        unlink(file_name);
    }
    fail();
}

static void
test_lyd_print_fd_json(void **state)
{
    (void) state; /* unused */
    char *result = NULL;
    struct stat sb;
    char file_name[19];
    int rc;
    int fd;

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, "/tmp/libyang-XXXXXX", 21);

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }

    rc = lyd_print_fd(fd, root, LYD_JSON, 0);
    if (rc) {
        goto error;
    }

    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_json, result);

    close(fd);
    unlink(file_name);

    return;
error:
    if (fd > 0) {
        close(fd);
        unlink(file_name);
    }
    fail();
}

static void
test_lyd_print_file_xml(void **state)
{
    (void) state; /* unused */
    struct stat sb;
    char file_name[19];
    char *result;
    FILE *f = NULL;
    int rc;
    int fd;

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, "/tmp/libyang-XXXXXX", 21);

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }
    close(fd);

    f = (fopen(file_name,"r+"));
    if (f == NULL) {
        goto error;
    }

    rc = lyd_print_file(f, root, LYD_XML, 0);
    if (rc) {
        goto error;
    }

    fclose(f);

    fd = open(file_name, O_RDONLY);
    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_xml, result);

    close(fd);
    unlink(file_name);

    return;
error:
    if (f)
        fclose(f);
    if (fd > 0) {
        unlink(file_name);
        close(fd);
    }
    fail();
}

static void
test_lyd_print_file_xml_format(void **state)
{
    (void) state; /* unused */
    struct stat sb;
    char file_name[19];
    char *result;
    FILE *f = NULL;
    int rc;
    int fd;

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, "/tmp/libyang-XXXXXX", 21);

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }
    close(fd);

    f = (fopen(file_name,"r+"));
    if (f == NULL) {
        goto error;
    }

    rc = lyd_print_file(f, root, LYD_XML_FORMAT, 0);
    if (rc) {
        goto error;
    }

    fclose(f);

    fd = open(file_name, O_RDONLY);
    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_xml_format, result);

    close(fd);
    unlink(file_name);

    return;
error:
    if (f)
        fclose(f);
    if (fd > 0) {
        unlink(file_name);
        close(fd);
    }
    fail();
}

static void
test_lyd_print_file_json(void **state)
{
    (void) state; /* unused */
    struct stat sb;
    char file_name[19];
    char *result;
    FILE *f = NULL;
    int rc;
    int fd;

    memset(file_name, 0, sizeof(file_name));
    strncpy(file_name, "/tmp/libyang-XXXXXX", 21);

    fd = mkstemp(file_name);
    if (fd < 1) {
        goto error;
    }
    close(fd);

    f = (fopen(file_name,"r+"));
    if (f == NULL) {
        goto error;
    }

    rc = lyd_print_file(f, root, LYD_JSON, 0);
    if (rc) {
        goto error;
    }

    fclose(f);

    fd = open(file_name, O_RDONLY);
    if (fstat(fd, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        goto error;
    }

    result = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    assert_string_equal(result_json, result);

    close(fd);
    unlink(file_name);

    return;
error:
    if (f)
        fclose(f);
    if (fd > 0) {
        unlink(file_name);
        close(fd);
    }
    fail();
}

struct buff {
	int len;
	const char *cmp;
};

ssize_t custom_lyd_print_clb(void *arg, const void *buf, size_t count) {
    int rc;
    int len;
    struct buff *pos = arg;

    len = pos->len + count;

    const char *chunk = &pos->cmp[pos->len];

    rc = strncmp(chunk, buf, count);
    if (rc) {
	    fail();
    }

    pos->len = len;
}

static void
test_lyd_print_clb_xml(void **state)
{
    (void) state; /* unused */
    char *result;
    int rc;

    struct buff *buf = calloc(1, sizeof(struct buff));
    if (!buf) {
        fail();
    }

    buf->len = 0;
    buf->cmp = result_xml;
    void *arg = buf;

    rc = lyd_print_clb(custom_lyd_print_clb, arg, root, LYD_XML, 0);
    if (rc) {
        fail();
        free(buf);
    }

    free(buf);
}

static void
test_lyd_print_clb_xml_format(void **state)
{
    (void) state; /* unused */
    char *result;
    int rc;

    struct buff *buf = calloc(1, sizeof(struct buff));
    if (!buf) {
        fail();
    }

    buf->len = 0;
    buf->cmp = result_xml_format;
    void *arg = buf;

    rc = lyd_print_clb(custom_lyd_print_clb, arg, root, LYD_XML_FORMAT, 0);
    if (rc) {
        fail();
        free(buf);
    }

    free(buf);
}

static void
test_lyd_print_clb_json(void **state)
{
    (void) state; /* unused */
    char *result;
    int rc;

    struct buff *buf = calloc(1, sizeof(struct buff));
    if (!buf) {
        fail();
    }

    buf->len = 0;
    buf->cmp = result_json;
    void *arg = buf;

    rc = lyd_print_clb(custom_lyd_print_clb, arg, root, LYD_JSON, 0);
    if (rc) {
        fail();
        free(buf);
    }

    free(buf);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_lyd_parse_mem),
        cmocka_unit_test(test_lyd_parse_fd),
        cmocka_unit_test(test_lyd_parse_path),
        cmocka_unit_test(test_lyd_parse_xml),
	cmocka_unit_test(test_lyd_new),
	cmocka_unit_test(test_lyd_new_leaf),
        cmocka_unit_test_setup_teardown(test_lyd_change_leaf, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_dup, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_insert, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_insert_before, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_insert_after, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_get_node, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_get_node_2, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_get_list_keys, setup_f_keys, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_validate, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_unlink, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_free, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_free_withsiblings, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_insert_attr, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_free_attr, setup_f, teardown_f),
	cmocka_unit_test_setup_teardown(test_lyd_print_mem_xml, setup_f, teardown_f),
	cmocka_unit_test_setup_teardown(test_lyd_print_mem_xml_format, setup_f, teardown_f),
	cmocka_unit_test_setup_teardown(test_lyd_print_mem_json, setup_f, teardown_f),
	cmocka_unit_test_setup_teardown(test_lyd_print_fd_xml, setup_f, teardown_f),
	cmocka_unit_test_setup_teardown(test_lyd_print_fd_xml_format, setup_f, teardown_f),
	cmocka_unit_test_setup_teardown(test_lyd_print_fd_json, setup_f, teardown_f),
	cmocka_unit_test_setup_teardown(test_lyd_print_clb_xml, setup_f, teardown_f),
	cmocka_unit_test_setup_teardown(test_lyd_print_clb_xml_format, setup_f, teardown_f),
	cmocka_unit_test_setup_teardown(test_lyd_print_clb_json, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
