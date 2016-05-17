/**
 * @file test_xpath.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Cmocka tests for XPath expression evaluation.
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
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
#include <cmocka.h>

#include "../config.h"
#include "../../src/libyang.h"

struct state {
    struct ly_ctx *ctx;
    struct lyd_node *dt;
};

static const char *data =
"{"
  "\"ietf-interfaces:interfaces\": {"
    "\"interface\": ["
      "{"
        "\"name\": \"iface1\","
        "\"description\": \"iface1 dsc\","
        "\"type\": \"iana-if-type:ethernetCsmacd\","
        "\"@type\": {"
          "\"yang:type_attr\":\"1\""
        "},"
        "\"enabled\": true,"
        "\"link-up-down-trap-enable\": \"disabled\","
        "\"ietf-ip:ipv4\": {"
          "\"@\": {"
            "\"yang:ip_attr\":\"14\""
          "},"
          "\"enabled\": true,"
          "\"forwarding\": true,"
          "\"mtu\": 68,"
          "\"address\": ["
            "{"
              "\"ip\": \"10.0.0.1\","
              "\"netmask\": \"255.0.0.0\""
            "},"
            "{"
              "\"ip\": \"172.0.0.1\","
              "\"prefix-length\": 16"
            "}"
          "],"
          "\"neighbor\": ["
            "{"
              "\"ip\": \"10.0.0.2\","
              "\"link-layer-address\": \"01:34:56:78:9a:bc:de:f0\""
            "}"
          "]"
        "},"
        "\"ietf-ip:ipv6\": {"
          "\"@\": {"
            "\"yang:ip_attr\":\"16\""
          "},"
          "\"enabled\": true,"
          "\"forwarding\": false,"
          "\"mtu\": 1280,"
          "\"address\": ["
            "{"
              "\"ip\": \"2001:abcd:ef01:2345:6789:0:1:1\","
              "\"prefix-length\": 64"
            "}"
          "],"
          "\"neighbor\": ["
            "{"
              "\"ip\": \"2001:abcd:ef01:2345:6789:0:1:2\","
              "\"link-layer-address\": \"01:34:56:78:9a:bc:de:f0\""
            "}"
          "],"
          "\"dup-addr-detect-transmits\": 52,"
          "\"autoconf\": {"
            "\"create-global-addresses\": true,"
            "\"create-temporary-addresses\": false,"
            "\"temporary-valid-lifetime\": 600,"
            "\"temporary-preferred-lifetime\": 300"
          "}"
        "}"
      "},"
      "{"
        "\"name\": \"iface2\","
        "\"description\": \"iface2 dsc\","
        "\"type\": \"iana-if-type:softwareLoopback\","
        "\"@type\": {"
          "\"yang:type_attr\":\"2\""
        "},"
        "\"enabled\": false,"
        "\"link-up-down-trap-enable\": \"disabled\","
        "\"ietf-ip:ipv4\": {"
          "\"@\": {"
            "\"yang:ip_attr\":\"24\""
          "},"
          "\"address\": ["
            "{"
              "\"ip\": \"10.0.0.5\","
              "\"netmask\": \"255.0.0.0\""
            "},"
            "{"
              "\"ip\": \"172.0.0.5\","
              "\"prefix-length\": 16"
            "}"
          "],"
          "\"neighbor\": ["
            "{"
              "\"ip\": \"10.0.0.1\","
              "\"link-layer-address\": \"01:34:56:78:9a:bc:de:fa\""
            "}"
          "]"
        "},"
        "\"ietf-ip:ipv6\": {"
          "\"@\": {"
            "\"yang:ip_attr\":\"26\""
          "},"
          "\"address\": ["
            "{"
              "\"ip\": \"2001:abcd:ef01:2345:6789:0:1:5\","
              "\"prefix-length\": 64"
            "}"
          "],"
          "\"neighbor\": ["
            "{"
              "\"ip\": \"2001:abcd:ef01:2345:6789:0:1:1\","
              "\"link-layer-address\": \"01:34:56:78:9a:bc:de:fa\""
            "}"
          "],"
          "\"dup-addr-detect-transmits\": 100,"
          "\"autoconf\": {"
            "\"create-global-addresses\": true,"
            "\"create-temporary-addresses\": false,"
            "\"temporary-valid-lifetime\": 600,"
            "\"temporary-preferred-lifetime\": 300"
          "}"
        "}"
      "}"
    "]"
  "}"
"}"
;

static int
setup_f(void **state)
{
    struct state *st;
    const char *augschema = "ietf-ip";
    const char *typeschema = "iana-if-type";
    const char *ietfdir = TESTS_DIR"/schema/yin/ietf/";
    const struct lys_module *mod;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error.\n");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(ietfdir);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        goto error;
    }

    /* schema */
    mod = ly_ctx_load_module(st->ctx, augschema, NULL);
    if (!mod) {
        fprintf(stderr, "Failed to load data module \"%s\".\n", augschema);
        goto error;
    }
    lys_features_enable(mod, "*");

    mod = ly_ctx_get_module(st->ctx, "ietf-interfaces", NULL);
    if (!mod) {
        fprintf(stderr, "Failed to get data module \"ietf-interfaces\".\n");
        goto error;
    }
    lys_features_enable(mod, "*");

    mod = ly_ctx_load_module(st->ctx, typeschema, NULL);
    if (!mod) {
        fprintf(stderr, "Failed to load data module \"%s\".\n", typeschema);
        goto error;
    }

    return 0;

error:
    ly_ctx_destroy(st->ctx, NULL);
    free(st);
    (*state) = NULL;

    return -1;
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
test_parse(void **state)
{
    struct state *st = (*state);

    st->dt = lyd_parse_mem(st->ctx, data, LYD_JSON, LYD_OPT_CONFIG);
    assert_ptr_not_equal(st->dt, NULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test_setup_teardown(test_parse, setup_f, teardown_f),
                    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

