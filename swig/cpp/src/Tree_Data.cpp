/**
 * @file Tree_Data.cpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Implementation of header Tree_Data.hpp.
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
#include "Libyang.hpp"
#include "Tree_Data.hpp"
#include "Tree_Schema.hpp"

extern "C" {
#include "libyang.h"
#include "tree_data.h"
#include "tree_schema.h"
}

Value::Value(lyd_val value, uint16_t value_type, S_Deleter deleter):
    value(value),
    type(value_type),
    deleter(deleter)
{};
Value::~Value() {};
S_Data_Node Value::instance() {
    if (LY_TYPE_INST != type) {
        return nullptr;
    }
    return value.instance ? std::make_shared<Data_Node>(value.instance, deleter) : nullptr;
}
S_Data_Node Value::leafref() {
    if (LY_TYPE_LEAFREF != type) {
        return nullptr;
    }
    return value.leafref ? std::make_shared<Data_Node>(value.leafref, deleter) : nullptr;
}

Data_Node::Data_Node(struct lyd_node *node, S_Deleter deleter):
    node(node),
    deleter(deleter)
{};
Data_Node::Data_Node(S_Data_Node parent, S_Module module, const char *name) {
    lyd_node *new_node = nullptr;

    if (!module) {
        throw std::invalid_argument("Module can not be empty");
    }

    new_node = lyd_new(parent->node, module->module, name);
    if (!new_node) {
        throw std::invalid_argument("libyang could not create new data node, invalid argument");
    }

    node = new_node;
    deleter = nullptr;
};
Data_Node::Data_Node(S_Data_Node parent, S_Module module, const char *name, const char *val_str) {
    lyd_node *new_node = nullptr;

    if (!module) {
        throw std::invalid_argument("Module can not be empty");
    }

    new_node = lyd_new_leaf(parent->node, module->module, name, val_str);
    if (!new_node) {
        throw std::invalid_argument("libyang could not create new data node, invalid argument");
    }

    node = new_node;
    deleter = nullptr;
};
Data_Node::Data_Node(S_Data_Node parent, S_Module module, const char *name, const char *value, LYD_ANYDATA_VALUETYPE value_type) {
    lyd_node *new_node = nullptr;

    if (!module) {
        throw std::invalid_argument("Module can not be empty");
    }

    new_node = lyd_new_anydata(parent->node, module->module, name, (void *) value, value_type);
    if (!new_node) {
        throw std::invalid_argument("libyang could not create new data node, invalid argument");
    }

    node = new_node;
    deleter = nullptr;
};
Data_Node::Data_Node(S_Data_Node parent, S_Module module, const char *name, S_Data_Node value, LYD_ANYDATA_VALUETYPE value_type) {
    lyd_node *new_node = nullptr;

    if (!module) {
        throw std::invalid_argument("Module can not be empty");
    }

    new_node = lyd_new_anydata(parent->node, module->module, name, (void *) value->node, value_type);
    if (!new_node) {
        throw std::invalid_argument("libyang could not create new data node, invalid argument");
    }

    node = new_node;
    deleter = nullptr;
};
Data_Node::Data_Node(S_Data_Node parent, S_Module module, const char *name, S_Xml_Elem value, LYD_ANYDATA_VALUETYPE value_type) {
    lyd_node *new_node = nullptr;

    if (!module) {
        throw std::invalid_argument("Module can not be empty");
    }

    new_node = lyd_new_anydata(parent->node, module->module, name, (void *) value->elem, value_type);
    if (!new_node) {
        throw std::invalid_argument("libyang could not create new data node, invalid argument");
    }

    node = new_node;
    deleter = nullptr;
}
Data_Node::~Data_Node() {};
S_Attr Data_Node::attr() LY_NEW(node, attr, Attr);
std::string Data_Node::path() {
    char *path = nullptr;

    path = lyd_path(node);
    if (!path) {
        return nullptr;
    }

    std::string s_path = path;
    free(path);
    return s_path;
}
S_Data_Node Data_Node::dup(int recursive) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_dup(node, recursive);

    return new_node ? std::make_shared<Data_Node>(new_node, deleter) : nullptr;
}
S_Data_Node Data_Node::dup_to_ctx(int recursive, S_Context context) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_dup_to_ctx(node, recursive, context->ctx);

    return new_node ? std::make_shared<Data_Node>(new_node, deleter) : nullptr;
}
int Data_Node::merge(S_Data_Node source, int options) {
    return lyd_merge(node, source->node, options);
}
int Data_Node::merge_to_ctx(S_Data_Node source, int options, S_Context context) {
    return lyd_merge_to_ctx(&node, source->node, options, context->ctx);
}
int Data_Node::insert(S_Data_Node new_node) {
    return lyd_insert(node, new_node->node);
}
int Data_Node::insert_sibling(S_Data_Node new_node) {
    return lyd_insert_sibling(&node, new_node->node);
}
int Data_Node::insert_before(S_Data_Node new_node) {
    return lyd_insert_before(node, new_node->node);
}
int Data_Node::insert_after(S_Data_Node new_node) {
    return lyd_insert_after(node, new_node->node);
}
int Data_Node::schema_sort(int recursive) {
    return lyd_schema_sort(node, recursive);
}
S_Set Data_Node::find_path(const char *expr) {
    struct ly_set *set = lyd_find_path(node, expr);
    if (!set) {
        return nullptr;
    }

    return std::make_shared<Set>(set, std::make_shared<Deleter>(set, deleter));
}
S_Set Data_Node::find_instance(S_Schema_Node schema) {
    struct ly_set *set = lyd_find_instance(node, schema->node);
    if (!set) {
        return nullptr;
    }

    return std::make_shared<Set>(set, std::make_shared<Deleter>(set, deleter));
}
S_Data_Node Data_Node::first_sibling() {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_first_sibling(node);

    return new_node ? std::make_shared<Data_Node>(new_node, deleter) : nullptr;
}
int Data_Node::validate(int options, S_Context var_arg) {
    return lyd_validate(&node, options, (void *) var_arg->ctx);
}
int Data_Node::validate(int options, S_Data_Node var_arg) {
    return lyd_validate(&node, options, (void *) var_arg->node);
}
S_Difflist Data_Node::diff(S_Data_Node second, int options) {
    struct lyd_difflist *diff;

    diff = lyd_diff(node, second->node, options);

    return diff ? std::make_shared<Difflist>(diff, deleter) : nullptr;
}
S_Data_Node Data_Node::new_path(S_Context ctx, const char *path, void *value, LYD_ANYDATA_VALUETYPE value_type, int options) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_new_path(node, ctx->ctx, path, value, value_type, options);

    return new_node ? std::make_shared<Data_Node>(new_node, deleter) : nullptr;
}
S_Attr Data_Node::insert_attr(S_Module module, const char *name, const char *value) {
    struct lyd_attr *attr = nullptr;

    attr = lyd_insert_attr(node, module->module, name, value);

    return attr ? std::make_shared<Attr>(attr, deleter) : nullptr;
}
S_Module Data_Node::node_module() {
    struct lys_module *module = nullptr;

    module = lyd_node_module(node);

    return module ? std::make_shared<Module>(module, deleter) : nullptr;
}
std::string Data_Node::print_mem(LYD_FORMAT format, int options) {
    char *strp = nullptr;
    int rc = 0;

    rc = lyd_print_mem(&strp, node, format, options);
    if (0 != rc) {
        return nullptr;
    }

    std::string s_strp = strp;
    free(strp);
    return s_strp;

}
std::vector<S_Data_Node> *Data_Node::tree_for() {
    auto s_vector = new std::vector<S_Data_Node>;

    struct lyd_node *elem = nullptr;
    LY_TREE_FOR(node, elem) {
        s_vector->push_back(std::make_shared<Data_Node>(elem, deleter));
    }

    return s_vector;
}
std::vector<S_Data_Node> *Data_Node::tree_dfs() {
    auto s_vector = new std::vector<S_Data_Node>;

    struct lyd_node *elem = nullptr, *next = nullptr;
    LY_TREE_DFS_BEGIN(node, next, elem) {
        s_vector->push_back(std::make_shared<Data_Node>(elem, deleter));
        LY_TREE_DFS_END(node, next, elem)
    }

    return s_vector;
}

Data_Node_Leaf_List::Data_Node_Leaf_List(struct lyd_node *node, S_Deleter deleter):
    Data_Node(node, deleter),
    node(node),
    deleter(deleter)
{};
Data_Node_Leaf_List::~Data_Node_Leaf_List() {};
S_Value Data_Node_Leaf_List::value() {
    struct lyd_node_leaf_list *leaf = (struct lyd_node_leaf_list *) node;
    return std::make_shared<Value>(leaf->value, leaf->value_type, deleter);
}
int Data_Node_Leaf_List::change_leaf(const char *val_str) {
    return lyd_change_leaf((struct lyd_node_leaf_list *) node, val_str);
}
int Data_Node_Leaf_List::wd_default() {
    return lyd_wd_default((struct lyd_node_leaf_list *)node);
}
S_Type Data_Node_Leaf_List::leaf_type() {
    const struct lys_type *type = lyd_leaf_type((const struct lyd_node_leaf_list *) node);
    return std::make_shared<Type>((struct lys_type *) type, deleter);
};

Data_Node_Anydata::Data_Node_Anydata(struct lyd_node *node, S_Deleter deleter):
    Data_Node(node, deleter),
    node(node),
    deleter(deleter)
{};
Data_Node_Anydata::~Data_Node_Anydata() {};

Attr::Attr(struct lyd_attr *attr, S_Deleter deleter):
    attr(attr),
    deleter(deleter)
{};
Attr::~Attr() {};
S_Value Attr::value() {
    struct lyd_node_leaf_list *leaf = (struct lyd_node_leaf_list *) attr;
    return std::make_shared<Value>(leaf->value, leaf->value_type, deleter);
}
S_Attr Attr::next() LY_NEW(attr, next, Attr);

Difflist::Difflist(struct lyd_difflist *diff, S_Deleter deleter) {
    diff = diff;
    deleter = std::make_shared<Deleter>(diff, deleter);
}
Difflist::~Difflist() {};
std::vector<S_Data_Node> *Difflist::first() {
    unsigned int i = 0;
    if (!*diff->first) {
        return nullptr;
    }

    auto s_vector = new std::vector<S_Data_Node>;

    for(i = 0; i < sizeof(*diff->first); i++) {
        s_vector->push_back(std::make_shared<Data_Node>(*diff->first, deleter));
    }

    return s_vector;
}
std::vector<S_Data_Node> *Difflist::second() {
    unsigned int i = 0;
    if (!*diff->second) {
        return nullptr;
    }

    auto s_vector = new std::vector<S_Data_Node>;

    for(i = 0; i < sizeof(*diff->second); i++) {
        s_vector->push_back(std::make_shared<Data_Node>(*diff->second, deleter));
    }

    return s_vector;
}

S_Data_Node create_new_Data_Node(struct lyd_node *new_node) {
    return new_node ? std::make_shared<Data_Node>(new_node, nullptr) : nullptr;
}
