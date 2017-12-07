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

Context::Context(ly_ctx *ctx, S_Deleter deleter):
    ctx(ctx),
    deleter(deleter)
{};
Context::Context(const char *search_dir, int options) {
    ctx = ly_ctx_new(search_dir, options);
    if (!ctx) {
        throw std::runtime_error("can not create new context");
    }
    deleter = std::make_shared<Deleter>(ctx);
}
Context::Context(const char *search_dir, const char *path, LYD_FORMAT format, int options) {
    ctx = ly_ctx_new_ylpath(search_dir, path, format, options);
    if (!ctx) {
        throw std::runtime_error("can not create new context");
    }
    deleter = std::make_shared<Deleter>(ctx);
}
Context::Context(const char *search_dir, LYD_FORMAT format, const char *data, int options) {
    ctx = ly_ctx_new_ylmem(search_dir, data, format, options);
    if (!ctx) {
        throw std::runtime_error("can not create new context");
    }
    deleter = std::make_shared<Deleter>(ctx);
}
Context::~Context() {}
S_Data_Node Context::info() {
    struct lyd_node *new_node = ly_ctx_info(ctx);
    return new_node ? std::make_shared<Data_Node>(new_node, deleter) : nullptr;
}
S_Module Context::get_module(const char *name, const char *revision, int implemented) {
    const struct lys_module *module = ly_ctx_get_module(ctx, name, revision, implemented);
    return module ? std::make_shared<Module>((lys_module *) module, deleter) : nullptr;
}
S_Module Context::get_module_older(S_Module module) {
    const struct lys_module *new_module = ly_ctx_get_module_older(ctx, module->module);
    return new_module ? std::make_shared<Module>((lys_module *) new_module, deleter) : nullptr;
}
S_Module Context::load_module(const char *name, const char *revision) {
    const struct lys_module *module = ly_ctx_load_module(ctx, name, revision);
    return module ? std::make_shared<Module>((lys_module *) module, deleter) : nullptr;
}
S_Module Context::get_module_by_ns(const char *ns, const char *revision, int implemented) {
    const struct lys_module *module = ly_ctx_get_module_by_ns(ctx, ns, revision, implemented);
    return module ? std::make_shared<Module>((lys_module *) module, deleter) : nullptr;
}
std::vector<S_Module> *Context::get_module_iter() {
    const struct lys_module *mod = nullptr;
    uint32_t i = 0;

    auto s_vector = new std::vector<S_Module>;

    while ((mod = ly_ctx_get_module_iter(ctx, &i))) {
        if (mod == nullptr) {
            break;
        }
        s_vector->push_back(std::make_shared<Module>((lys_module *) mod, deleter));
    }

    return s_vector;
}
std::vector<S_Module> *Context::get_disabled_module_iter() {
    const struct lys_module *mod = nullptr;
    uint32_t i = 0;

    auto s_vector = new std::vector<S_Module>;

    while ((mod = ly_ctx_get_disabled_module_iter(ctx, &i))) {
        if (mod == nullptr) {
            break;
        }
        s_vector->push_back(std::make_shared<Module>((lys_module *) mod, deleter));
    }

    return s_vector;
}
void Context::clean() {
    return ly_ctx_clean(ctx, nullptr);
}
std::vector<std::string> *Context::get_searchdirs() {
    const char * const *data = ly_ctx_get_searchdirs(ctx);
    if (!data) {
        return nullptr;
    }

    auto s_vector = new std::vector<std::string>;

    int size = 0;
    while (true) {
        if (data[size] == nullptr) {
            break;
        }
        s_vector->push_back(std::string(data[size]));
        size++;
    }

    return s_vector;
};
S_Submodule Context::get_submodule(const char *module, const char *revision, const char *submodule, const char *sub_revision) {
    const struct lys_submodule *tmp_submodule = nullptr;

    tmp_submodule = ly_ctx_get_submodule(ctx, module, revision, submodule, sub_revision);

    return tmp_submodule ? std::make_shared<Submodule>((struct lys_submodule *) tmp_submodule, deleter) : nullptr;
}
S_Submodule Context::get_submodule2(S_Module main_module, const char *submodule) {
    const struct lys_submodule *tmp_submodule = nullptr;

    tmp_submodule = ly_ctx_get_submodule2(main_module->module, submodule);

    return tmp_submodule ? std::make_shared<Submodule>((struct lys_submodule *) tmp_submodule, deleter) : nullptr;
}
S_Schema_Node Context::get_node(S_Schema_Node start, const char *data_path, int output) {
    const struct lys_node *node = nullptr;

    node = ly_ctx_get_node(ctx, start->node, data_path, output);

    return node ? std::make_shared<Schema_Node>((struct lys_node *) node, deleter) : nullptr;
}
S_Data_Node Context::parse_mem(const char *data, LYD_FORMAT format, int options) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_parse_mem(ctx, data, format, options);
    if (!new_node) {
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(new_node, deleter);
    return std::make_shared<Data_Node>(new_node, new_deleter);
}
S_Data_Node Context::parse_fd(int fd, LYD_FORMAT format, int options) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_parse_fd(ctx, fd, format, options);
    if (!new_node) {
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(new_node, deleter);
    return std::make_shared<Data_Node>(new_node, new_deleter);
}
S_Module Context::parse_path(const char *path, LYS_INFORMAT format) {
    struct lys_module *module = nullptr;

    module = (struct lys_module *) lys_parse_path(ctx, path, format);
    if (!module) {
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(module, deleter);
    return std::make_shared<Module>(module, new_deleter);
}
S_Data_Node Context::parse_data_path(const char *path, LYD_FORMAT format, int options) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_parse_path(ctx, path, format, options);
    if (!new_node) {
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(new_node, deleter);
    return std::make_shared<Data_Node>(new_node, new_deleter);
}
S_Data_Node Context::parse_xml(S_Xml_Elem elem, int options) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_parse_xml(ctx, &elem->elem, options);
    if (!new_node) {
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(new_node, deleter);
    return std::make_shared<Data_Node>(new_node, new_deleter);
}

Set::Set() {
    struct ly_set *set = ly_set_new();
    if (!set) {
        throw std::runtime_error("can not create new set");
    }

    set = set;
    deleter = std::make_shared<Deleter>(set);
}
Set::Set(struct ly_set *set, S_Deleter deleter):
    set(set),
    deleter(deleter)
{};
Set::~Set() {}
std::vector<S_Data_Node> *Set::data() {
    auto s_vector = new std::vector<S_Data_Node>;

    unsigned int i;
    for (i = 0; i < set->number; i++){
        s_vector->push_back(std::make_shared<Data_Node>(set->set.d[i], deleter));
    }

    return s_vector;
};
std::vector<S_Schema_Node> *Set::schema() {
    auto s_vector = new std::vector<S_Schema_Node>;

    unsigned int i;
    for (i = 0; i < set->number; i++){
        s_vector->push_back(std::make_shared<Schema_Node>(set->set.s[i], deleter));
    }

    return s_vector;
};
S_Set Set::dup() {
    ly_set *new_set = ly_set_dup(set);
    if (!new_set) {
        return nullptr;
    }

    auto deleter = std::make_shared<Deleter>(new_set);
    return std::make_shared<Set>(new_set, deleter);
}
int Set::add(S_Data_Node node, int options) {
    return ly_set_add(set, (void *) node->node, options);
}
int Set::add(S_Schema_Node node, int options) {
    return ly_set_add(set, (void *) node->node, options);
}
int Set::contains(S_Data_Node node) {
    return ly_set_contains(set, (void *) node->node);
}
int Set::contains(S_Schema_Node node) {
    return ly_set_contains(set, (void *) node->node);
}
int Set::clean() {
    return ly_set_clean(set);
}
int Set::rm(S_Data_Node node) {
    return ly_set_rm(set, (void *) node->node);
}
int Set::rm(S_Schema_Node node) {
    return ly_set_rm(set, (void *) node->node);
}
int Set::rm_index(unsigned int index) {
    return ly_set_rm_index(set, index);
}
