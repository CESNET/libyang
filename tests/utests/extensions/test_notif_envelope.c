/**
 * @file test_notif_envelope.c
 * @author Roman Janota <Roman.Janota@cesnet.cz>
 * @brief Unit tests for YANG-Push notification envelope
 *
 * Copyright (c) 2025 CESNET, z.s.p.o.
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

/* ---- Test YANG module definitions ---- */

/* Simple notification used as envelope contents across all tests */
static const char *notif_yang =
        "module test-notif {"
        "  yang-version 1.1;"
        "  namespace \"urn:tests:notification\";"
        "  prefix tn;"
        "  notification event {"
        "    leaf message { type string; }"
        "  }"
        "}";

/* Demonstrates sx:augment-structure on ietf-yp-notification:envelope */
static const char *augment_yang =
        "module test-yp-augment {"
        "  yang-version 1.1;"
        "  namespace \"urn:tests:yp-augment\";"
        "  prefix tya;"
        "  import ietf-yang-structure-ext { prefix sx; }"
        "  import ietf-yp-notification { prefix iypn; }"
        "  sx:augment-structure \"/iypn:envelope\" {"
        "    leaf foo {"
        "      type string;"
        "    }"
        "  }"
        "}";

/* Second notification module: provides an additional notification
 * for the multiple-notifications test, a notification with a mandatory
 * leaf for the validation-failure test, and a top-level container
 * for the non-notification-in-contents test */
static const char *notif_yang2 =
        "module test-notif2 {"
        "  yang-version 1.1;"
        "  namespace \"urn:tests:notification2\";"
        "  prefix tn2;"
        "  notification event2 {"
        "    leaf message { type string; }"
        "  }"
        "  notification event3 {"
        "    leaf mandatory-message { type string; mandatory true; }"
        "  }"
        "  container top-container {"
        "    leaf val { type string; }"
        "  }"
        "}";

/* Second augment module for testing multiple independent
 * sx:augment-structure augmentations on the same envelope */
static const char *augment_yang2 =
        "module test-yp-augment2 {"
        "  yang-version 1.1;"
        "  namespace \"urn:tests:yp-augment2\";"
        "  prefix tya2;"
        "  import ietf-yang-structure-ext { prefix sx; }"
        "  import ietf-yp-notification { prefix iypn; }"
        "  sx:augment-structure \"/iypn:envelope\" {"
        "    leaf bar {"
        "      type string;"
        "    }"
        "  }"
        "}";

/* ---- Helpers ---- */

/**
 * @brief Find a child of a given parent by name.
 *
 * @param[in] parent Parent node to search.
 * @param[in] name Name of the child to find.
 * @return Child node if found, NULL otherwise.
 */
static struct lyd_node *
find_child(const struct lyd_node *parent, const char *name)
{
    struct lyd_node *child;

    for (child = lyd_child(parent); child; child = child->next) {
        if (!strcmp(LYD_NAME(child), name)) {
            return child;
        }
    }
    return NULL;
}

/* ---- Setup ---- */

/* Prepare context with search directories so real IETF modules
 * (ietf-yp-notification and its import chain) can be auto-resolved. */
static int
setup(void **state)
{
    UTEST_SETUP;
    ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG);
    return 0;
}

/* ============================================================
 * Group 1: Valid envelope parsing
 * ============================================================ */

static void
test_parse_xml(void **state)
{
    struct lyd_node *tree, *contents, *notif;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<contents>"
            "<event xmlns=\"urn:tests:notification\">"
            "<message>interface up</message>"
            "</event>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));

    /* The envelope root is an sx:structure extension instance */
    assert_non_null(tree);
    assert_string_equal(LYD_NAME(tree), "envelope");
    assert_true(tree->flags & LYD_EXT);

    /* Drill into contents anydata to reach the notification */
    contents = find_child(tree, "contents");
    assert_non_null(contents);
    notif = lyd_child_any(contents);
    assert_non_null(notif);
    assert_string_equal(LYD_NAME(notif), "event");
    assert_int_equal(notif->schema->nodetype, LYS_NOTIF);

    lyd_free_all(tree);
}

static void
test_parse_json(void **state)
{
    struct lyd_node *tree, *contents, *notif;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    const char *json =
            "{\"ietf-yp-notification:envelope\":{"
            "\"event-time\":\"2024-10-10T08:00:11.22Z\","
            "\"contents\":{"
            "\"test-notif:event\":{\"message\":\"interface up\"}"
            "}}}";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, json, LYD_JSON,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));

    assert_non_null(tree);
    assert_string_equal(LYD_NAME(tree), "envelope");

    contents = find_child(tree, "contents");
    assert_non_null(contents);
    notif = lyd_child_any(contents);
    assert_non_null(notif);
    assert_string_equal(LYD_NAME(notif), "event");
    assert_int_equal(notif->schema->nodetype, LYS_NOTIF);

    lyd_free_all(tree);
}

static void
test_parse_lyb(void **state)
{
    struct lyd_node *tree1, *tree2;
    char *lyb;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<contents>"
            "<event xmlns=\"urn:tests:notification\">"
            "<message>interface up</message>"
            "</event>"
            "</contents>"
            "</envelope>";

    /* Parse XML -> serialize to LYB -> re-parse LYB -> compare trees */
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, &tree1));

    assert_int_equal(LY_SUCCESS, lyd_print_mem(&lyb, tree1, LYD_LYB, LYD_PRINT_SIBLINGS));

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, lyb, LYD_LYB,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree2));

    CHECK_LYD(tree1, tree2);

    free(lyb);
    lyd_free_all(tree1);
    lyd_free_all(tree2);
}

/* ============================================================
 * Group 2: Invalid envelope rejection
 * ============================================================ */

static void
test_invalid_bad_event_time(void **state)
{
    struct lyd_node *tree = NULL;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    /* yang:date-and-time has a pattern constraint - garbage must be rejected */
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>not-a-date</event-time>"
            "<contents>"
            "<event xmlns=\"urn:tests:notification\">"
            "<message>test</message>"
            "</event>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_EVALID, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    UTEST_LOG_CTX_CLEAN;
}

static void
test_invalid_unknown_metadata(void **state)
{
    struct lyd_node *tree = NULL;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    /* An element that is not defined in the envelope schema nor any
     * sx:augment-structure - strict parsing must reject it */
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<unknown-meta>value</unknown-meta>"
            "<contents>"
            "<event xmlns=\"urn:tests:notification\">"
            "<message>test</message>"
            "</event>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_EVALID, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    UTEST_LOG_CTX_CLEAN;
}

static void
test_invalid_bad_contents(void **state)
{
    struct lyd_node *tree = NULL;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    /* With LYD_PARSE_ANYDATA_STRICT, content that does not match any
     * known schema inside the anydata is rejected.  Here, "unknown-el"
     * in the test-notif namespace does not map to any YANG node. */
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<contents>"
            "<unknown-el xmlns=\"urn:tests:notification\"/>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_EVALID, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT | LYD_PARSE_ANYDATA_STRICT,
            LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    UTEST_LOG_CTX_CLEAN;
}

static void
test_invalid_missing_event_time(void **state)
{
    struct lyd_node *tree = NULL;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    /* event-time is a mandatory leaf in ietf-yp-notification:envelope,
     * so an envelope without it must be rejected by validation. */
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<contents>"
            "<event xmlns=\"urn:tests:notification\">"
            "<message>test</message>"
            "</event>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_EVALID, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    UTEST_LOG_CTX_CLEAN;
}

static void
test_invalid_missing_contents(void **state)
{
    struct lyd_node *tree = NULL;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    /* contents is optional in the schema, but the envelope validator
     * requires it to be present */
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "</envelope>";

    assert_int_equal(LY_EVALID, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    UTEST_LOG_CTX_CLEAN;
}

static void
test_invalid_empty_contents(void **state)
{
    struct lyd_node *tree = NULL;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    /* contents is present but empty - no notification inside */
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<contents/>"
            "</envelope>";

    assert_int_equal(LY_EVALID, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    UTEST_LOG_CTX_CLEAN;
}

static void
test_invalid_multiple_notifs(void **state)
{
    struct lyd_node *tree = NULL;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);
    UTEST_ADD_MODULE(notif_yang2, LYS_IN_YANG, NULL, NULL);

    /* Two notifications inside contents - the parser auto-detection
     * sets LYD_INTOPT_NOTIF which rejects a second notification */
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<contents>"
            "<event xmlns=\"urn:tests:notification\">"
            "<message>msg1</message>"
            "</event>"
            "<event2 xmlns=\"urn:tests:notification2\">"
            "<message>msg2</message>"
            "</event2>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_EVALID, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    UTEST_LOG_CTX_CLEAN;
}

static void
test_invalid_bad_contents_no_strict(void **state)
{
    struct lyd_node *tree = NULL;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    /* Without LYD_PARSE_ANYDATA_STRICT, the auto-detection of envelope
     * contents still forces strict parsing, rejecting unknown elements */
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<contents>"
            "<unknown-el xmlns=\"urn:tests:notification\"/>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_EVALID, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    UTEST_LOG_CTX_CLEAN;
}

/* ============================================================
 * Group 3: Envelope augmentation (sx:augment-structure)
 * ============================================================ */

static void
test_augment_xml(void **state)
{
    struct lyd_node *tree, *node;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);
    UTEST_ADD_MODULE(augment_yang, LYS_IN_YANG, NULL, NULL);

    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<foo xmlns=\"urn:tests:yp-augment\">bar</foo>"
            "<contents>"
            "<event xmlns=\"urn:tests:notification\">"
            "<message>interface up</message>"
            "</event>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));

    /* Augmented 'foo' sits alongside event-time in the envelope header */
    node = find_child(tree, "foo");
    assert_non_null(node);
    assert_string_equal(lyd_get_value(node), "bar");

    /* Contents still holds the notification as expected */
    node = find_child(tree, "contents");
    assert_non_null(node);
    assert_int_equal(lyd_child_any(node)->schema->nodetype, LYS_NOTIF);

    lyd_free_all(tree);
}

static void
test_augment_json(void **state)
{
    struct lyd_node *tree, *node;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);
    UTEST_ADD_MODULE(augment_yang, LYS_IN_YANG, NULL, NULL);

    /* In JSON, the augmented leaf is namespace-qualified with its module name */
    const char *json =
            "{\"ietf-yp-notification:envelope\":{"
            "\"event-time\":\"2024-10-10T08:00:11.22Z\","
            "\"test-yp-augment:foo\":\"bar\","
            "\"contents\":{"
            "\"test-notif:event\":{\"message\":\"interface up\"}"
            "}}}";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, json, LYD_JSON,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));

    node = find_child(tree, "foo");
    assert_non_null(node);
    assert_string_equal(lyd_get_value(node), "bar");

    node = find_child(tree, "contents");
    assert_non_null(node);
    assert_int_equal(lyd_child_any(node)->schema->nodetype, LYS_NOTIF);

    lyd_free_all(tree);
}

static void
test_augment_lyb(void **state)
{
    struct lyd_node *tree1, *tree2;
    char *lyb;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);
    UTEST_ADD_MODULE(augment_yang, LYS_IN_YANG, NULL, NULL);

    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<foo xmlns=\"urn:tests:yp-augment\">bar</foo>"
            "<contents>"
            "<event xmlns=\"urn:tests:notification\">"
            "<message>interface up</message>"
            "</event>"
            "</contents>"
            "</envelope>";

    /* Parse XML -> serialize to LYB -> re-parse LYB -> compare trees */
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, &tree1));

    assert_int_equal(LY_SUCCESS, lyd_print_mem(&lyb, tree1, LYD_LYB, LYD_PRINT_SIBLINGS));

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, lyb, LYD_LYB,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree2));

    CHECK_LYD(tree1, tree2);

    free(lyb);
    lyd_free_all(tree1);
    lyd_free_all(tree2);
}

static void
test_augment_multiple(void **state)
{
    struct lyd_node *tree, *node;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);
    UTEST_ADD_MODULE(augment_yang, LYS_IN_YANG, NULL, NULL);
    UTEST_ADD_MODULE(augment_yang2, LYS_IN_YANG, NULL, NULL);

    /* Two independent augment modules each add a leaf to the envelope */
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<foo xmlns=\"urn:tests:yp-augment\">bar</foo>"
            "<bar xmlns=\"urn:tests:yp-augment2\">baz</bar>"
            "<contents>"
            "<event xmlns=\"urn:tests:notification\">"
            "<message>interface up</message>"
            "</event>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));

    node = find_child(tree, "foo");
    assert_non_null(node);
    assert_string_equal(lyd_get_value(node), "bar");

    node = find_child(tree, "bar");
    assert_non_null(node);
    assert_string_equal(lyd_get_value(node), "baz");

    node = find_child(tree, "contents");
    assert_non_null(node);
    assert_int_equal(lyd_child_any(node)->schema->nodetype, LYS_NOTIF);

    lyd_free_all(tree);
}

/* ============================================================
 * Group 4: hostname-sequence-number feature
 * ============================================================ */

static void
test_feature_disabled(void **state)
{
    const struct lys_module *mod;

    /* Explicitly disable all features */
    const char *no_feats[] = {NULL};

    mod = ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", no_feats);
    assert_non_null(mod);

    /* Verify the hostname-sequence-number feature is disabled */
    assert_int_equal(LY_ENOT, lys_feature_value(mod, "hostname-sequence-number"));

    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    /* Basic envelope without hostname/sequence-number parses correctly
     * even when the feature is disabled */
    struct lyd_node *tree = NULL;
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<contents>"
            "<event xmlns=\"urn:tests:notification\">"
            "<message>test</message>"
            "</event>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_all(tree);
}

static void
test_feature_enabled_optional(void **state)
{
    struct lyd_node *tree = NULL;
    const char *feats[] = {"hostname-sequence-number", NULL};

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", feats));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    /* Feature is enabled but hostname and sequence-number are optional;
     * an envelope without them must still parse successfully */
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<contents>"
            "<event xmlns=\"urn:tests:notification\">"
            "<message>test</message>"
            "</event>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);
    lyd_free_all(tree);
}

static void
test_feature_disabled_hostname_accepted(void **state)
{
    struct lyd_node *tree, *node;
    const char *no_feats[] = {NULL};

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", no_feats));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    /* The structure plugin compiles with LYS_COMPILE_NO_DISABLED (ignore
     * if-feature), so hostname/sequence-number are always present in the
     * compiled schema even when the feature is disabled.  Data for these
     * nodes is therefore accepted regardless of the feature state. */
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<hostname>example.com</hostname>"
            "<sequence-number>42</sequence-number>"
            "<contents>"
            "<event xmlns=\"urn:tests:notification\">"
            "<message>test</message>"
            "</event>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));
    assert_non_null(tree);

    node = find_child(tree, "hostname");
    assert_non_null(node);
    assert_string_equal(lyd_get_value(node), "example.com");

    node = find_child(tree, "sequence-number");
    assert_non_null(node);
    assert_string_equal(lyd_get_value(node), "42");

    lyd_free_all(tree);
}

static void
test_feature_enabled_xml(void **state)
{
    struct lyd_node *tree, *node;

    const char *feats[] = {"hostname-sequence-number", NULL};

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", feats));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<hostname>example-router.example.com</hostname>"
            "<sequence-number>42</sequence-number>"
            "<contents>"
            "<event xmlns=\"urn:tests:notification\">"
            "<message>interface up</message>"
            "</event>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));

    node = find_child(tree, "hostname");
    assert_non_null(node);
    assert_string_equal(lyd_get_value(node), "example-router.example.com");

    node = find_child(tree, "sequence-number");
    assert_non_null(node);
    assert_string_equal(lyd_get_value(node), "42");

    lyd_free_all(tree);
}

static void
test_feature_enabled_json(void **state)
{
    struct lyd_node *tree, *node;

    const char *feats[] = {"hostname-sequence-number", NULL};

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", feats));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    const char *json =
            "{\"ietf-yp-notification:envelope\":{"
            "\"event-time\":\"2024-10-10T08:00:11.22Z\","
            "\"hostname\":\"example-router.example.com\","
            "\"sequence-number\":42,"
            "\"contents\":{"
            "\"test-notif:event\":{\"message\":\"interface up\"}"
            "}}}";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, json, LYD_JSON,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));

    node = find_child(tree, "hostname");
    assert_non_null(node);
    assert_string_equal(lyd_get_value(node), "example-router.example.com");

    node = find_child(tree, "sequence-number");
    assert_non_null(node);
    assert_string_equal(lyd_get_value(node), "42");

    lyd_free_all(tree);
}

/* ============================================================
 * Group 5: Serialization (XML print round-trip)
 * ============================================================ */

static void
test_print_xml(void **state)
{
    struct lyd_node *tree1, *tree2;
    char *printed;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<contents>"
            "<event xmlns=\"urn:tests:notification\">"
            "<message>interface up</message>"
            "</event>"
            "</contents>"
            "</envelope>";

    /* Parse -> print to XML -> re-parse -> compare */
    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, &tree1));

    assert_int_equal(LY_SUCCESS, lyd_print_mem(&printed, tree1, LYD_XML, LYD_PRINT_SIBLINGS));

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, printed, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree2));

    CHECK_LYD(tree1, tree2);

    free(printed);
    lyd_free_all(tree1);
    lyd_free_all(tree2);
}

/* ============================================================
 * Group 6: ietf-yp-observation augmentation
 * ============================================================ */

static void
test_observation_push_update(void **state)
{
    struct lyd_node *tree, *contents, *notif, *node;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yang-push", "2019-09-09", NULL));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-observation", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    /* push-update augmented with observation timestamp and point-in-time */
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<contents>"
            "<push-update xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-push\">"
            "<id>1</id>"
            "<timestamp xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-observation\">"
            "2024-10-10T08:00:10.00Z</timestamp>"
            "<point-in-time xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-observation\">"
            "current-accounting</point-in-time>"
            "</push-update>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));

    contents = find_child(tree, "contents");
    assert_non_null(contents);
    notif = lyd_child_any(contents);
    assert_non_null(notif);
    assert_string_equal(LYD_NAME(notif), "push-update");
    assert_int_equal(notif->schema->nodetype, LYS_NOTIF);

    /* Verify the augmented observation metadata */
    node = find_child(notif, "timestamp");
    assert_non_null(node);
    assert_string_equal(lyd_get_value(node), "2024-10-10T08:00:10.00Z");

    node = find_child(notif, "point-in-time");
    assert_non_null(node);
    assert_string_equal(lyd_get_value(node), "current-accounting");

    lyd_free_all(tree);
}

static void
test_invalid_observation_bad_point_in_time(void **state)
{
    struct lyd_node *tree = NULL;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yang-push", "2019-09-09", NULL));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-observation", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    /* Invalid point-in-time enumeration value */
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<contents>"
            "<push-update xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-push\">"
            "<point-in-time xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-observation\">"
            "invalid-value</point-in-time>"
            "</push-update>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_EVALID, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    UTEST_LOG_CTX_CLEAN;
}

static void
test_observation_point_in_time_values(void **state)
{
    static const char * const values[] = {"current-accounting", "initial-state", "state-changed"};
    uint32_t i;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yang-push", "2019-09-09", NULL));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-observation", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);

    for (i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        struct lyd_node *tree = NULL, *contents, *notif, *node;
        char xml[512];

        snprintf(xml, sizeof(xml),
                "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
                "<event-time>2024-10-10T08:00:11.22Z</event-time>"
                "<contents>"
                "<push-update xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-push\">"
                "<point-in-time xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-observation\">"
                "%s</point-in-time>"
                "</push-update>"
                "</contents>"
                "</envelope>",
                values[i]);

        assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
                LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));
        assert_non_null(tree);

        contents = find_child(tree, "contents");
        assert_non_null(contents);
        notif = lyd_child_any(contents);
        assert_non_null(notif);
        assert_string_equal(LYD_NAME(notif), "push-update");

        node = find_child(notif, "point-in-time");
        assert_non_null(node);
        assert_string_equal(lyd_get_value(node), values[i]);

        lyd_free_all(tree);
    }
}

/* ============================================================
 * Group 7: Edge cases for contents anydata
 * ============================================================ */

static void
test_invalid_non_notif_in_contents(void **state)
{
    struct lyd_node *tree = NULL;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);
    UTEST_ADD_MODULE(notif_yang2, LYS_IN_YANG, NULL, NULL);

    /* A container (not a notification) inside contents - parsing succeeds
     * but the envelope validator rejects it since no notification is found */
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<contents>"
            "<top-container xmlns=\"urn:tests:notification2\">"
            "<val>test</val>"
            "</top-container>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_EVALID, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    UTEST_LOG_CTX_CLEAN;
}

static void
test_invalid_notif_validation_fail(void **state)
{
    struct lyd_node *tree = NULL;

    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-yp-notification", "2025-12-24", NULL));
    UTEST_ADD_MODULE(notif_yang, LYS_IN_YANG, NULL, NULL);
    UTEST_ADD_MODULE(notif_yang2, LYS_IN_YANG, NULL, NULL);

    /* event3 has a mandatory leaf which is missing here; the envelope
     * validator calls lyd_validate_op() on the inner notification which
     * must reject the missing mandatory leaf */
    const char *xml =
            "<envelope xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yp-notification\">"
            "<event-time>2024-10-10T08:00:11.22Z</event-time>"
            "<contents>"
            "<event3 xmlns=\"urn:tests:notification2\"/>"
            "</contents>"
            "</envelope>";

    assert_int_equal(LY_EVALID, lyd_parse_data_mem(UTEST_LYCTX, xml, LYD_XML,
            LYD_PARSE_STRICT, LYD_VALIDATE_NO_STATE | LYD_VALIDATE_PRESENT, &tree));
    assert_null(tree);
    UTEST_LOG_CTX_CLEAN;
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        /* Group 1: valid parsing */
        UTEST(test_parse_xml, setup),
        UTEST(test_parse_json, setup),
        UTEST(test_parse_lyb, setup),

        /* Group 2: invalid envelopes */
        UTEST(test_invalid_bad_event_time, setup),
        UTEST(test_invalid_unknown_metadata, setup),
        UTEST(test_invalid_bad_contents, setup),
        UTEST(test_invalid_missing_event_time, setup),
        UTEST(test_invalid_missing_contents, setup),
        UTEST(test_invalid_empty_contents, setup),
        UTEST(test_invalid_multiple_notifs, setup),
        UTEST(test_invalid_bad_contents_no_strict, setup),

        /* Group 3: sx:augment-structure */
        UTEST(test_augment_xml, setup),
        UTEST(test_augment_json, setup),
        UTEST(test_augment_lyb, setup),
        UTEST(test_augment_multiple, setup),

        /* Group 4: hostname-sequence-number feature */
        UTEST(test_feature_disabled, setup),
        UTEST(test_feature_enabled_optional, setup),
        UTEST(test_feature_disabled_hostname_accepted, setup),
        UTEST(test_feature_enabled_xml, setup),
        UTEST(test_feature_enabled_json, setup),

        /* Group 5: serialization */
        UTEST(test_print_xml, setup),

        /* Group 6: ietf-yp-observation augmentation */
        UTEST(test_observation_push_update, setup),
        UTEST(test_invalid_observation_bad_point_in_time, setup),
        UTEST(test_observation_point_in_time_values, setup),

        /* Group 7: edge cases for contents anydata */
        UTEST(test_invalid_non_notif_in_contents, setup),
        UTEST(test_invalid_notif_validation_fail, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
