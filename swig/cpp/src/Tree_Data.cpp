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

#include "Xml.hpp"
#include "Libyang.hpp"
#include "Tree_Data.hpp"
#include "Tree_Schema.hpp"

extern "C" {
#include "libyang.h"
#include "tree_data.h"
#include "tree_schema.h"
}

using namespace std;

Value::Value(lyd_val value, uint16_t value_type, S_Deleter deleter) {
    _value = value;
    _type = value_type;
    _deleter = deleter;
}
Value::~Value() {};
S_Data_Node Value::instance() {
    if (LY_TYPE_INST != _type) {
        return NULL;
    }
    return _value.instance ? S_Data_Node(new Data_Node(_value.instance, _deleter)) : NULL;
}
S_Data_Node Value::leafref() {
    if (LY_TYPE_LEAFREF != _type) {
        return NULL;
    }
    return _value.leafref ? S_Data_Node(new Data_Node(_value.leafref, _deleter)) : NULL;
}

Data_Node::Data_Node(struct lyd_node *node, S_Deleter deleter) {
    _node = node;
    _deleter = deleter;
};
Data_Node::Data_Node(S_Data_Node parent, S_Module module, const char *name) {
    lyd_node *node = NULL;

    if (NULL == module) {
        throw std::invalid_argument("Module can not be empty");
    }

    node = lyd_new(parent->_node, module->_module, name);
    if (NULL == node) {
        throw std::invalid_argument("libyang could not create new data node, invalid argument");
    }

    _node = node;
    _deleter = NULL;
};
Data_Node::Data_Node(S_Data_Node parent, S_Module module, const char *name, const char *val_str) {
    lyd_node *node = NULL;

    if (NULL == module) {
        throw std::invalid_argument("Module can not be empty");
    }

    node = lyd_new_leaf(parent->_node, module->_module, name, val_str);
    if (NULL == node) {
        throw std::invalid_argument("libyang could not create new data node, invalid argument");
    }

    _node = node;
    _deleter = NULL;
};
Data_Node::Data_Node(S_Data_Node parent, S_Module module, const char *name, const char *value, LYD_ANYDATA_VALUETYPE value_type) {
    lyd_node *node = NULL;

    if (NULL == module) {
        throw std::invalid_argument("Module can not be empty");
    }

    node = lyd_new_anydata(parent->_node, module->_module, name, (void *) value, value_type);
    if (NULL == node) {
        throw std::invalid_argument("libyang could not create new data node, invalid argument");
    }

    _node = node;
    _deleter = NULL;
};
Data_Node::Data_Node(S_Data_Node parent, S_Module module, const char *name, S_Data_Node value, LYD_ANYDATA_VALUETYPE value_type) {
    lyd_node *node = NULL;

    if (NULL == module) {
        throw std::invalid_argument("Module can not be empty");
    }

    node = lyd_new_anydata(parent->_node, module->_module, name, (void *) value->_node, value_type);
    if (NULL == node) {
        throw std::invalid_argument("libyang could not create new data node, invalid argument");
    }

    _node = node;
    _deleter = NULL;
};
Data_Node::Data_Node(S_Data_Node parent, S_Module module, const char *name, S_Xml_Elem value, LYD_ANYDATA_VALUETYPE value_type) {
    lyd_node *node = NULL;

    if (NULL == module) {
        throw std::invalid_argument("Module can not be empty");
    }

    node = lyd_new_anydata(parent->_node, module->_module, name, (void *) value->_elem, value_type);
    if (NULL == node) {
        throw std::invalid_argument("libyang could not create new data node, invalid argument");
    }

    _node = node;
    _deleter = NULL;
}
Data_Node::~Data_Node() {};
S_Attr Data_Node::attr() NEW(_node, attr, Attr);
S_String Data_Node::path() {
    char *path = NULL;

    path = lyd_path(_node);
    if (NULL == path) {
        return NULL;
    }

    S_String s_path = path;
    free(path);
    return s_path;
}
S_Data_Node Data_Node::dup(int recursive) {
    struct lyd_node *node = NULL;

    node = lyd_dup(_node, recursive);

    return node ? S_Data_Node(new Data_Node(node, _deleter)) : NULL;
}
S_Data_Node Data_Node::dup_to_ctx(int recursive, S_Context context) {
    struct lyd_node *node = NULL;

    node = lyd_dup_to_ctx(_node, recursive, context->_ctx);

    return node ? S_Data_Node(new Data_Node(node, _deleter)) : NULL;
}
int Data_Node::merge(S_Data_Node source, int options) {
    return lyd_merge(_node, source->_node, options);
}
int Data_Node::merge_to_ctx(S_Data_Node source, int options, S_Context context) {
    return lyd_merge_to_ctx(&_node, source->_node, options, context->_ctx);
}
int Data_Node::insert(S_Data_Node node) {
    return lyd_insert(_node, node->_node);
}
int Data_Node::insert_sibling(S_Data_Node node) {
    return lyd_insert_sibling(&_node, node->_node);
}
int Data_Node::insert_before(S_Data_Node node) {
    return lyd_insert_before(_node, node->_node);
}
int Data_Node::insert_after(S_Data_Node node) {
    return lyd_insert_after(_node, node->_node);
}
int Data_Node::schema_sort(int recursive) {
    return lyd_schema_sort(_node, recursive);
}
S_Set Data_Node::find_path(const char *expr) {
    struct ly_set *set = lyd_find_path(_node, expr);
    if (NULL == set) {
        return NULL;
    }

    return S_Set(new Set(set, S_Deleter(new Deleter(set, _deleter))));
}
S_Set Data_Node::find_instance(S_Schema_Node schema) {
    struct ly_set *set = lyd_find_instance(_node, schema->_node);
    if (NULL == set) {
        return NULL;
    }

    return S_Set(new Set(set, S_Deleter(new Deleter(set, _deleter))));
}
S_Data_Node Data_Node::first_sibling() {
    struct lyd_node *node = NULL;

    node = lyd_first_sibling(_node);

    return node ? S_Data_Node(new Data_Node(node, _deleter)) : NULL;
}
int Data_Node::validate(int options, S_Context var_arg) {
    return lyd_validate(&_node, options, (void *) var_arg->_ctx);
}
int Data_Node::validate(int options, S_Data_Node var_arg) {
    return lyd_validate(&_node, options, (void *) var_arg->_node);
}
S_Difflist Data_Node::diff(S_Data_Node second, int options) {
    struct lyd_difflist *diff;

    diff = lyd_diff(_node, second->_node, options);

    return diff ? S_Difflist(new Difflist(diff, _deleter)) : NULL;
}
S_Data_Node Data_Node::new_path(S_Context ctx, const char *path, void *value, LYD_ANYDATA_VALUETYPE value_type, int options) {
    struct lyd_node *node = NULL;

    node = lyd_new_path(_node, ctx->_ctx, path, value, value_type, options);

    return node ? S_Data_Node(new Data_Node(node, _deleter)) : NULL;
}
S_Attr Data_Node::insert_attr(S_Module module, const char *name, const char *value) {
    struct lyd_attr *attr = NULL;

    attr = lyd_insert_attr(_node, module->_module, name, value);

    return attr ? S_Attr(new Attr(attr, _deleter)) : NULL;
}
S_Module Data_Node::node_module() {
    struct lys_module *module = NULL;

    module = lyd_node_module(_node);

    return module ? S_Module(new Module(module, _deleter)) : NULL;
}
S_String Data_Node::print_mem(LYD_FORMAT format, int options) {
    char *strp = NULL;
    int rc = 0;

    rc = lyd_print_mem(&strp, _node, format, options);
    if (0 != rc) {
        return NULL;
    }

    S_String s_strp = strp;
    free(strp);
    return s_strp;

}
std::vector<S_Data_Node> *Data_Node::tree_for() {
    auto s_vector = new vector<S_Data_Node>;

    if (NULL == s_vector) {
        return NULL;
    }

    struct lyd_node *elem = NULL;
    LY_TREE_FOR(_node, elem) {
        s_vector->push_back(S_Data_Node(new Data_Node(elem, _deleter)));
    }

    return s_vector;
}
std::vector<S_Data_Node> *Data_Node::tree_dfs() {
    auto s_vector = new vector<S_Data_Node>;

    if (NULL == s_vector) {
        return NULL;
    }

    struct lyd_node *elem = NULL, *next = NULL;
    LY_TREE_DFS_BEGIN(_node, next, elem) {
        s_vector->push_back(S_Data_Node(new Data_Node(elem, _deleter)));
        LY_TREE_DFS_END(_node, next, elem)
    }

    return s_vector;
}

Data_Node_Leaf_List::Data_Node_Leaf_List(struct lyd_node *node, S_Deleter deleter) : Data_Node(node, deleter) {
    _node = node;
    _deleter = deleter;
};
Data_Node_Leaf_List::~Data_Node_Leaf_List() {};
S_Value Data_Node_Leaf_List::value() {
    struct lyd_node_leaf_list *leaf = (struct lyd_node_leaf_list *) _node;
    return S_Value(new Value(leaf->value, leaf->value_type, _deleter));
}
int Data_Node_Leaf_List::change_leaf(const char *val_str) {
    return lyd_change_leaf((struct lyd_node_leaf_list *) _node, val_str);
}
int Data_Node_Leaf_List::wd_default() {
    return lyd_wd_default((struct lyd_node_leaf_list *)_node);
}
S_Type Data_Node_Leaf_List::leaf_type() {
    const struct lys_type *type = lyd_leaf_type((const struct lyd_node_leaf_list *) _node);
    return S_Type(new Type((struct lys_type *) type, _deleter));
};

Data_Node_Anydata::Data_Node_Anydata(struct lyd_node *node, S_Deleter deleter) : Data_Node(node, deleter) {
    _node = node;
    _deleter = deleter;
};
Data_Node_Anydata::~Data_Node_Anydata() {};

Attr::Attr(struct lyd_attr *attr, S_Deleter deleter) {
    _attr = attr;
    _deleter = deleter;
};
Attr::~Attr() {};
S_Value Attr::value() {
    struct lyd_node_leaf_list *leaf = (struct lyd_node_leaf_list *) _attr;
    return S_Value(new Value(leaf->value, leaf->value_type, _deleter));
}
S_Attr Attr::next() NEW(_attr, next, Attr);

Difflist::Difflist(struct lyd_difflist *diff, S_Deleter deleter) {
    _diff = diff;
    _deleter = S_Deleter(new Deleter(diff, deleter));
}
Difflist::~Difflist() {};
std::vector<S_Data_Node> *Difflist::first() {
    unsigned int i = 0;
    if (NULL == *_diff->first) {
        return NULL;
    }

    auto s_vector = new vector<S_Data_Node>;
    if (NULL == s_vector) {
        return NULL;
    }

    for(i = 0; i < sizeof(*_diff->first); i++) {
        s_vector->push_back(S_Data_Node(new Data_Node(*_diff->first, _deleter)));
    }

    return s_vector;
}
std::vector<S_Data_Node> *Difflist::second() {
    unsigned int i = 0;
    if (NULL == *_diff->second) {
        return NULL;
    }

    auto s_vector = new vector<S_Data_Node>;
    if (NULL == s_vector) {
        return NULL;
    }

    for(i = 0; i < sizeof(*_diff->second); i++) {
        s_vector->push_back(S_Data_Node(new Data_Node(*_diff->second, _deleter)));
    }

    return s_vector;
}

S_Data_Node create_new_Data_Node(struct lyd_node *node) {
    return node ? S_Data_Node(new Data_Node(node, NULL)) : NULL;
}
