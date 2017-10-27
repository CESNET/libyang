/**
 * @file Libyang.cpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Implementation of header Libyang.hpp
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
#include <memory>
#include <stdexcept>
#include <vector>

#include "Xml.hpp"
#include "Internal.hpp"
#include "Libyang.hpp"
#include "Tree_Data.hpp"
#include "Tree_Schema.hpp"

extern "C" {
#include "libyang.h"
#include "tree_data.h"
#include "tree_schema.h"
}

using namespace std;

Context::Context(ly_ctx *ctx, S_Deleter deleter) {
    _ctx = ctx;
    _deleter = deleter;
}
Context::Context(const char *search_dir, int options) {
    _ctx = ly_ctx_new(search_dir, options);
    if (NULL == _ctx) {
        throw runtime_error("can not create new context");
    }
    _deleter = S_Deleter(new Deleter(_ctx));
}
Context::Context(const char *search_dir, const char *path, LYD_FORMAT format, int options) {
    _ctx = ly_ctx_new_ylpath(search_dir, path, format, options);
    if (NULL == _ctx) {
        throw runtime_error("can not create new context");
    }
    _deleter = S_Deleter(new Deleter(_ctx));
}
Context::Context(const char *search_dir, LYD_FORMAT format, const char *data, int options) {
    _ctx = ly_ctx_new_ylmem(search_dir, data, format, options);
    if (NULL == _ctx) {
        throw runtime_error("can not create new context");
    }
    _deleter = S_Deleter(new Deleter(_ctx));
}
Context::~Context() {}
S_Data_Node Context::info() {
    struct lyd_node *node = ly_ctx_info(_ctx);
    return node ? S_Data_Node(new Data_Node(node, _deleter)) : NULL;
}
S_Module Context::get_module(const char *name, const char *revision, int implemented) {
    const struct lys_module *module = ly_ctx_get_module(_ctx, name, revision, implemented);
    return module ? S_Module(new Module((lys_module *) module, _deleter)) : NULL;
}
S_Module Context::get_module_older(S_Module module) {
    const struct lys_module *new_module = ly_ctx_get_module_older(_ctx, module->_module);
    return new_module ? S_Module(new Module((lys_module *) new_module, _deleter)) : NULL;
}
S_Module Context::load_module(const char *name, const char *revision) {
    const struct lys_module *module = ly_ctx_load_module(_ctx, name, revision);
    return module ? S_Module(new Module((lys_module *) module, _deleter)) : NULL;
}
S_Module Context::get_module_by_ns(const char *ns, const char *revision, int implemented) {
    const struct lys_module *module = ly_ctx_get_module_by_ns(_ctx, ns, revision, implemented);
    return module ? S_Module(new Module((lys_module *) module, _deleter)) : NULL;
}
vector<S_Module> *Context::get_module_iter() {
    const struct lys_module *mod = NULL;
    uint32_t i = 0;

    auto s_vector = new vector<S_Module>;
    if (NULL == s_vector) {
        return NULL;
    }

    while ((mod = ly_ctx_get_module_iter(_ctx, &i))) {
        if (mod == NULL) {
            break;
        }
        s_vector->push_back(S_Module(new Module((lys_module *) mod, _deleter)));
    }

    return s_vector;
}
vector<S_Module> *Context::get_disabled_module_iter() {
    const struct lys_module *mod = NULL;
    uint32_t i = 0;

    auto s_vector = new vector<S_Module>;
    if (NULL == s_vector) {
        return NULL;
    }

    while ((mod = ly_ctx_get_disabled_module_iter(_ctx, &i))) {
        if (mod == NULL) {
            break;
        }
        s_vector->push_back(S_Module(new Module((lys_module *) mod, _deleter)));
    }

    return s_vector;
}
void Context::clean() {
    return ly_ctx_clean(_ctx, NULL);
}
vector<string> *Context::get_searchdirs() {
    const char * const *data = ly_ctx_get_searchdirs(_ctx);
    if (NULL == data) {
        return NULL;
    }

    auto s_vector = new vector<string>;
    if (NULL == s_vector) {
        return NULL;
    }

    int size = 0;
    while (true) {
        if (data[size] == NULL) {
            break;
        }
        s_vector->push_back(std::string(data[size]));
        size++;
    }

    return s_vector;
};
S_Submodule Context::get_submodule(const char *module, const char *revision, const char *submodule, const char *sub_revision) {
    const struct lys_submodule *tmp_submodule = NULL;

    tmp_submodule = ly_ctx_get_submodule(_ctx, module, revision, submodule, sub_revision);

    return tmp_submodule ? S_Submodule(new Submodule((struct lys_submodule *) tmp_submodule, _deleter)) : NULL;
}
S_Submodule Context::get_submodule2(S_Module main_module, const char *submodule) {
    const struct lys_submodule *tmp_submodule = NULL;

    tmp_submodule = ly_ctx_get_submodule2(main_module->_module, submodule);

    return tmp_submodule ? S_Submodule(new Submodule((struct lys_submodule *) tmp_submodule, _deleter)) : NULL;
}
S_Schema_Node Context::get_node(S_Schema_Node start, const char *data_path, int output) {
    const struct lys_node *node = NULL;

    node = ly_ctx_get_node(_ctx, start->_node, data_path, output);

    return node ? S_Schema_Node(new Schema_Node((struct lys_node *) node, _deleter)) : NULL;
}
S_Data_Node Context::parse_mem(const char *data, LYD_FORMAT format, int options) {
    struct lyd_node *node = NULL;

    node = lyd_parse_mem(_ctx, data, format, options);
    if (NULL == node) {
        return NULL;
    }

    S_Deleter deleter = S_Deleter(new Deleter(node, _deleter));
    return S_Data_Node(new Data_Node(node, deleter));
}
S_Data_Node Context::parse_fd(int fd, LYD_FORMAT format, int options) {
    struct lyd_node *node = NULL;

    node = lyd_parse_fd(_ctx, fd, format, options);
    if (NULL == node) {
        return NULL;
    }

    S_Deleter deleter = S_Deleter(new Deleter(node, _deleter));
    return S_Data_Node(new Data_Node(node, deleter));
}
S_Module Context::parse_path(const char *path, LYS_INFORMAT format) {
    struct lys_module *module = NULL;

    module = (struct lys_module *) lys_parse_path(_ctx, path, format);
    if (NULL == module) {
        return NULL;
    }

    S_Deleter deleter = S_Deleter(new Deleter(module, _deleter));
    return S_Module(new Module(module, deleter));
}
S_Data_Node Context::parse_data_path(const char *path, LYD_FORMAT format, int options) {
    struct lyd_node *node = NULL;

    node = lyd_parse_path(_ctx, path, format, options);
    if (NULL == node) {
        return NULL;
    }

    S_Deleter deleter = S_Deleter(new Deleter(node, _deleter));
    return S_Data_Node(new Data_Node(node, deleter));
}
S_Data_Node Context::parse_xml(S_Xml_Elem elem, int options) {
    struct lyd_node *node = NULL;

    node = lyd_parse_xml(_ctx, &elem->_elem, options);
    if (NULL == node) {
        return NULL;
    }

    S_Deleter deleter = S_Deleter(new Deleter(node, _deleter));
    return S_Data_Node(new Data_Node(node, deleter));
}

Set::Set() {
    struct ly_set *set = ly_set_new();
    if (NULL == _set) {
        throw runtime_error("can not create new set");
    }

    _set = set;
    _deleter = S_Deleter(new Deleter(_set));
}
Set::Set(struct ly_set *set, S_Deleter deleter) {
    _set = set;
    _deleter = deleter;
}
Set::~Set() {}
vector<S_Data_Node> *Set::data() {
    auto s_vector = new vector<S_Data_Node>;
    if (NULL == s_vector) {
        return NULL;
    }

    unsigned int i;
    for (i = 0; i < _set->number; i++){
        s_vector->push_back(S_Data_Node(new Data_Node(_set->set.d[i], _deleter)));
    }

    return s_vector;
};
vector<S_Schema_Node> *Set::schema() {
    auto s_vector = new vector<S_Schema_Node>;
    if (NULL == s_vector) {
        return NULL;
    }

    unsigned int i;
    for (i = 0; i < _set->number; i++){
        s_vector->push_back(S_Schema_Node(new Schema_Node(_set->set.s[i], _deleter)));
    }

    return s_vector;
};
S_Set Set::dup() {
    ly_set *set = ly_set_dup(_set);
    if (NULL == set) {
        return NULL;
    }

    auto deleter = S_Deleter(new Deleter(set));
    return S_Set(new Set(set, deleter));
}
int Set::add(S_Data_Node node, int options) {
    return ly_set_add(_set, (void *) node->_node, options);
}
int Set::add(S_Schema_Node node, int options) {
    return ly_set_add(_set, (void *) node->_node, options);
}
int Set::contains(S_Data_Node node) {
    return ly_set_contains(_set, (void *) node->_node);
}
int Set::contains(S_Schema_Node node) {
    return ly_set_contains(_set, (void *) node->_node);
}
int Set::clean() {
    return ly_set_clean(_set);
}
int Set::rm(S_Data_Node node) {
    return ly_set_rm(_set, (void *) node->_node);
}
int Set::rm(S_Schema_Node node) {
    return ly_set_rm(_set, (void *) node->_node);
}
int Set::rm_index(unsigned int index) {
    return ly_set_rm_index(_set, index);
}
