/**
 * @file Tree_Data.hpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Class implementation for libyang C header tree_data.h.
 *
 * Copyright (c) 2017 Deutsche Telekom AG.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef TREE_DATA_H
#define TREE_DATA_H

#include <iostream>
#include <memory>
#include <exception>
#include <vector>

#include "Internal.hpp"
#include "Tree_Schema.hpp"

extern "C" {
#include "libyang.h"
#include "tree_data.h"
}


/* defined */
class Value;
class Data_Node;
class Data_Node_Leaf_List;
class Data_Node_Anydata;
class Attr;
class Difflist;

/* used */
class Schema_Node;
class Xml_Elem;
class Context;
class Type_Enum;
class Ident;

class Value
{
public:
    Value(lyd_val value, uint16_t value_type, S_Deleter deleter);
    ~Value();
    const char *binary() {return LY_TYPE_BINARY == type ? value.binary : throw "wrong type";};
    //struct lys_type_bit **bit();
    //TODO, check size
    //its size is always the number of defined bits in the schema
    int8_t bln() {return LY_TYPE_BOOL == type ? value.bln : throw "wrong type";};
    int64_t dec64() {return LY_TYPE_DEC64 == type ? value.dec64 : throw "wrong type";};
    S_Type_Enum enm() {return LY_TYPE_ENUM == type ? std::make_shared<Type_Enum>(value.enm, deleter) : throw "wrong type";};
    S_Ident ident() {return LY_TYPE_IDENT == type ? std::make_shared<Ident>(value.ident, deleter) : throw "wrong type";};
    S_Data_Node instance();
    int8_t int8() {return LY_TYPE_INT8 == type ? value.int8 : throw "wrong type";};
    int16_t int16() {return LY_TYPE_INT16 == type ? value.int16 : throw "wrong type";};
    int32_t int32() {return LY_TYPE_INT32 == type ? value.int32 : throw "wrong type";};
    int64_t int64() {return LY_TYPE_INT64 == type ? value.int64 : throw "wrong type";};
    S_Data_Node leafref();
    const char *string() {return LY_TYPE_STRING == type ? value.string : throw "wrong type";};
    int8_t uint8() {return LY_TYPE_UINT8 == type ? value.uint8 : throw "wrong type";};
    int16_t uint16() {return LY_TYPE_UINT16 == type ? value.uint16 : throw "wrong type";};
    int32_t uintu32() {return LY_TYPE_UINT32 == type ? value.uint32 : throw "wrong type";};
    int64_t uint64() {return LY_TYPE_UINT64 == type ? value.uint64 : throw "wrong type";};

private:
    lyd_val value;
    uint16_t type;
    S_Deleter deleter;
};

class Data_Node
{
public:
    Data_Node(struct lyd_node *node, S_Deleter deleter = nullptr);
    Data_Node(S_Data_Node parent, S_Module module, const char *name);
    Data_Node(S_Data_Node parent, S_Module module, const char *name, const char *val_str);
    Data_Node(S_Data_Node parent, S_Module module, const char *name, const char *value, LYD_ANYDATA_VALUETYPE value_type);
    Data_Node(S_Data_Node parent, S_Module module, const char *name, S_Data_Node value, LYD_ANYDATA_VALUETYPE value_type);
    Data_Node(S_Data_Node parent, S_Module module, const char *name, S_Xml_Elem value, LYD_ANYDATA_VALUETYPE value_type);
    //TODO
    //struct lyd_node *lyd_new_output(struct lyd_node *parent, const struct lys_module *module, const char *name);
    //struct lyd_node *lyd_new_output_leaf(struct lyd_node *parent, const struct lys_module *module, const char *name,
    //                                     const char *val_str);
    //struct lyd_node *lyd_new_output_leaf(struct lyd_node *parent, const struct lys_module *module, const char *name,
    //                                     void *value, LYD_ANYDATA_VALUETYPE value_type);
    ~Data_Node();
    S_Schema_Node schema() LY_NEW(node, schema, Schema_Node);
    uint8_t validity() {return node->validity;};
    uint8_t dflt() {return node->dflt;};
    uint8_t when_status() {return node->when_status;};
    S_Attr attr();
    S_Data_Node next() LY_NEW(node, next, Data_Node);
    S_Data_Node prev() LY_NEW(node, prev, Data_Node);
    S_Data_Node parent() LY_NEW(node, parent, Data_Node);
    virtual S_Data_Node child() LY_NEW(node, child, Data_Node);

    /* functions */
    std::string path();
    S_Data_Node dup(int recursive);
    S_Data_Node dup_to_ctx(int recursive, S_Context context);
    int merge(S_Data_Node source, int options);
    int merge_to_ctx(S_Data_Node source, int options, S_Context context);
    int insert(S_Data_Node new_node);
    int insert_sibling(S_Data_Node new_node);
    int insert_before(S_Data_Node new_node);
    int insert_after(S_Data_Node new_node);
    int schema_sort(int recursive);
    S_Set find_path(const char *expr);
    S_Set find_instance(S_Schema_Node schema);
    S_Data_Node first_sibling();
    int validate(int options, S_Context var_arg);
    int validate(int options, S_Data_Node var_arg);
    int validate_value(const char *value) {return lyd_validate_value(node->schema, value);};
    S_Difflist diff(S_Data_Node second, int options);
    S_Data_Node new_path(S_Context ctx, const char *path, void *value, LYD_ANYDATA_VALUETYPE value_type, int options);
    unsigned int list_pos() {return lyd_list_pos(node);};
    int unlink() {return lyd_unlink(node);};
    S_Attr insert_attr(S_Module module, const char *name, const char *value);
    S_Module node_module();
    std::string print_mem(LYD_FORMAT format, int options);

    /* emulate TREE macro's */
    std::vector<S_Data_Node> *tree_for();
    std::vector<S_Data_Node> *tree_dfs();

    /* SWIG can not access private variables so it needs public getters */
    struct lyd_node *swig_node() {return node;};
    S_Deleter swig_deleter() {return deleter;};

    friend Set;

    /* for libnetconf2 */
    struct lyd_node *C_lyd_node() {return node;};

private:
    struct lyd_node *node;
    S_Deleter deleter;
};

S_Data_Node create_new_Data_Node(struct lyd_node *node);

class Data_Node_Leaf_List : public Data_Node
{
public:
    Data_Node_Leaf_List(struct lyd_node *node, S_Deleter deleter = nullptr);
    ~Data_Node_Leaf_List();
    const char *value_str() {return ((struct lyd_node_leaf_list *) node)->value_str;};
    S_Value value();
    uint16_t value_type() {return ((struct lyd_node_leaf_list *) node)->value_type;};
    S_Data_Node child() {return nullptr;};

    /* functions */
    int change_leaf(const char *val_str);
    int wd_default();
    S_Type leaf_type();

private:
    struct lyd_node *node;
    S_Deleter deleter;
};

class Data_Node_Anydata : public Data_Node
{
public:
    Data_Node_Anydata(struct lyd_node *node, S_Deleter deleter = nullptr);
    ~Data_Node_Anydata();
    LYD_ANYDATA_VALUETYPE value_type() {return ((struct lyd_node_anydata *) node)->value_type;};
    //union value
    S_Data_Node child() {return nullptr;};

private:
    struct lyd_node *node;
    S_Deleter deleter;
};

class Attr
{
public:
    Attr(struct lyd_attr *attr, S_Deleter deleter = nullptr);
    ~Attr();
    S_Data_Node parent() LY_NEW(attr, parent, Data_Node);
    S_Attr next();
    //struct lys_ext_instance_complex *annotation
    const char *name() {return attr->name;};
    const char *value_str() {return attr->value_str;};
    S_Value value();
    uint16_t value_type() {return attr->value_type;};
private:
    struct lyd_attr *attr;
    S_Deleter deleter;
};

class Difflist
{
public:
    Difflist(struct lyd_difflist *diff, S_Deleter deleter);
    ~Difflist();
    LYD_DIFFTYPE *type() {return diff->type;};
    std::vector<S_Data_Node> *first();
    std::vector<S_Data_Node> *second();

private:
    struct lyd_difflist *diff;
    S_Deleter deleter;
};

#endif
