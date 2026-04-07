/**
 * @file plg.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief yang update test plugin
 *
 * Copyright (c) 2026 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE

#include "yang_update.h"

#include <assert.h>
#include <string.h>

static struct lyu_plg_rule rules_findmod1[] = {
    { 0 }
};

struct lyu_plg plg_findmod1_1_3 = {
    .module_name = "find_mod1",
    .revision_old = "2025-01-01",
    .features_old = NULL,
    .imports_old = NULL,
    .revision_new = "2025-03-01",
    .features_new = NULL,
    .imports_new = NULL,
    .rules = rules_findmod1
};

static const char *features_findmod1_6[] = {
    "feat1",
    NULL
};

struct lyu_plg plg_findmod1_3_6 = {
    .module_name = "find_mod1",
    .revision_old = "2025-03-01",
    .features_old = NULL,
    .imports_old = NULL,
    .revision_new = "2025-06-01",
    .features_new = features_findmod1_6,
    .imports_new = NULL,
    .rules = rules_findmod1
};

struct lyu_plg plg_findmod1_6_9 = {
    .module_name = "find_mod1",
    .revision_old = "2025-06-01",
    .features_old = features_findmod1_6,
    .imports_old = NULL,
    .revision_new = "2025-09-01",
    .features_new = features_findmod1_6,
    .imports_new = NULL,
    .rules = rules_findmod1
};

static const char *features_findmod1_12[] = {
    "feat1",
    "feat2",
    NULL
};

struct lyu_plg plg_findmod1_9_12 = {
    .module_name = "find_mod1",
    .revision_old = "2025-09-01",
    .features_old = features_findmod1_6,
    .imports_old = NULL,
    .revision_new = "2025-12-01",
    .features_new = features_findmod1_12,
    .imports_new = NULL,
    .rules = rules_findmod1
};

struct lyu_plg plg_findmod1_no_mod_name = {
    .module_name = "findd_mod1",
    .revision_old = "2025-10-01",
    .features_old = NULL,
    .imports_old = NULL,
    .revision_new = "2025-12-02",
    .features_new = NULL,
    .imports_new = NULL,
    .rules = rules_findmod1
};

struct lyu_plg plg_findmod1_no_rev_old = {
    .module_name = "find_mod1",
    .revision_old = "2025-10-02",
    .features_old = NULL,
    .imports_old = NULL,
    .revision_new = "2025-11-02",
    .features_new = NULL,
    .imports_new = NULL,
    .rules = rules_findmod1
};

static const char *features_findmod1_invalid[] = {
    "feat1",
    "feattt2",
    NULL
};

struct lyu_plg plg_findmod1_no_feature_old = {
    .module_name = "find_mod1",
    .revision_old = "2025-12-01",
    .features_old = features_findmod1_invalid,
    .imports_old = NULL,
    .revision_new = "2025-12-02",
    .features_new = NULL,
    .imports_new = NULL,
    .rules = rules_findmod1
};
