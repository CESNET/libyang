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
    t(Free_Type::CONTEXT),
    parent(parent),
    context(nullptr)
{
    v.ctx = ctx;
};
Deleter::Deleter(struct lyd_node *data, S_Deleter parent):
    t(Free_Type::DATA_NODE),
    parent(parent),
    context(nullptr)
{
    v.data = data;
};
Deleter::Deleter(struct lys_node *schema, S_Deleter parent):
    t(Free_Type::SCHEMA_NODE),
    parent(parent),
    context(nullptr)
{
    v.schema = schema;
};
Deleter::Deleter(struct lys_module *module, S_Deleter parent):
    t(Free_Type::MODULE),
    parent(parent),
    context(nullptr)
{
    v.module = module;
};
Deleter::Deleter(struct lys_submodule *submodule, S_Deleter parent):
    t(Free_Type::SUBMODULE),
    parent(parent),
    context(nullptr)
{
    v.submodule = submodule;
};
Deleter::Deleter(S_Context context, struct lyxml_elem *elem, S_Deleter parent):
    t(Free_Type::XML),
    parent(parent),
    context(context)
{
    v.elem = elem;
};
Deleter::Deleter(struct ly_set *set, S_Deleter parent):
    t(Free_Type::SET),
    parent(parent),
    context(nullptr)
{
    v.set = set;
}
Deleter::Deleter(struct lyd_difflist *diff, S_Deleter parent):
    t(Free_Type::DIFFLIST),
    parent(parent),
    context(nullptr)
{
    v.diff = diff;
}
Deleter::~Deleter() {
    switch(t) {
    case Free_Type::CONTEXT:
        if (v.ctx) ly_ctx_destroy(v.ctx, nullptr);
        v.ctx = nullptr;
        break;
    case Free_Type::DATA_NODE:
        if (v.data) lyd_free(v.data);
        v.data = nullptr;
        break;
    case Free_Type::SCHEMA_NODE:
        break;
    case Free_Type::MODULE:
        break;
    case Free_Type::SUBMODULE:
        break;
    case Free_Type::XML:
        if (v.elem) lyxml_free(context->ctx, v.elem);
        v.elem = nullptr;
        break;
    case Free_Type::SET:
        if (v.set) ly_set_free(v.set);
        v.set = nullptr;
        break;
    case Free_Type::DIFFLIST:
        if (v.diff) lyd_free_diff(v.diff);
        v.diff = nullptr;
        break;
    }
};
