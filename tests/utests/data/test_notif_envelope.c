/**
 * @file test_notif_envelope.c
 * @author Roman Janota <janota@cesnet.cz>
 * @brief unit tests for YANG-Push notification envelope parsing
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include "libyang.h"

static int
setup(void **state)
{
    const char *feats[] = {"hostname-sequence-number", NULL};

    UTEST_SETUP;

    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2026-09-25", feats));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-observation", "2026-09-25", NULL));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-interfaces", NULL, NULL));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "iana-if-type", NULL, NULL));

    return 0;
}

#define CHECK_PARSE_ENVELOPE(DATA, FORMAT, TREE, OP) \
    { \
        struct ly_in *_in; \
        assert_int_equal(LY_SUCCESS, ly_in_new_memory(DATA, &_in)); \
        assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, _in, FORMAT, LYD_TYPE_NOTIF_ENVELOPE, 0, &TREE, &OP)); \
        ly_in_free(_in, 0); \
    }

#define CHECK_PARSE_ENVELOPE_ERR(DATA, FORMAT, RC) \
    { \
        struct ly_in *_in; \
        struct lyd_node *_tree, *_op; \
        assert_int_equal(LY_SUCCESS, ly_in_new_memory(DATA, &_in)); \
        assert_int_equal(RC, lyd_parse_op(UTEST_LYCTX, NULL, _in, FORMAT, LYD_TYPE_NOTIF_ENVELOPE, 0, &_tree, &_op)); \
        assert_null(_tree); \
        assert_null(_op); \
        ly_in_free(_in, 0); \
    }

/**
 * @brief Put the notification separated by the parser back into the envelope contents, to print the whole message.
 *
 * @param[in] tree Envelope with an empty contents.
 * @param[in] op Separated notification, freed.
 */
static void
envelope_attach(struct lyd_node *tree, struct lyd_node *op)
{
    struct lyd_node *contents, *root;

    assert_int_equal(LY_SUCCESS, lyd_find_path(tree, "contents", 0, &contents));
    assert_null(((struct lyd_node_any *)contents)->child);
    for (root = op; lyd_parent(root); root = lyd_parent(root)) {}
    assert_int_equal(LY_SUCCESS, lyd_any_copy_value(contents, root, NULL, 0));
    lyd_free_all(op);
}

static void
test_parse_xml(void **state)
{
    struct lyd_node *tree, *op, *node;
    const char *data;

    /* draft Figure 1 */
    data = "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">\n"
            "  <event-time>2024-10-10T10:59:55.32Z</event-time>\n"
            "  <contents>\n"
            "    <push-update xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-push\">\n"
            "      <id>1011</id>\n"
            "      <datastore-contents>\n"
            "        <interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\">\n"
            "          <interface>\n"
            "            <name>eth0</name>\n"
            "            <type xmlns:ianaift=\"urn:ietf:params:xml:ns:yang:iana-if-type\">ianaift:ethernetCsmacd</type>\n"
            "          </interface>\n"
            "        </interfaces>\n"
            "      </datastore-contents>\n"
            "    </push-update>\n"
            "  </contents>\n"
            "</envelope>\n";
    CHECK_PARSE_ENVELOPE(data, LYD_XML, tree, op);

    /* envelope with its schema */
    assert_string_equal(LYD_NAME(tree), "envelope");
    assert_true(tree->flags & LYD_EXT);
    assert_null(tree->next);
    node = lyd_child(tree);
    assert_string_equal(LYD_NAME(node), "event-time");
    assert_int_equal(node->schema->nodetype, LYS_LEAF);
    assert_string_equal(LYD_NAME(node->next), "contents");

    /* notification separated from the contents */
    assert_string_equal(LYD_NAME(op), "push-update");
    assert_int_equal(op->schema->nodetype, LYS_NOTIF);
    assert_null(lyd_parent(op));
    assert_null(((struct lyd_node_any *)node->next)->child);

    /* follow-up validation */
    assert_int_equal(LY_SUCCESS, lyd_validate_op(op, NULL, LYD_TYPE_NOTIF_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lyd_validate_all(&tree, NULL, LYD_VALIDATE_PRESENT, NULL));

    envelope_attach(tree, op);
    CHECK_LYD_STRING_PARAM(tree, data, LYD_XML, 0);
    lyd_free_all(tree);
}

static void
test_parse_json(void **state)
{
    struct lyd_node *tree, *op, *node;
    const char *data;

    /* draft Figure 5 */
    data = "{\n"
            "  \"ietf-yp-notification:envelope\": {\n"
            "    \"event-time\": \"2023-03-25T08:30:11.22Z\",\n"
            "    \"hostname\": \"example-router.example.com\",\n"
            "    \"sequence-number\": 1,\n"
            "    \"contents\": {\n"
            "      \"ietf-yang-push:push-update\": {\n"
            "        \"id\": 6666,\n"
            "        \"datastore-contents\": {\n"
            "          \"ietf-interfaces:interfaces\": {\n"
            "            \"interface\": [\n"
            "              {\n"
            "                \"name\": \"eth0\",\n"
            "                \"type\": \"iana-if-type:ethernetCsmacd\"\n"
            "              }\n"
            "            ]\n"
            "          }\n"
            "        }\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}\n";
    CHECK_PARSE_ENVELOPE(data, LYD_JSON, tree, op);

    assert_string_equal(LYD_NAME(tree), "envelope");
    assert_int_equal(LY_SUCCESS, lyd_find_path(tree, "sequence-number", 0, &node));
    assert_int_equal(((struct lyd_node_term *)node)->value.uint32, 1);
    assert_string_equal(LYD_NAME(op), "push-update");

    assert_int_equal(LY_SUCCESS, lyd_validate_op(op, NULL, LYD_TYPE_NOTIF_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lyd_validate_all(&tree, NULL, LYD_VALIDATE_PRESENT, NULL));

    envelope_attach(tree, op);
    CHECK_LYD_STRING_PARAM(tree, data, LYD_JSON, 0);
    lyd_free_all(tree);
}

static void
test_create_print(void **state)
{
    const struct lys_module *mod;
    struct lyd_node *env, *notif, *tree, *op;
    char *str;
    const char *xml, *json;

    xml = "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2026-11-02T10:00:00Z</event-time>"
            "<hostname>router</hostname>"
            "<sequence-number>7</sequence-number>"
            "<contents>"
            "<push-update xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-push\">"
            "<id>1</id>"
            "<timestamp xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-observation\">2026-11-02T09:59:59Z</timestamp>"
            "<point-in-time xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-observation\">current-accounting</point-in-time>"
            "</push-update>"
            "</contents>"
            "</envelope>";
    json = "{\"ietf-yp-notification:envelope\":{"
            "\"event-time\":\"2026-11-02T10:00:00Z\","
            "\"hostname\":\"router\","
            "\"sequence-number\":7,"
            "\"contents\":{\"ietf-yang-push:push-update\":{"
            "\"id\":1,"
            "\"ietf-yp-observation:timestamp\":\"2026-11-02T09:59:59Z\","
            "\"ietf-yp-observation:point-in-time\":\"current-accounting\"}}}}";

    /* the notification */
    mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "ietf-yang-push");
    assert_int_equal(LY_SUCCESS, lyd_new_inner(NULL, mod, "push-update", 0, &notif));
    assert_int_equal(LY_SUCCESS, lyd_new_term(notif, NULL, "id", "1", 0, NULL));
    mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "ietf-yp-observation");
    assert_int_equal(LY_SUCCESS, lyd_new_term(notif, mod, "timestamp", "2026-11-02T09:59:59Z", 0, NULL));
    assert_int_equal(LY_SUCCESS, lyd_new_term(notif, mod, "point-in-time", "current-accounting", 0, NULL));

    /* the envelope */
    mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "ietf-yp-notification");
    assert_int_equal(LY_SUCCESS, lyd_new_inner(NULL, mod, "envelope", 0, &env));
    assert_int_equal(LY_SUCCESS, lyd_new_term(env, NULL, "event-time", "2026-11-02T10:00:00Z", 0, NULL));
    assert_int_equal(LY_SUCCESS, lyd_new_term(env, NULL, "hostname", "router", 0, NULL));
    assert_int_equal(LY_SUCCESS, lyd_new_term(env, NULL, "sequence-number", "7", 0, NULL));
    assert_int_equal(LY_SUCCESS, lyd_new_any(env, NULL, "contents", notif, NULL, 0, LYD_NEW_ANY_USE_VALUE, NULL));
    assert_int_equal(LY_SUCCESS, lyd_validate_all(&env, NULL, LYD_VALIDATE_PRESENT, NULL));

    /* print and parse back */
    CHECK_LYD_STRING_PARAM(env, xml, LYD_XML, LYD_PRINT_SHRINK);
    CHECK_LYD_STRING_PARAM(env, json, LYD_JSON, LYD_PRINT_SHRINK);
    lyd_free_all(env);

    CHECK_PARSE_ENVELOPE(xml, LYD_XML, tree, op);
    assert_int_equal(LY_SUCCESS, lyd_validate_op(op, NULL, LYD_TYPE_NOTIF_YANG, NULL));
    envelope_attach(tree, op);
    assert_int_equal(LY_SUCCESS, lyd_print_mem(&str, tree, LYD_JSON, LYD_PRINT_SHRINK));
    assert_string_equal(str, json);
    free(str);
    lyd_free_all(tree);

    CHECK_PARSE_ENVELOPE(json, LYD_JSON, tree, op);
    assert_int_equal(LY_SUCCESS, lyd_validate_op(op, NULL, LYD_TYPE_NOTIF_YANG, NULL));
    envelope_attach(tree, op);
    CHECK_LYD_STRING_PARAM(tree, xml, LYD_XML, LYD_PRINT_SHRINK);
    lyd_free_all(tree);
}

#ifdef LY_HAVE_CBOR

static void
test_cbor(void **state)
{
    struct lyd_node *tree, *op;
    char *buffer, *json;
    struct ly_out *out;
    struct ly_in *in;
    const char *data;

    data = "{\"ietf-yp-notification:envelope\":{\"event-time\":\"2026-11-02T10:00:00Z\",\"hostname\":\"router\","
            "\"sequence-number\":7,\"contents\":{\"ietf-yang-push:push-update\":{\"id\":1011}}}}";
    CHECK_PARSE_ENVELOPE(data, LYD_JSON, tree, op);
    envelope_attach(tree, op);

    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&buffer, 0, &out));
    assert_int_equal(LY_SUCCESS, lyd_print_all(out, tree, LYD_CBOR, 0));
    lyd_free_all(tree);

    assert_int_equal(LY_SUCCESS, ly_in_new_memory_chunk(buffer, ly_out_printed(out), &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_CBOR, LYD_TYPE_NOTIF_ENVELOPE, 0, &tree, &op));
    ly_in_free(in, 0);
    ly_out_free(out, NULL, 0);
    free(buffer);

    assert_string_equal(LYD_NAME(op), "push-update");
    assert_int_equal(LY_SUCCESS, lyd_validate_op(op, NULL, LYD_TYPE_NOTIF_YANG, NULL));
    assert_int_equal(LY_SUCCESS, lyd_validate_all(&tree, NULL, LYD_VALIDATE_PRESENT, NULL));

    envelope_attach(tree, op);
    assert_int_equal(LY_SUCCESS, lyd_print_mem(&json, tree, LYD_JSON, LYD_PRINT_SHRINK));
    assert_string_equal(json, data);
    free(json);
    lyd_free_all(tree);
}

#endif /* LY_HAVE_CBOR */

static void
test_nested(void **state)
{
    struct lyd_node *tree, *op;
    struct ly_in *in;
    const char *mod, *data;

    mod = "module a {yang-version 1.1; namespace urn:tests:a; prefix a;"
            "container c {list l {key k; leaf k {type string;} notification n {leaf x {type uint8;}}}}}";
    UTEST_ADD_MODULE(mod, LYS_IN_YANG, NULL, NULL);

    data = "{\"ietf-yp-notification:envelope\":{\"event-time\":\"2026-11-02T10:00:00Z\","
            "\"contents\":{\"a:c\":{\"l\":[{\"k\":\"key\",\"n\":{\"x\":5}}]}}}}";
    CHECK_PARSE_ENVELOPE(data, LYD_JSON, tree, op);

    assert_string_equal(LYD_NAME(op), "n");
    assert_string_equal(LYD_NAME(lyd_parent(op)), "l");
    assert_int_equal(LY_SUCCESS, lyd_validate_op(op, NULL, LYD_TYPE_NOTIF_YANG, NULL));
    envelope_attach(tree, op);
    CHECK_LYD_STRING_PARAM(tree, data, LYD_JSON, LYD_PRINT_SHRINK);
    lyd_free_all(tree);

    /* anydata in the notification is still parsed as opaque anydata content, even with strict parsing */
    data = "{\"ietf-yp-notification:envelope\":{\"event-time\":\"2026-11-02T10:00:00Z\",\"contents\":{"
            "\"ietf-yang-push:push-update\":{\"id\":1,\"datastore-contents\":{\"x:unknown\":{\"leaf\":1}}}}}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_SUCCESS, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_JSON, LYD_TYPE_NOTIF_ENVELOPE,
            LYD_PARSE_STRICT, &tree, &op));
    ly_in_free(in, 0);
    assert_string_equal(LYD_NAME(op), "push-update");
    lyd_free_all(tree);
    lyd_free_all(op);
}

static void
test_augment(void **state)
{
    struct lyd_node *tree, *op, *node;
    const char *mod, *data;

    mod = "module b {yang-version 1.1; namespace urn:tests:b; prefix b;"
            "import ietf-yang-structure-ext {prefix sx;}"
            "import ietf-yp-notification {prefix iypn;}"
            "sx:augment-structure \"/iypn:envelope\" {leaf foo {type uint16;}}}";
    UTEST_ADD_MODULE(mod, LYS_IN_YANG, NULL, NULL);

    data = "{\"ietf-yp-notification:envelope\":{\"event-time\":\"2026-11-02T10:00:00Z\","
            "\"contents\":{\"ietf-yang-push:push-update\":{\"id\":1}},\"b:foo\":42}}";
    CHECK_PARSE_ENVELOPE(data, LYD_JSON, tree, op);

    assert_int_equal(LY_SUCCESS, lyd_find_path(tree, "b:foo", 0, &node));
    assert_int_equal(((struct lyd_node_term *)node)->value.uint16, 42);
    assert_string_equal(LYD_NAME(op), "push-update");
    assert_int_equal(LY_SUCCESS, lyd_validate_all(&tree, NULL, LYD_VALIDATE_PRESENT, NULL));
    envelope_attach(tree, op);
    CHECK_LYD_STRING_PARAM(tree, data, LYD_JSON, LYD_PRINT_SHRINK);
    lyd_free_all(tree);
}

static void
test_invalid(void **state)
{
    struct lyd_node *tree, *op;
    struct ly_in *in;
    const char *data;

    /* no envelope */
    data = "<push-update xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-push\"><id>1</id></push-update>";
    CHECK_PARSE_ENVELOPE_ERR(data, LYD_XML, LY_EVALID);
    CHECK_LOG_CTX("Unexpected notification element \"push-update\".", "/ietf-yang-push:push-update", 1);

    data = "<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\"/>";
    CHECK_PARSE_ENVELOPE_ERR(data, LYD_XML, LY_EVALID);
    CHECK_LOG_CTX("Unexpected container element \"interfaces\".", "/ietf-interfaces:interfaces", 1);

    /* two notifications */
    data = "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2026-11-02T10:00:00Z</event-time><contents>"
            "<push-update xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-push\"><id>1</id></push-update>"
            "<push-update xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-push\"><id>2</id></push-update>"
            "</contents></envelope>";
    CHECK_PARSE_ENVELOPE_ERR(data, LYD_XML, LY_EVALID);
    CHECK_LOG_CTX("Unexpected notification element \"push-update\", notification \"push-update\" already parsed.",
            "/ietf-yang-push:push-update", 1);

    /* RPC instead of the notification */
    data = "{\"ietf-yp-notification:envelope\":{\"event-time\":\"2026-11-02T10:00:00Z\",\"contents\":{"
            "\"ietf-subscribed-notifications:kill-subscription\":{\"id\":1}}}}";
    CHECK_PARSE_ENVELOPE_ERR(data, LYD_JSON, LY_EVALID);
    CHECK_LOG_CTX("Unexpected RPC element \"kill-subscription\".",
            "/ietf-subscribed-notifications:kill-subscription", 1);

    /* invalid notification value, the contents are not parsed as opaque anydata content */
    data = "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2026-11-02T10:00:00Z</event-time><contents>"
            "<push-update xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-push\"><id>x</id></push-update>"
            "</contents></envelope>";
    CHECK_PARSE_ENVELOPE_ERR(data, LYD_XML, LY_EVALID);
    CHECK_LOG_CTX("Invalid type uint32 value \"x\".",
            "/ietf-yp-notification:envelope/contents/ietf-yang-push:push-update/id", 1);

#ifdef LY_HAVE_CBOR
    data = "\xa1\x78\x1d" "ietf-yp-notification:envelope" "\xa2"
            "\x6a" "event-time" "\x74" "2026-11-02T10:00:00Z"
            "\x68" "contents" "\xa1\x78\x1a" "ietf-yang-push:push-update" "\xa1\x62" "id" "\x61" "x";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory_chunk(data, strlen(data), &in));
    assert_int_equal(LY_EVALID, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_CBOR, LYD_TYPE_NOTIF_ENVELOPE, 0, &tree, &op));
    ly_in_free(in, 0);
    CHECK_LOG_CTX("Invalid non-number-encoded uint32 value \"x\".", "/ietf-yang-push:push-update/id", 1);
#endif /* LY_HAVE_CBOR */

    /* unknown node in the notification with strict parsing */
    data = "{\"ietf-yp-notification:envelope\":{\"event-time\":\"2026-11-02T10:00:00Z\",\"contents\":{"
            "\"ietf-yang-push:push-update\":{\"id\":1,\"unknown\":1}}}}";
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_EVALID, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_JSON, LYD_TYPE_NOTIF_ENVELOPE,
            LYD_PARSE_STRICT, &tree, &op));
    ly_in_free(in, 0);
    CHECK_LOG_CTX("Node \"unknown\" not found as a child of \"push-update\" node.",
            "/ietf-yp-notification:envelope/contents/ietf-yang-push:push-update", 1);

    /* no notification */
    data = "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2026-11-02T10:00:00Z</event-time><contents/></envelope>";
    CHECK_PARSE_ENVELOPE_ERR(data, LYD_XML, LY_EVALID);
    CHECK_LOG_CTX("Missing the operation node.", NULL, 1);

    data = "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2026-11-02T10:00:00Z</event-time><contents>"
            "<interfaces xmlns=\"urn:ietf:params:xml:ns:yang:ietf-interfaces\"/></contents></envelope>";
    CHECK_PARSE_ENVELOPE_ERR(data, LYD_XML, LY_EVALID);
    CHECK_LOG_CTX("Missing the operation node.", NULL, 1);

    /* sibling of the envelope */
    data = "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2026-11-02T10:00:00Z</event-time><contents>"
            "<push-update xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-push\"><id>1</id></push-update>"
            "</contents></envelope>"
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\"/>";
    CHECK_PARSE_ENVELOPE_ERR(data, LYD_XML, LY_EVALID);
    CHECK_LOG_CTX("Unexpected sibling node.", NULL, 1);

    /* invalid arguments */
    assert_int_equal(LY_SUCCESS, ly_in_new_memory(data, &in));
    assert_int_equal(LY_EINVAL, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_LYB, LYD_TYPE_NOTIF_ENVELOPE, 0, &tree, &op));
    CHECK_LOG_CTX("Invalid argument (format == LYD_XML) || (format == LYD_JSON) || (format == LYD_CBOR) (lyd_parse_op()).",
            NULL, 0);
    assert_int_equal(LY_EINVAL, lyd_parse_op(UTEST_LYCTX, NULL, in, LYD_XML, LYD_TYPE_NOTIF_ENVELOPE, 0, &tree, NULL));
    CHECK_LOG_CTX("Invalid argument op (lyd_parse_op()).", NULL, 0);
    ly_in_free(in, 0);
}

static void
test_invalid_validate(void **state)
{
    struct lyd_node *tree, *op;
    const char *data;

    /* missing event-time */
    data = "{\"ietf-yp-notification:envelope\":{\"contents\":{\"ietf-yang-push:push-update\":{\"id\":1}}}}";
    CHECK_PARSE_ENVELOPE(data, LYD_JSON, tree, op);
    assert_int_equal(LY_EVALID, lyd_validate_all(&tree, NULL, LYD_VALIDATE_PRESENT, NULL));
    CHECK_LOG_CTX("Mandatory node \"event-time\" instance does not exist.", "/ietf-yp-notification:envelope", 0);
    lyd_free_all(tree);
    lyd_free_all(op);
}

static void
test_no_module(void **state)
{
    const char *data;

    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yang-push", NULL, NULL));

    data = "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2026-11-02T10:00:00Z</event-time><contents>"
            "<push-update xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-push\"><id>1</id></push-update>"
            "</contents></envelope>";
    CHECK_PARSE_ENVELOPE_ERR(data, LYD_XML, LY_EINVAL);
    CHECK_LOG_CTX("Module \"ietf-yp-notification\" is not implemented.", NULL, 0);

    data = "{\"ietf-yp-notification:envelope\":{\"event-time\":\"2026-11-02T10:00:00Z\",\"contents\":{"
            "\"ietf-yang-push:push-update\":{\"id\":1}}}}";
    CHECK_PARSE_ENVELOPE_ERR(data, LYD_JSON, LY_EINVAL);
    CHECK_LOG_CTX("Module \"ietf-yp-notification\" is not implemented.", NULL, 0);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_parse_xml, setup),
        UTEST(test_parse_json, setup),
        UTEST(test_create_print, setup),
#ifdef LY_HAVE_CBOR
        UTEST(test_cbor, setup),
#endif /* LY_HAVE_CBOR */
        UTEST(test_nested, setup),
        UTEST(test_augment, setup),
        UTEST(test_invalid, setup),
        UTEST(test_invalid_validate, setup),
        UTEST(test_no_module),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
