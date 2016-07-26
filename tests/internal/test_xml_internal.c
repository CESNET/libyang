/*
 * @file test_internal_data.c
 * @author: Antonio Paunovic <antonio.paunovic@sartura.hr>
 * @brief unit tests for functions from xml_internal.h header
 *
 * Copyright (C) 2016 Deutsche Telekom AG.
 *
 * Author: Antonio Paunovic <antonio.paunovic@sartura.hr>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
#include "../../src/xml_internal.h"


struct ly_ctx *ctx = NULL;
struct lyd_node *root = NULL;

const char *a_data_xml = "\
<x xmlns=\"urn:a\">\n\
  <bubba>meaningful test</bubba>\n\
  </x>";

const char *a_data_bad_ns_xml = "\
<y xmlns=\"urn:a\">\n\
  <bubba>meaningful test</bubba>\n\
  </y>\n";

int
generic_init(char *yang_file, char *yang_folder)
{
    LYS_INFORMAT yang_format;
    char *schema = NULL;
    struct stat sb_schema;
    int fd = -1;

    if (!yang_file || !yang_folder) {
        goto error;
    }

    yang_format = LYS_IN_YIN;

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

    if (!lys_parse_mem(ctx, schema, yang_format)) {
        goto error;
    }

    /* cleanup */
    munmap(schema, sb_schema.st_size);

    return 0;

error:
    if (schema) {
        munmap(schema, sb_schema.st_size);
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
    char *yang_file = TESTS_DIR"/internal/files/a.yin";
    char *yang_folder = TESTS_DIR"/internal/files";
    int rc;

    rc = generic_init(yang_file, yang_folder);

    if (rc) {
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    (void) state; /* unused */
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    return 0;
}

void
test_lyxml_add_child(void **state)
{
    (void) state; /* unused */
    struct lyxml_elem *parent, *child;
    const char *path = TESTS_DIR"/internal/files/a.xml";
    int rc;
    struct lyxml_elem *p;

    parent = lyxml_parse_path(ctx, path, 0);
    child = lyxml_parse_mem(ctx, a_data_xml, 0);

    assert_non_null(parent);
    assert_non_null(child);

    rc = lyxml_add_child(ctx, parent, child);
    assert_int_equal(rc, EXIT_SUCCESS);

    /* get to the last child. */
    p = parent;
    while((p = p->child) && (p->child != NULL));

    assert_string_equal(p->next->name, child->name);
    assert_ptr_equal(p->next, child);

    lyxml_free(ctx, child);
    lyxml_free(ctx, p);
    lyxml_free(ctx, parent);
}

void
test_lyxml_correct_elem_ns(void **state)
{
    (void) state; /* unused */
    struct lyxml_elem *elem;
    int copy_ns, corr_attrs;

    elem = lyxml_parse_mem(ctx, a_data_bad_ns_xml, 0);

    copy_ns = 1, corr_attrs = 0;
    lyxml_correct_elem_ns(ctx, elem, copy_ns, corr_attrs);
    assert_string_equal("urn:a", elem->ns->value);

    copy_ns = 0, corr_attrs = 0;
    lyxml_correct_elem_ns(ctx, elem, copy_ns, corr_attrs);
    assert_null(elem->ns);

    lyxml_free(ctx, elem);
}

void
test_lyxml_dup_elem(void **state)
{
    (void) state; /* unused */
    const char *path = TESTS_DIR"/internal/files/b_example.xml";
    struct lyxml_elem *elem, *parent, *dup;
    int recursive;

    parent = lyxml_parse_path(ctx, path, 0);
    elem = parent->child->next;


    recursive = 1;
    dup = lyxml_dup_elem(ctx, elem, parent, recursive);

    assert_string_equal("x", parent->name);
    assert_string_equal("bubba", parent->child->name);
    assert_string_equal("y", elem->name);
    assert_string_equal("y", dup->name);
    assert_string_equal("bar", dup->child->name);
    assert_string_equal(elem->child->name, dup->child->name);

    recursive = 0;
    dup = lyxml_dup_elem(ctx, elem, parent, recursive);

    assert_string_equal("x", parent->name);
    assert_string_equal("bubba", parent->child->name);
    assert_string_equal("y", elem->name);
    assert_string_equal("y", dup->name);
    assert_null(dup->child);

    lyxml_free(ctx, elem);
    lyxml_free(ctx, dup);
    lyxml_free(ctx, parent);
}

void
test_lyxml_parse_elem(void **state)
{
    (void) state; /* unused */
    struct lyxml_elem *elem, *parent;
    uint len;

    parent = lyxml_parse_mem(ctx, a_data_xml, 0);
    elem = lyxml_parse_elem(ctx, a_data_xml, &len, parent);

    assert_non_null(elem);
    assert_string_equal(elem->name, parent->name);
    assert_ptr_not_equal(elem, parent);
    assert_int_equal(len, strlen(a_data_xml));

    lyxml_free(ctx, elem);
    lyxml_free(ctx, parent);
}

void
test_lyxml_free_attr(void **state)
{
    (void) state; /* unused */
    const char *path = TESTS_DIR"/internal/files/b_example.xml";
    const char *name = "style", *ns = NULL , *attr_val = NULL;
    struct lyxml_elem *parent, *elem;
    struct lyxml_attr *attr;

    parent = lyxml_parse_path(ctx, path, 0);
    assert_non_null(parent);
    assert_string_equal("x", parent->name);

    elem = parent->child->next->next;
    attr = elem->attr;

    assert_non_null(attr);

    attr_val = lyxml_get_attr(elem, name, ns);
    assert_string_equal("veggies", attr_val);

    lyxml_free_attr(ctx, elem, attr);

    attr_val = lyxml_get_attr(elem, name, ns);
    assert_null(attr_val);

    lyxml_free(ctx, elem);
    lyxml_free(ctx, parent);
}

void
test_lyxml_free_attrs(void **state)
{
    (void) state; /* unused */
    const char *name = "style", *ns = NULL;
    struct lyxml_elem *parent, *elem;
    struct lyxml_attr *attr = NULL;
    char *value;
    int *ptr;

    const char *data =
        "<x xmlns=\"urn:b\">"
          "<bubba>food</bubba>"
          "<y>"
            "<bar>tender</bar>"
          "</y>"
          "<baz style=\"veggies\">cheese</baz>"
        "</x>";

    parent = lyxml_parse_mem(ctx, data, 0);
    assert_non_null(parent);
    assert_string_equal("x", parent->name);

    elem = parent->child->next->next;
    attr = elem->attr;
    assert_non_null(attr);

    value = (char*)lyxml_get_attr(elem, name, ns);
    assert_string_equal("veggies", value);
    ptr = (int*)elem->attr->value;

    lyxml_free_attrs(ctx, elem);
    assert_ptr_not_equal(ptr, elem->attr);

    elem->attr = NULL;

    lyxml_free(ctx, parent);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_lyxml_add_child, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_correct_elem_ns, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_parse_elem, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_dup_elem, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_free_attr, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyxml_free_attrs, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
