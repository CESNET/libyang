/*
 * @file test_lys_node_switch.c
 * @author: Antonio Paunovic <antonio.paunovic@sartura.hr>
 * @brief unit tests for functions from tree_internal.h header
 *
 * Copyright (C) 2016 Deutsche Telekom AG.
 *
 * Author: Antonio Paunovic <antonio.paunovic@sartura.hr>
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

#include "../../src/libyang.h"
#include "../../src/tree_internal.h"

struct ly_ctx *ctx = NULL;
struct lyd_node *root = NULL;

static int
setup_f(void **state)
{
    (void) state; /* unused */

    return 0;
}

static int
teardown_f(void **state)
{
    (void) state; /* unused */
    if (ctx) {
        ly_ctx_destroy(ctx, NULL);
    }

    return 0;
}

/**
 * Function lys_node_switch() takes two parameters: dst and src. src is a shallow copy of dst
 * src replaces dst. dst is left without children, they go to src.
 */
static void
test_lys_node_switch(void **state)
{
    (void) state; /* unused */
    struct lys_node_leaf
        *dst, *src,
        *dst_previous, *dst_next, *dst_child, *dst_parent,
        *src_previous, *src_next;
    struct lys_module *module;

    /* Allocation of data. module is needed as an implementation detail. */
    module = calloc(1, sizeof(*module));
    dst = calloc(1, sizeof(*dst));
    src = calloc(1, sizeof(*src));
    dst_previous = calloc(1, sizeof(*dst_previous));
    src_previous = calloc(1, sizeof(*src_previous));
    dst_next = calloc(1, sizeof(*dst_next));
    src_next = calloc(1, sizeof(*src_next));
    dst_parent = calloc(1, sizeof(*dst_parent));
    dst_child = calloc(1, sizeof(*dst_child));

    /* src is supposed to be shallow copy of dst.
     * That means it copies dst values but not its pointers.
     * src and dst must have same names and types atleast. */
    dst->module = src->module = module;
    dst->name = "x";
    src->name = "x";
    dst->nodetype = LYS_LEAF;
    src->nodetype = LYS_LEAF;

    /* dst has a child, previous and next siblings, child and a parent.
     * They all have inverse relation to it. */
    dst->prev = (struct lys_node*)dst_previous;
    dst->next = (struct lys_node*)dst_next;
    dst_previous->next = (struct lys_node*)dst;
    dst_next->prev = (struct lys_node*)dst;
    dst->child = (struct lys_node*)dst_child;
    dst->parent = (struct lys_node*)dst_parent;
    dst->parent->child = (struct lys_node*)dst;
    dst->child = (struct lys_node*)dst_child;
    dst->child->parent = (struct lys_node*)dst;

    /* Procedure call. */
    lys_node_switch((struct lys_node*)dst, (struct lys_node*)src);

    /* Previous sibling of dst is now previous sibling of src. */
    assert_ptr_equal(dst_previous->next, src);

    /* Next sibling of dst is now next sibling of src. */
    assert_ptr_equal(src->next, dst_next);

    /* Previous sibling of src is now previous sibling of dst. */
    assert_ptr_equal(src->prev, dst_previous);

    /* dst mustn't reference it's old next sibling so its null-ed */
    assert_ptr_equal(dst->next, NULL);

    /* dst is now its own previous sibling because there has to be previous sibling. */
    assert_ptr_equal(dst->prev, dst);

    /* src parent is now former dst parent. */
    assert_ptr_equal(src->parent, dst_parent);

    /* src child is now former dst child. */
    assert_ptr_equal(src->child, dst_child);

    /* dst mustn't reference it's old next sibling so its null-ed */
    assert_ptr_equal(dst->child, NULL);

    free(dst_child);
    free(dst_parent);
    free(dst_next);
    free(dst_previous);

    free(dst->child);
    free(dst->parent);
    free(dst->next);
    free(dst);

    free(src_previous);
    free(src_next);
    free(src);
    free(module);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_lys_node_switch, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
