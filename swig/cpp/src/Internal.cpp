/**
 * @file Internal.cpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Implementation of header internal helper classes.
 *
 * Copyright (c) 2017 Deutsche Telekom AG.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <iostream>

#include "Internal.hpp"
#include "Libyang.hpp"
#include "Tree_Data.hpp"

extern "C" {
#include "libyang.h"
#include "tree_data.h"
#include "tree_schema.h"
}

Deleter::Deleter(ly_ctx *ctx, S_Deleter parent):
    t(CONTEXT),
    parent(parent),
    context(nullptr)
{
    v.ctx = ctx;
};
Deleter::Deleter(struct lyd_node *data, S_Deleter parent):
    t(DATA_NODE),
    parent(parent),
    context(nullptr)
{
    v.data = data;
};
Deleter::Deleter(struct lys_node *schema, S_Deleter parent):
    t(SCHEMA_NODE),
    parent(parent),
    context(nullptr)
{
    v.schema = schema;
};
Deleter::Deleter(struct lys_module *module, S_Deleter parent):
    t(MODULE),
    parent(parent),
    context(nullptr)
{
    v.module = module;
};
Deleter::Deleter(struct lys_submodule *submodule, S_Deleter parent):
    t(SUBMODULE),
    parent(parent),
    context(nullptr)
{
    v.submodule = submodule;
};
Deleter::Deleter(S_Context context, struct lyxml_elem *elem, S_Deleter parent):
    t(XML),
    parent(parent),
    context(context)
{
    v.elem = elem;
};
Deleter::Deleter(struct ly_set *set, S_Deleter parent):
    t(SET),
    parent(parent),
    context(nullptr)
{
    v.set = set;
}
Deleter::Deleter(struct lyd_difflist *diff, S_Deleter parent):
    t(DIFFLIST),
    parent(parent),
    context(nullptr)
{
    v.diff = diff;
}
Deleter::~Deleter() {
    switch(t) {
    case CONTEXT:
        if (v.ctx) ly_ctx_destroy(v.ctx, nullptr);
        v.ctx = nullptr;
        break;
    case DATA_NODE:
        if (v.data) lyd_free(v.data);
        v.data = nullptr;
        break;
    case SCHEMA_NODE:
        break;
    case MODULE:
        break;
    case SUBMODULE:
        break;
    case XML:
        if (v.elem) lyxml_free(context->ctx, v.elem);
        v.elem = nullptr;
        break;
    case SET:
        if (v.set) ly_set_free(v.set);
        v.set = nullptr;
        break;
    case DIFFLIST:
        if (v.diff) lyd_free_diff(v.diff);
        v.diff = nullptr;
    }
};
