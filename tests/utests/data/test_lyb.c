/**
 * @file test_lyb.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Cmocka tests for LYB binary data format.
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <inttypes.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>

#include "hash_table.h"
#include "libyang.h"
#include "tests/config.h"

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *dt1, *dt2;
    char *mem;
};

static void
check_data_tree_next(struct lyd_node **start, struct lyd_node **next, struct lyd_node **elem)
{
    if (*elem) {
        goto loop_next;
    }

loop_begin:
    LYD_TREE_DFS_BEGIN(*start, *next, *elem) {
        return;
loop_next:
        LYD_TREE_DFS_END(*start, *next, *elem);
    }

    if (!*next) {
        /* top-level siblings */
        *start = (*start)->next;
        if (!(*start)) {
            *elem = NULL;
            return;
        }
        goto loop_begin;
    }

    return;
}

static void
check_data_tree(struct lyd_node *root1, struct lyd_node *root2)
{
    struct lyd_node *next1, *next2, *elem1 = NULL, *elem2 = NULL, *iter;
    struct lyd_meta *meta1, *meta2;
    struct lyd_node_inner *in1, *in2;
    uint32_t i1, i2;

    for (check_data_tree_next(&root1, &next1, &elem1), check_data_tree_next(&root2, &next2, &elem2);
         elem1 && elem2;
         check_data_tree_next(&root1, &next1, &elem1), check_data_tree_next(&root2, &next2, &elem2)) {

        if (elem1->schema != elem2->schema) {
            fprintf(stderr, "Schema mismatch (\"%s\" and \"%s\").\n", elem1->schema->name, elem2->schema->name);
            fail();
        }

        /* check common data node attributes */
        if (elem1->flags != elem2->flags) {
            fprintf(stderr, "\"%s\": flags mismatch (\"%u\" and \"%u\").\n", elem1->schema->name, elem1->flags, elem2->flags);
            fail();
        }

        /* check data node attributes */
        for (meta1 = elem1->meta, meta2 = elem2->meta; meta1 && meta2; meta1 = meta1->next, meta2 = meta2->next) {
            if (meta1->annotation != meta2->annotation) {
                fprintf(stderr, "\"%s\": meta annotation mismatch.\n", elem1->schema->name);
                fail();
            }
            if (strcmp(meta1->name, meta2->name)) {
                fprintf(stderr, "\"%s\": meta name mismatch (\"%s\" and \"%s\").\n", elem1->schema->name, meta1->name, meta2->name);
                fail();
            }
            if (lyd_compare_meta(meta1, meta2)) {
                fprintf(stderr, "\"%s\": meta value mismatch.\n", elem1->schema->name);
                fail();
            }
        }
        if (meta1) {
            fprintf(stderr, "\"%s\": meta mismatch (\"%s\" and \"NULL\").\n", elem1->schema->name, meta1->name);
            fail();
        }
        if (meta2) {
            fprintf(stderr, "\"%s\": meta mismatch (\"NULL\" and \"%s\").\n", elem1->schema->name, meta2->name);
            fail();
        }

        /* check specific data node attributes */
        switch (elem1->schema->nodetype) {
        case LYS_CONTAINER:
        case LYS_LIST:
        case LYS_RPC:
        case LYS_ACTION:
        case LYS_NOTIF:
            in1 = (struct lyd_node_inner *)elem1;
            in2 = (struct lyd_node_inner *)elem2;

            i1 = 0;
            LY_LIST_FOR(in1->child, iter) {
                ++i1;
            }

            i2 = 0;
            LY_LIST_FOR(in2->child, iter) {
                ++i2;
            }

            if (i1 != i2) {
                fprintf(stderr, "\"%s\": child count mismatch (%u and %u).\n", elem1->schema->name, i1, i2);
                fail();
            }

            if (i1 >= LYD_HT_MIN_ITEMS) {
                if (!in1->children_ht || !in2->children_ht) {
                    fprintf(stderr, "\"%s\": missing hash table (%p and %p).\n", elem1->schema->name, in1->children_ht,
                            in2->children_ht);
                    fail();
                }

                LY_LIST_FOR(in1->child, iter) {
                    if (lyht_find(in1->children_ht, &iter, iter->hash, NULL)) {
                        fprintf(stderr, "\"%s\": missing child \"%s\" in the hash table 1.\n", elem1->schema->name, iter->schema->name);
                        fail();
                    }
                }
                LY_LIST_FOR(in2->child, iter) {
                    if (lyht_find(in2->children_ht, &iter, iter->hash, NULL)) {
                        fprintf(stderr, "\"%s\": missing child \"%s\" in the hash table 2.\n", elem1->schema->name, iter->schema->name);
                        fail();
                    }
                }
            }
            break;
        case LYS_LEAF:
        case LYS_LEAFLIST:
        case LYS_ANYDATA:
        case LYS_ANYXML:
            if (lyd_compare_single(elem1, elem2, 0)) {
                fprintf(stderr, "\"%s\": value mismatch.\n", elem1->schema->name);
                fail();
            }
            break;
        default:
            fprintf(stderr, "Unexpected data node type.\n");
            fail();
        }

        if (!elem1->hash) {
            fprintf(stderr, "\"%s\": hash not calculated.\n", elem1->schema->name);
            fail();
        }
        if (elem1->hash != elem2->hash) {
            fprintf(stderr, "\"%s\": hashes do not match (%u and %u).\n", elem1->schema->name, elem1->hash, elem2->hash);
            fail();
        }
    }

    if (elem1) {
        fprintf(stderr, "Schema mismatch (\"%s\" and \"NULL\").\n", elem1->schema->name);
        fail();
    }
    if (elem2) {
        fprintf(stderr, "Schema mismatch (\"NULL\" and \"%s\").\n", elem2->schema->name);
        fail();
    }
}

static int
setup_f(void **state)
{
    struct state *st;

    (*state) = st = calloc(1, sizeof *st);
    assert_non_null(st);

    /* libyang context */
    assert_int_equal(ly_ctx_new(TESTS_DIR_MODULES_YANG, 0, &st->ctx), LY_SUCCESS);

    return 0;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free_siblings(st->dt1);
    lyd_free_siblings(st->dt2);
    ly_ctx_destroy(st->ctx, NULL);
    free(st->mem);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
test_ietf_interfaces(void **state)
{
    struct state *st = (*state);
    int ret;
    const char *data_xml =
    "<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">"
        "<interface>"
            "<name>eth0</name>"
            "<description>Ethernet 0</description>"
            "<type xmlns:ianaift=\"urn:ietf:params:xml:ns:yang:iana-if-type\">ianaift:ethernetCsmacd</type>"
            "<enabled>true</enabled>"
            "<ipv4 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">"
                "<enabled>true</enabled>"
                "<mtu>1500</mtu>"
                "<address>"
                    "<ip>192.168.2.100</ip>"
                    "<prefix-length>24</prefix-length>"
                "</address>"
            "</ipv4>"
        "</interface>"
        "<interface>"
            "<name>eth1</name>"
            "<description>Ethernet 1</description>"
            "<type xmlns:ianaift=\"urn:ietf:params:xml:ns:yang:iana-if-type\">ianaift:ethernetCsmacd</type>"
            "<enabled>true</enabled>"
            "<ipv4 xmlns=\"urn:ietf:params:xml:ns:yang:ietf-ip\">"
                "<enabled>true</enabled>"
                "<mtu>1500</mtu>"
                "<address>"
                    "<ip>10.10.1.5</ip>"
                    "<prefix-length>16</prefix-length>"
                "</address>"
            "</ipv4>"
        "</interface>"
        "<interface>"
            "<name>gigaeth0</name>"
            "<description>GigabitEthernet 0</description>"
            "<type xmlns:ianaift=\"urn:ietf:params:xml:ns:yang:iana-if-type\">ianaift:ethernetCsmacd</type>"
            "<enabled>false</enabled>"
        "</interface>"
    "</interfaces>";


    assert_non_null(ly_ctx_load_module(st->ctx, "ietf-ip", NULL));
    assert_non_null(ly_ctx_load_module(st->ctx, "iana-if-type", NULL));

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, data_xml, LYD_XML, LYD_PARSE_ONLY, 0, &st->dt1));
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYD_PRINT_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, st->mem, LYD_LYB, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, &st->dt2));
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

static void
test_origin(void **state)
{
    struct state *st = (*state);
    int ret;
    const char *origin_yang =
    "module test-origin {"
    "   namespace \"urn:test-origin\";"
    "   prefix to;"
    "   import ietf-origin {"
    "       prefix or;"
    "   }"
    ""
    "   container cont {"
    "       leaf leaf1 {"
    "           type string;"
    "       }"
    "       leaf leaf2 {"
    "           type string;"
    "       }"
    "       leaf leaf3 {"
    "           type uint8;"
    "       }"
    "   }"
    "}";
    const char *data_xml =
    "<cont xmlns=\"urn:test-origin\">"
        "<leaf1 xmlns:or=\"urn:ietf:params:xml:ns:yang:ietf-origin\" or:origin=\"or:default\">value1</leaf1>"
        "<leaf2>value2</leaf2>"
        "<leaf3 xmlns:or=\"urn:ietf:params:xml:ns:yang:ietf-origin\" or:origin=\"or:system\">125</leaf3>"
    "</cont>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(st->ctx, origin_yang, LYS_IN_YANG, NULL));
    lys_set_implemented(ly_ctx_get_module_latest(st->ctx, "ietf-origin"));

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, data_xml, LYD_XML, LYD_PARSE_ONLY, 0, &st->dt1));
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYD_PRINT_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, st->mem, LYD_LYB, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, &st->dt2));
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

static void
test_statements(void **state)
{
    struct state *st = (*state);
    int ret;
    const char *links_yang =
    "module links {"
        "yang-version 1.1;"
        "namespace \"urn:module2\";"
        "prefix mod2;"

        "identity just-another-identity {"

        "}"

        "leaf one-leaf {"
            "type string;"
        "}"

        "list list-for-augment {"
            "key keyleaf;"

            "leaf keyleaf {"
                "type string;"
            "}"

            "leaf just-leaf {"
                "type int32;"
            "}"
        "}"

        "leaf rleaf {"
            "type string;"
        "}"

        "leaf-list llist {"
            "type string;"
            "min-elements 0;"
            "max-elements 100;"
            "ordered-by user;"
        "}"

        "grouping rgroup {"
            "leaf rg1 {"
                "type string;"
            "}"

            "leaf rg2 {"
                "type string;"
            "}"
        "}"
    "}";

    const char *statements_yang =
    "module statements {"
        "namespace \"urn:module\";"
        "prefix mod;"
        "yang-version 1.1;"

        "import links {"
            "prefix mod2;"
        "}"

        "identity random-identity {"
            "base \"mod2:just-another-identity\";"
            "base \"another-identity\";"
        "}"

        "identity another-identity {"
            "base \"mod2:just-another-identity\";"
        "}"

        "typedef percent {"
            "type uint8 {"
                "range \"0 .. 100\";"
            "}"
            "units percent;"
        "}"

        "container ice-cream-shop {"
            "container employees {"
                "list employee {"
                    "config true;"
                    "key id;"
                    "unique name;"
                    "min-elements 0;"
                    "max-elements 100;"

                    "leaf id {"
                        "type uint64;"
                        "mandatory true;"
                    "}"

                    "leaf name {"
                        "type string;"
                    "}"

                    "leaf age {"
                        "type uint32;"
                    "}"
                "}"
            "}"
        "}"

        "container random {"
            "choice switch {"
                "case a {"
                    "leaf aleaf {"
                        "type string;"
                        "default aaa;"
                    "}"
                "}"

                "case c {"
                    "leaf cleaf {"
                        "type string;"
                    "}"
                "}"
            "}"

            "anyxml xml-data;"
            "anydata any-data;"
            "leaf-list leaflist {"
                "type string;"
                "min-elements 0;"
                "max-elements 20;"
                "ordered-by system;"
            "}"

            "grouping group {"
                "leaf g1 {"
                    "mandatory false;"
                    "type percent;"
                "}"

                "leaf g2 {"
                    "type string;"
                "}"
            "}"

            "uses group;"
            "uses mod2:rgroup;"

            "leaf lref {"
                "type leafref {"
                    "path \"/mod2:one-leaf\";"
                "}"
            "}"

            "leaf iref {"
                "type identityref {"
                    "base \"mod2:just-another-identity\";"
                "}"
            "}"
        "}"

        "augment \"/random\" {"
            "leaf aug-leaf {"
                "type string;"
            "}"
        "}"
    "}";

    const char *data_xml =
    "<ice-cream-shop xmlns=\"urn:module\">"
        "<employees>"
            "<employee>"
                "<id>0</id>"
                "<name>John Doe</name>"
                "<age>28</age>"
            "</employee>"
            "<employee>"
                "<id>1</id>"
                "<name>Dohn Joe</name>"
                "<age>20</age>"
            "</employee>"
        "</employees>"
    "</ice-cream-shop>"
    "<one-leaf xmlns=\"urn:module2\">reference leaf</one-leaf>"
    "<random xmlns=\"urn:module\">"
        "<aleaf>string</aleaf>"
        "<xml-data><anyxml>data</anyxml></xml-data>"
        "<any-data><data>any data</data></any-data>"
        "<leaflist>l0</leaflist>"
        "<leaflist>l1</leaflist>"
        "<leaflist>l2</leaflist>"
        "<g1>40</g1>"
        "<g2>string</g2>"
        "<aug-leaf>string</aug-leaf>"
        "<rg1>string</rg1>"
        "<rg2>string</rg2>"
        "<lref>reference leaf</lref>"
        "<iref>random-identity</iref>"
    "</random>";

    assert_int_equal(LY_SUCCESS, lys_parse_mem(st->ctx, links_yang, LYS_IN_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lys_parse_mem(st->ctx, statements_yang, LYS_IN_YANG, NULL));

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, data_xml, LYD_XML, LYD_PARSE_ONLY, 0, &st->dt1));
    assert_ptr_not_equal(st->dt1, NULL);

    ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYD_PRINT_WITHSIBLINGS);
    assert_int_equal(ret, 0);

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(st->ctx, st->mem, LYD_LYB, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, &st->dt2));
    assert_ptr_not_equal(st->dt2, NULL);

    check_data_tree(st->dt1, st->dt2);
}

// static void
// test_types(void **state)
// {
//     struct state *st = (*state);
//     int ret;
//
//     ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
//     assert_non_null(ly_ctx_load_module(st->ctx, "types", NULL));
//
//     st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/types.xml", LYD_XML, LYD_OPT_CONFIG);
//     assert_ptr_not_equal(st->dt1, NULL);
//
//     ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
//     assert_int_equal(ret, 0);
//
//     st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
//     assert_ptr_not_equal(st->dt2, NULL);
//
//     check_data_tree(st->dt1, st->dt2);
// }
//
// static void
// test_annotations(void **state)
// {
//     struct state *st = (*state);
//     int ret;
//
//     ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
//     assert_non_null(ly_ctx_load_module(st->ctx, "annotations", NULL));
//
//     st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/annotations.xml", LYD_XML, LYD_OPT_CONFIG);
//     assert_ptr_not_equal(st->dt1, NULL);
//
//     ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
//     assert_int_equal(ret, 0);
//
//     st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
//     assert_ptr_not_equal(st->dt2, NULL);
//
//     check_data_tree(st->dt1, st->dt2);
// }
//
// static void
// test_similar_annot_names(void **state)
// {
//     struct state *st = (*state);
//     int ret;
//
//     ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
//     assert_non_null(ly_ctx_load_module(st->ctx, "annotations", NULL));
//
//     st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/similar-annot-names.xml", LYD_XML, LYD_OPT_CONFIG);
//     assert_ptr_not_equal(st->dt1, NULL);
//
//     ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
//     assert_int_equal(ret, 0);
//
//     st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
//     assert_ptr_not_equal(st->dt2, NULL);
//
//     check_data_tree(st->dt1, st->dt2);
// }
//
// static void
// test_many_child_annot(void **state)
// {
//     struct state *st = (*state);
//     int ret;
//
//     ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
//     assert_non_null(ly_ctx_load_module(st->ctx, "annotations", NULL));
//
//     st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/many-childs-annot.xml", LYD_XML, LYD_OPT_CONFIG);
//     assert_ptr_not_equal(st->dt1, NULL);
//
//     ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
//     assert_int_equal(ret, 0);
//
//     st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
//     assert_ptr_not_equal(st->dt2, NULL);
//
//     check_data_tree(st->dt1, st->dt2);
// }
//
// static void
// test_union(void **state)
// {
//     struct state *st = (*state);
//     int ret;
//
//     ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
//     assert_non_null(ly_ctx_load_module(st->ctx, "union", NULL));
//
//     st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/union.xml", LYD_XML, LYD_OPT_CONFIG);
//     assert_ptr_not_equal(st->dt1, NULL);
//
//     ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
//     assert_int_equal(ret, 0);
//
//     st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
//     assert_ptr_not_equal(st->dt2, NULL);
//
//     check_data_tree(st->dt1, st->dt2);
// }
//
// static void
// test_union2(void **state)
// {
//     struct state *st = (*state);
//     int ret;
//
//     ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
//     assert_non_null(ly_ctx_load_module(st->ctx, "statements", NULL));
//
//     st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/union2.xml", LYD_XML, LYD_OPT_CONFIG);
//     assert_ptr_not_equal(st->dt1, NULL);
//
//     ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
//     assert_int_equal(ret, 0);
//
//     st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
//     assert_ptr_not_equal(st->dt2, NULL);
//
//     check_data_tree(st->dt1, st->dt2);
// }
//
// static void
// test_collisions(void **state)
// {
//     struct state *st = (*state);
//     int ret;
//
//     ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
//     assert_non_null(ly_ctx_load_module(st->ctx, "annotations", NULL));
//
//     st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/collisions.xml", LYD_XML, LYD_OPT_CONFIG);
//     assert_ptr_not_equal(st->dt1, NULL);
//
//     ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
//     assert_int_equal(ret, 0);
//
//     st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
//     assert_ptr_not_equal(st->dt2, NULL);
//
//     check_data_tree(st->dt1, st->dt2);
// }
//
// static void
// test_anydata(void **state)
// {
//     struct state *st = (*state);
//     const struct lys_module *mod;
//     int ret;
//     const char *test_anydata =
//     "module test-anydata {"
//     "   namespace \"urn:test-anydata\";"
//     "   prefix ya;"
//     ""
//     "   container cont {"
//     "       anydata ntf;"
//     "   }"
//     "}";
//
//     assert_non_null(ly_ctx_load_module(st->ctx, "ietf-netconf-notifications", NULL));
//
//     st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/ietf-netconf-notifications.json", LYD_JSON, LYD_OPT_NOTIF | LYD_OPT_TRUSTED, NULL);
//     assert_ptr_not_equal(st->dt1, NULL);
//
//     /* get notification in LYB format to set as anydata content */
//     ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
//     assert_int_equal(ret, 0);
//
//     lyd_free_withsiblings(st->dt1);
//     st->dt1 = NULL;
//
//     /* now comes the real test, test anydata */
//     mod = lys_parse_mem(st->ctx, test_anydata, LYS_YANG);
//     assert_non_null(mod);
//
//     st->dt1 = lyd_new(NULL, mod, "cont");
//     assert_non_null(st->dt1);
//
//     assert_non_null(lyd_new_anydata(st->dt1, NULL, "ntf", st->mem, LYD_ANYDATA_LYBD));
//     st->mem = NULL;
//
//     ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
//     assert_int_equal(ret, 0);
//
//     ret = lyd_validate(&st->dt1, LYD_OPT_CONFIG, NULL);
//     assert_int_equal(ret, 0);
//
//     st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
//     assert_ptr_not_equal(st->dt2, NULL);
//
//     check_data_tree(st->dt1, st->dt2);
//
//     /* and also test the embedded notification itself */
//     free(st->mem);
//     ret = lyd_lyb_data_length(((struct lyd_node_anydata *)st->dt1->child)->value.mem);
//     st->mem = malloc(ret);
//     memcpy(st->mem, ((struct lyd_node_anydata *)st->dt1->child)->value.mem, ret);
//
//     lyd_free_withsiblings(st->dt2);
//     st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_NOTIF | LYD_OPT_STRICT | LYD_OPT_NOEXTDEPS, NULL);
//     assert_ptr_not_equal(st->dt2, NULL);
//
//     /* parse the JSON again for this comparison */
//     lyd_free_withsiblings(st->dt1);
//     st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/ietf-netconf-notifications.json", LYD_JSON, LYD_OPT_NOTIF | LYD_OPT_TRUSTED, NULL);
//     assert_ptr_not_equal(st->dt1, NULL);
//
//     check_data_tree(st->dt1, st->dt2);
// }
//
// static void
// test_submodule_feature(void **state)
// {
//     struct state *st = (*state);
//     const struct lys_module *mod;
//     int ret;
//
//     ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
//     mod = ly_ctx_load_module(st->ctx, "feature-submodule-main", NULL);
//     assert_non_null(mod);
//     assert_int_equal(lys_features_enable(mod, "test-submodule-feature"), 0);
//
//     st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/test-submodule-feature.json", LYD_JSON, LYD_OPT_CONFIG);
//     assert_ptr_not_equal(st->dt1, NULL);
//
//     ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
//     assert_int_equal(ret, 0);
//
//     st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
//     assert_ptr_not_equal(st->dt2, NULL);
//
//     check_data_tree(st->dt1, st->dt2);
// }
//
// static void
// test_coliding_augments(void **state)
// {
//     struct state *st = (*state);
//     int ret;
//
//     ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
//     assert_non_null(ly_ctx_load_module(st->ctx, "augment-target", NULL));
//     assert_non_null(ly_ctx_load_module(st->ctx, "augment0", NULL));
//     assert_non_null(ly_ctx_load_module(st->ctx, "augment1", NULL));
//
//     st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/augment.xml", LYD_XML, LYD_OPT_CONFIG);
//     assert_ptr_not_equal(st->dt1, NULL);
//
//     ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
//     assert_int_equal(ret, 0);
//
//     st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
//     assert_ptr_not_equal(st->dt2, NULL);
//
//     check_data_tree(st->dt1, st->dt2);
// }
//
// static void
// test_leafrefs(void **state)
// {
//     struct state *st = (*state);
//     int ret;
//
//     ly_ctx_set_searchdir(st->ctx, TESTS_DIR"/data/files");
//     assert_non_null(ly_ctx_load_module(st->ctx, "leafrefs2", NULL));
//
//     st->dt1 = lyd_parse_path(st->ctx, TESTS_DIR"/data/files/leafrefs2.json", LYD_JSON, LYD_OPT_CONFIG | LYD_OPT_STRICT);
//     assert_ptr_not_equal(st->dt1, NULL);
//
//     ret = lyd_print_mem(&st->mem, st->dt1, LYD_LYB, LYP_WITHSIBLINGS);
//     assert_int_equal(ret, 0);
//
//     st->dt2 = lyd_parse_mem(st->ctx, st->mem, LYD_LYB, LYD_OPT_CONFIG | LYD_OPT_STRICT);
//     assert_ptr_not_equal(st->dt2, NULL);
//
//     check_data_tree(st->dt1, st->dt2);
// }

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_ietf_interfaces, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_origin, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_statements, setup_f, teardown_f),
        /*cmocka_unit_test_setup_teardown(test_types, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_annotations, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_similar_annot_names, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_many_child_annot, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_union, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_union2, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_collisions, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_anydata, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_submodule_feature, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_coliding_augments, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_leafrefs, setup_f, teardown_f),*/
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
