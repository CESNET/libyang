/*
 * @file test_internal_data.c
 * @author: Antonio Paunovic <antonio.paunovic@sartura.hr>
 * @brief unit tests for functions from tree_internal.h header
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
#include "../../src/tree_internal.h"


struct ly_ctx *ctx = NULL;
struct lyd_node *root = NULL;

const char *a_data_xml =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>    \
<submodule name=\"asub\"                               \
           xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" \
           xmlns:a=\"urn:a\">                          \
  <belongs-to module=\"a\">                            \
    <prefix value=\"a_pref\"/>                         \
  </belongs-to>                                        \
  <include module=\"atop\"/>                           \
  <feature name=\"not-working-with-fox\"/>             \
                                                       \
  <notification name=\"bar-notif\">                    \
    <if-feature name=\"bar\"/>                         \
  </notification>                                      \
  <notification name=\"fox-notif\">                    \
    <if-feature name=\"fox\"/>                         \
  </notification>                                      \
  <augment target-node=\"/a_pref:top\">                \
    <if-feature name=\"bar\"/>                         \
    <container name=\"bar-sub\"/>                      \
  </augment>                                           \
  <augment target-node=\"/top\">                       \
    <container name=\"bar-sub2\"/>                     \
  </augment>                                           \
</submodule>";

const char *a_data_xml_attr = "\
<x xmlns=\"urn:a\" bubba=\"test\">\n\
  <bubba xmlns=\"urn:a\" name=\"test\"/>\n\
</x>\n";

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
setup_f(void **state, char *yang_file, char *yang_folder)
{
    (void) state; /* unused */
    int rc;

    rc = generic_init(yang_file, yang_folder);

    if (rc) {
        return -1;
    }

    return 0;
}

static int
setup_mod_a(void **state)
{
    return setup_f(state, TESTS_DIR"/internal/files/a.yin", TESTS_DIR"/internal/files");
}

static int
setup_mod_b(void **state)
{
    return setup_f(state, TESTS_DIR"/internal/files/b.yin", TESTS_DIR"/internal/files");
}

static int
setup_mod_c(void **state)
{
    return setup_f(state, TESTS_DIR"/internal/files/c.yin", TESTS_DIR"/internal/files");
}
static int
setup_mod_d(void **state)
{
    return setup_f(state, TESTS_DIR"/internal/files/d.yin", TESTS_DIR"/internal/files");
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
test_lys_submodule_parse(void **state)
{
    (void) state; /* unused */
    struct lys_submodule *submodule;
    struct lys_module *module;
    const char *mod_name = "a";
    const char *sub_name = "asub";
    struct unres_schema *unres;

    module = (struct lys_module*)ly_ctx_get_module(ctx, mod_name, NULL);
    assert_non_null(module);

    unres = calloc(1, sizeof(*unres));
    submodule = lys_submodule_parse(module, a_data_xml, LYS_YIN, unres);

    assert_non_null(submodule);
    assert_string_equal(sub_name, submodule->name);

    free(unres);
    lys_submodule_module_data_free(submodule);
    lys_submodule_free(submodule, NULL);
}

void
test_lys_submodule_read(void **state)
{
    (void) state; /* unused */
    struct lys_submodule *submodule;
    struct lys_module *module;
    const char *mod_name = "a";
    const char *sub_name = "asub";
    struct unres_schema *unres;
    int fd;
    const char *fname = TESTS_DIR"/internal/files/asub2.yin";

    module = (struct lys_module*)ly_ctx_get_module(ctx, mod_name, NULL);
    assert_non_null(module);

    unres = calloc(1, sizeof(*unres));
    fd = open(fname, O_RDONLY);
    submodule = lys_submodule_read(module, fd, LYS_YIN, unres);

    assert_non_null(submodule);
    assert_string_equal(sub_name, submodule->name);

    free(unres);
    close(fd);
    lys_submodule_free(submodule, NULL);
    lys_submodule_module_data_free(submodule);
    lys_free(module, NULL, 1);
}

void
test_lys_node_addchild(void **state)
{
    (void) state; /* unused */
    struct lys_node *parent, *child;
    struct lys_module *module;
    int rc = 0;

    module =(struct lys_module*)ly_ctx_get_module(ctx, "d", NULL);
    assert_non_null(module);

    parent = (struct lys_node *)ly_ctx_get_node(ctx, NULL, "/d:y");
    assert_non_null(parent);
    child = (struct lys_node *)ly_ctx_get_node(ctx, NULL, "/d:z/not");
    assert_non_null(child);

    rc = lys_node_addchild(parent, module, child);
    assert_true(rc == 0);
    assert_string_equal("not", parent->child->next->next->name);

    lys_node_free(child, NULL, 0);
    lys_node_free(parent, NULL, 0);
}

void
test_lys_find_grouping_up(void **state)
{
    (void) state; /* unused */
    struct lys_node *child, *node;
    struct lys_node_grp *grp;

    child = (struct lys_node *)ly_ctx_get_node(ctx, NULL, "/a:x");
    assert_non_null(child);

    grp = lys_find_grouping_up("gg", child);
    assert_non_null(grp);
    assert_string_equal(grp->name, "gg");

    node = (struct lys_node *)ly_ctx_get_node(ctx, NULL, "/a:x/bar-gggg");
    assert_non_null(node);
    assert_string_equal(node->name, grp->child->name);
}

void
test_lys_node_dup(void **state)
{
    (void) state; /* unused */

    struct unres_schema *unres;
    struct lys_node *parent, *new_node;
    struct lys_module *module;
    const char *nodeid;
    const char *test_word = "freedom";

    unres = calloc(1, sizeof(*unres));
    module = (struct lys_module*)ly_ctx_get_module(ctx, "a", NULL);
    assert_non_null(module);

    nodeid = "/a:x";
    parent = (struct lys_node *)ly_ctx_get_node(ctx, NULL, nodeid);
    assert_non_null(parent);
    new_node = parent->child;

    new_node->name = "freedom";
    new_node = lys_node_dup(module, NULL, new_node, 0, 0, unres, 0);
    assert_non_null(new_node);

    parent = (struct lys_node *)ly_ctx_get_node(ctx, NULL, "/a:freedom");
    assert_non_null(parent);
    assert_string_equal(test_word, parent->name);

    free(unres);
}

void
test_lys_node_switch(void **state)
{
    (void) state; /* unused */
    struct lys_module *module;
    struct lys_node *src;
    struct lys_node *dst;
    struct unres_schema *unres;
    struct lys_node *parent;

    unres = calloc(1, sizeof(*unres));
    module = (struct lys_module*)ly_ctx_get_module(ctx, "a", NULL);
    assert_non_null(module);

    parent = (struct lys_node *)ly_ctx_get_node(ctx, NULL, "/a:x");
    dst = (struct lys_node *)ly_ctx_get_node(ctx, NULL, "/a:x/baz");

    src = lys_node_dup((struct lys_module*)module, parent, dst, 0, 0, unres, 1);

    assert_ptr_not_equal(NULL, dst->parent);
    assert_ptr_not_equal(NULL, dst->next);
    assert_ptr_equal(parent, dst->parent);

    lys_node_switch(dst, src);

    assert_ptr_equal(NULL, dst->parent);
    assert_ptr_equal(NULL, dst->next);
    assert_ptr_not_equal(parent, dst->parent);

    free(unres);
    lys_node_free(dst, NULL, 0);
}

void
test_ly_check_mandatory(void **state)
{
    (void) state; /* unused */
    int status, rpc_output, rc;
    struct lyd_node *data = NULL;

    const char *mand_data =
        "<y xmlns=\"urn:d\">"
        "<why>work?</why>"
        "</y>";

    struct lys_node *schema = (struct lys_node *)ly_ctx_get_node(ctx, NULL, "/d:y");
    assert_non_null(schema);

    data = lyd_parse_mem(ctx, mand_data, LYD_XML, LYD_OPT_CONFIG);
    assert_non_null(data);

    status = rpc_output = 0;
    rc = ly_check_mandatory(data, schema, status, rpc_output);
    assert_int_equal(rc, EXIT_SUCCESS);

    lyd_free(data);
}


void
test_lyd_attr_parent(void **state)
{
    (void) state; /* unused */

    struct lyxml_elem *xml = NULL;
    const char *result = NULL;
    struct lyd_attr *attr = NULL;
    struct lyd_node *node;

    root = ly_ctx_info(ctx);
    xml = lyxml_parse_mem(ctx, a_data_xml_attr, 0);
    assert_non_null(xml);

    result = lyxml_get_attr(xml, "bubba", NULL);
    assert_non_null(result);

    attr = lyd_insert_attr(root, NULL, "test", "test");

    node =  (struct lyd_node *)lyd_attr_parent(root, attr);
    assert_ptr_equal(node, root);

    lyxml_free(ctx, xml);
    lyd_free_attr(ctx, node, attr, 0);
    lyd_free(node);
}

void
test_lys_get_import_module(void **state)
{
    (void) state; /* unused */
    const struct lys_module *cmodule, *amodule, *import_mod;

    cmodule = amodule = import_mod = NULL;
    amodule =ly_ctx_get_module(ctx, "a", NULL);
    assert_non_null(amodule);

    cmodule =ly_ctx_get_module(ctx, "c", NULL);
    assert_non_null(cmodule);

    import_mod = lys_get_import_module(cmodule, "a_mod", 0, NULL, 0);
    assert_non_null(import_mod);
    assert_string_equal("a", import_mod->name);
    assert_ptr_equal(import_mod, lys_get_import_module(cmodule, NULL, 0, "a", 1));
}

void
test_lys_get_sibiling(void **state)
{
    (void) state; /* unused */
    int rc = 0;
    LYS_NODE type = 0;
    const char *mod_name = "b";
    struct lys_node *siblings , *ret;


    siblings = (struct lys_node *)ly_ctx_get_node(ctx, NULL, "/b:x/bar-leaf");

    rc = lys_get_sibling(siblings, mod_name, strlen(mod_name), "bar-leaf", 0, type, (const struct lys_node **)&ret);
    assert_true(!rc);
    assert_ptr_equal(siblings, ret);

    rc = lys_get_sibling(siblings, mod_name, 1, "baz", 0, 0, (const struct lys_node **)&ret);
    assert_string_equal("baz", ret->name);

    lys_node_free(siblings, NULL, 0);
    lys_node_free(ret, NULL, 0);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_lys_submodule_parse, setup_mod_a, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_submodule_read, setup_mod_a, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_node_addchild, setup_mod_d, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_node_dup, setup_mod_a, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_node_switch, setup_mod_a, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_find_grouping_up, setup_mod_a, teardown_f),
        cmocka_unit_test_setup_teardown(test_ly_check_mandatory, setup_mod_d, teardown_f),
        cmocka_unit_test_setup_teardown(test_lyd_attr_parent, setup_mod_a, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_get_import_module, setup_mod_c, teardown_f),
        cmocka_unit_test_setup_teardown(test_lys_get_sibiling, setup_mod_b, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
