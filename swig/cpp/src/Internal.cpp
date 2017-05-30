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
#include "../../../src/libyang.h"
#include "../../../src/tree_data.h"
#include "../../../src/tree_schema.h"
}

using namespace std;
Deleter::Deleter(ly_ctx *ctx, S_Deleter parent) {
    _v.ctx = ctx;
    _t = CONTEXT;
    _parent = parent;
};
Deleter::Deleter(struct lyd_node *data, S_Deleter parent) {
    _v.data = data;
    _t = DATA_NODE;
    _parent = parent;
};
Deleter::Deleter(struct lys_node *schema, S_Deleter parent) {
    _v.schema = schema;
    _t = SCHEMA_NODE;
    _parent = parent;
};
Deleter::Deleter(struct lys_module *module, S_Deleter parent) {
    _v.module = module;
    _t = MODULE;
    _parent = parent;
};
Deleter::Deleter(struct lys_submodule *submodule, S_Deleter parent) {
    _v.submodule = submodule;
    _t = SUBMODULE;
    _parent = parent;
};
Deleter::Deleter(S_Context context, struct lyxml_elem *elem, S_Deleter parent) {
    _context = context;
    _v.elem = elem;
    _t = XML;
    _parent = parent;
};
Deleter::Deleter(struct ly_set *set, S_Deleter parent) {
    _v.set = set;
    _t = SET;
    _parent = parent;
}
Deleter::Deleter(struct lyd_difflist *diff, S_Deleter parent) {
    _v.diff = diff;
    _t = DIFFLIST;
    _parent = parent;
}
Deleter::~Deleter() {
    switch(_t) {
    case CONTEXT:
        if (_v.ctx) ly_ctx_destroy(_v.ctx, NULL);
        _v.ctx = NULL;
        break;
    case DATA_NODE:
        if (_v.data) lyd_free(_v.data);
        _v.data = NULL;
        break;
    case SCHEMA_NODE:
        break;
    case MODULE:
        break;
    case SUBMODULE:
        break;
    case XML:
        if (_v.elem) lyxml_free(_context->_ctx, _v.elem);
        _v.elem = NULL;
        break;
    case SET:
        if (_v.set) ly_set_free(_v.set);
        _v.set = NULL;
        break;
    case DIFFLIST:
        if (_v.diff) lyd_free_diff(_v.diff);
        _v.diff = NULL;
    }
};
