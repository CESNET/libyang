/**
 * @file test_yang_data_ns.c
 * @author Peter Schoenmaker <pds@ntt.net>
 * @brief Cmocka tests for YANG data namespace.
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#include "tests/config.h"
#include "libyang.h"

struct state {
    struct ly_ctx *ctx;
  const struct lys_module *mod1, *mod2, *mod_nc;
    struct lyd_node *dt, *config, *node1, *node2, *nodet;;
};

static int
setup_f(void **state)
{
    struct state *st;
    const char *yang1 = "module yang-data-config-ns1 {\n"
                       "  prefix dcn1;\n"
                       "  namespace \"urn:cesnet:yang-data-config-ns1\";\n"
                       "  container con1 {\n"
                       "    leaf leaf1 {\n"
                       "        type string;\n"
                       "    }\n"
                       "  }\n"
                       "}";

    const char *yang2 = "module yang-data-config-ns2 {\n"
                       "  prefix dcn2;\n"
                       "  namespace \"urn:cesnet:yang-data-config-ns2\";\n"
                       "  container con2 {\n"
                       "    leaf leaf2 {\n"
                       "        type string;\n"
                       "    }\n"
                       "  }\n"
                       "}";

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(TESTS_DIR"/schema/yang/ietf/", 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        return -1;
    }

    st->mod1 = lys_parse_mem(st->ctx, yang1, LYS_IN_YANG);
    if (!st->mod1){
        fprintf(stderr, "Failed to load module 1.\n");
        return -1;
    }
    st->mod2 = lys_parse_mem(st->ctx, yang2, LYS_IN_YANG);
    if (!st->mod2){
        fprintf(stderr, "Failed to load module 2.\n");
        return -1;
    }
    return 0;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free_withsiblings(st->dt);
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_anydata_ns(void **state)
{
  struct state *st = (*state);
  struct lyd_attr *attr;

  char *s;
  const char *opentag = "<config xmlns=\"urn:ietf:params:xml:ns:netconf:base:1.0\" xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\">";

  st->mod_nc = ly_ctx_load_module(st->ctx, "ietf-netconf", NULL);
  assert_ptr_not_equal(st->mod_nc, NULL);

  st->node1 = lyd_new(NULL, st->mod1, "con1");
  assert_ptr_not_equal(st->node1, NULL);
    
  attr = lyd_insert_attr(st->node1, st->mod_nc, "operation", "create");
  assert_ptr_not_equal(attr, NULL);
  
  st->nodet = lyd_new_leaf(st->node1, st->mod1, "leaf1", "test1234");
  assert_ptr_not_equal(st->nodet, NULL);

  st->node2 = lyd_new(NULL, st->mod2, "con2");
  assert_ptr_not_equal(st->node2, NULL);  
  attr = lyd_insert_attr(st->node2, st->mod_nc, "operation", "merge");
  assert_ptr_not_equal(attr, NULL);

  st->nodet = lyd_new_leaf(st->node2, st->mod2, "leaf2", "leaf2-test1234");
  assert_ptr_not_equal(st->nodet, NULL);
  
  assert_return_code(lyd_insert_sibling(&(st->node1), st->node2), 0);

  st->dt = lyd_new_anydata(NULL, st->mod_nc, "config", st->node1, LYD_ANYDATA_DATATREE);
  assert_ptr_not_equal(st->dt, NULL);

  lyd_print_mem(&s, st->dt, LYD_XML, LYP_WITHSIBLINGS | LYP_FORMAT);
  assert_return_code(strncmp(s, opentag, strlen(opentag)), 0);
  
}

static void
test_print_xml_ns(void **state)
{
  struct state *st = (*state);

  
  assert_return_code(1, 0);
  assert_ptr_not_equal(123, 123);
}
/*
static void
test_new_path(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_new_path(NULL, st->ctx, "/yang-data:#node/test1/first", "test", 0 ,0);
    assert_ptr_equal(st->dt, NULL);
    st->dt = lyd_new_path(NULL, st->ctx, "/yang-data:#node/test1/first", "50", 0 ,0);
    assert_ptr_not_equal(st->dt, NULL);
    assert_ptr_not_equal(lyd_new_path(st->dt, st->ctx, "/yang-data:#node/test1/second", "test-second", 0 ,0), NULL);
    assert_ptr_equal(lyd_new_path(st->dt, st->ctx, "/yang-data:test1/third", "true", 0 ,0), NULL);
}

static void
test_new_yangdata(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_new_yangdata(st->mod, "node", "test");
    assert_ptr_not_equal(st->dt, NULL);
    lyd_new_leaf(st->dt, st->mod, "num", "25");
    assert_ptr_not_equal(st->dt->child, NULL);
    st->str1 = lyd_path(st->dt->child);
    assert_string_equal(st->str1, "/yang-data:#node/test/num");
}

static void
test_mem_yangdata(void **state)
{
    struct state *st = (*state);
    char *str;

    char *xml_file = "<errors xmlns=\"urn:cesnet:yang-data\">\n"
                     "  <error>\n"
                     "    <error-type>protocol</error-type>\n"
                     "    <error-tag>Bad packet</error-tag>\n"
                     "  </error>\n"
                     "</errors>\n"
                     "<node xmlns=\"urn:cesnet:yang-data\">\n"
                     "  <test>\n"
                     "    <num>50</num>\n"
                     "  </test>\n"
                     "</node>\n";
    char *json_file1 = "{\n"
                       "   \"yang-data:errors\" : {\n"
                       "     \"error\" : [\n"
                       "       {\n"
                       "         \"error-type\" : \"protocol\",\n"
                       "         \"error-tag\" : \"Bad packet\"\n"
                       "       }\n"
                       "     ]\n"
                       "   }\n"
                       "   \"yang-data:node\" : {\n"
                       "     \"test\" : {\n"
                       "       \"num\" : 50\n"
                       "     }\n"
                       "   }\n"
                       "}\n";
    char *json_file2 = "{\n"
                       "   \"yang-data:errors\" : {\n"
                       "     \"error\" : [\n"
                       "       {\n"
                       "         \"error-type\" : \"protocol\",\n"
                       "         \"error-tag\" : \"Bad packet\"\n"
                       "       }\n"
                       "     ]\n"
                       "   }\n"
                       "}\n";
    st->dt = lyd_parse_mem(st->ctx, json_file1, LYD_JSON, LYD_OPT_DATA_TEMPLATE, "data");
    assert_ptr_equal(st->dt, NULL);
    st->dt = lyd_parse_mem(st->ctx, json_file2, LYD_JSON, LYD_OPT_DATA_TEMPLATE, "data");
    assert_ptr_not_equal(st->dt, NULL);

    lyd_print_mem(&str, st->dt, LYD_LYB, 0);
    lyd_free(st->dt);
    st->dt = lyd_parse_mem(st->ctx, str, LYD_LYB, LYD_OPT_DATA_TEMPLATE, "data");
    free(str);
    assert_ptr_not_equal(st->dt, NULL);
    lyd_free(st->dt);

    st->dt = lyd_parse_mem(st->ctx, xml_file, LYD_XML, LYD_OPT_DATA_TEMPLATE | LYD_OPT_STRICT, "data");
    assert_ptr_equal(st->dt, NULL);
    st->dt = lyd_parse_mem(st->ctx, xml_file, LYD_XML, LYD_OPT_DATA_TEMPLATE, "data");
    assert_ptr_not_equal(st->dt, NULL);
}

static void
test_validate_yangdata(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_new_path(NULL, st->ctx, "/yang-data:#data/errors/error[1]/error-type", "transport", 0 ,0);
    assert_return_code(lyd_validate(&st->dt, LYD_OPT_DATA_TEMPLATE, NULL), EXIT_FAILURE);
    assert_ptr_not_equal(lyd_new_leaf(st->dt->child, st->mod, "error-tag", "Error"), NULL);
    assert_return_code(lyd_validate(&st->dt, LYD_OPT_DATA_TEMPLATE, NULL), EXIT_SUCCESS);
}

static void
test_path_yangdata(void **state)
{
    struct state *st = (*state);
    struct ly_set *set;
    const struct lys_node *node;
    char *str;

    st->dt = lyd_new_path(st->dt, st->ctx, "/yang-data:#node/test1/second", "test-second", 0 ,0);
    set = lyd_find_path(st->dt,"/yang-data:#node/test1/second");
    assert_ptr_not_equal(set, NULL);
    assert_int_equal(set->number, 1);

    st->str1 = lys_data_path(st->dt->child->schema);
    st->str2 = lyd_path(st->dt->child);
    assert_string_equal(st->str1, st->str2);
    assert_string_equal(st->str1, "/yang-data:#node/test1/second");

    str = st->str2;
    free(st->str1);
    st->str1 = lys_path(st->dt->child->schema, LYS_PATH_FIRST_PREFIX);
    st->str2 = ly_path_data2schema(st->ctx,str);
    free(str);
    assert_string_equal(st->str1, st->str2);
    assert_string_equal(st->str1, "/yang-data:#node/select/test1/test1/second");

    ly_set_free(set);
    set = lys_find_path(st->mod, NULL, st->str1);
    assert_ptr_not_equal(set, NULL);
    assert_int_equal(set->number, 1);

    ly_set_free(set);
    free(st->str1);
    st->str1 = lys_path(st->dt->child->schema, 0);
    set = ly_ctx_find_path(st->ctx, st->str1);
    assert_ptr_not_equal(set, NULL);
    assert_int_equal(set->number, 1);
    ly_set_free(set);

    free(st->str2);
    st->str2 = lyd_path(st->dt->child);
    node = ly_ctx_get_node(st->ctx, NULL, st->str2, 0);
    assert_ptr_not_equal(node, NULL);
}
*/
int main(void)
{
    const struct CMUnitTest tests[] = {
	cmocka_unit_test_setup_teardown(test_anydata_ns, setup_f, teardown_f),
	//	cmocka_unit_test_setup_teardown(test_print_xml_ns, setup_f, teardown_f),
	//        cmocka_unit_test_setup_teardown(test_new_yangdata, setup_f, teardown_f),
        //cmocka_unit_test_setup_teardown(test_path_yangdata, setup_f, teardown_f),
	//    cmocka_unit_test_setup_teardown(test_validate_yangdata, setup_f, teardown_f),
        //cmocka_unit_test_setup_teardown(test_mem_yangdata, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
