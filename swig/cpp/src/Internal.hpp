/**
 * @file Internal.hpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Internal C++ helper class
 *
 * Copyright (c) 2017 Deutsche Telekom AG.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef INTERNAL_H
#define INTERNAL_H

#define S_String                 std::string
#define S_Deleter                std::shared_ptr<Deleter>

/* Xml.hpp */
#define S_Xml_Ns                 std::shared_ptr<Xml_Ns>
#define S_Xml_Attr               std::shared_ptr<Xml_Attr>
#define S_Xml_Elem               std::shared_ptr<Xml_Elem>

/* Libyang.hpp */
#define S_Context                std::shared_ptr<Context>
#define S_Set                    std::shared_ptr<Set>

/* Tree_Data.hpp */
#define S_Value                  std::shared_ptr<Value>
#define S_Data_Node              std::shared_ptr<Data_Node>
#define S_Data_Node_Leaf_List    std::shared_ptr<Data_Node_Leaf_List>
#define S_Data_Node_Anydata      std::shared_ptr<Data_Node_Anydata>
#define S_Attr                   std::shared_ptr<Attr>
#define S_Difflist               std::shared_ptr<Difflist>

/* Tree_Schema.hpp */
#define S_Module                 std::shared_ptr<Module>
#define S_Submodule              std::shared_ptr<Submodule>
#define S_Type_Info_Binary       std::shared_ptr<Type_Info_Binary>
#define S_Type_Bit               std::shared_ptr<Type_Bit>
#define S_Type_Info_Bits         std::shared_ptr<Type_Info_Bits>
#define S_Type_Info_Dec64        std::shared_ptr<Type_Info_Dec64>
#define S_Type_Enum              std::shared_ptr<Type_Enum>
#define S_Type_Info_Enums        std::shared_ptr<Type_Info_Enums>
#define S_Type_Info_Ident        std::shared_ptr<Type_Info_Ident>
#define S_Type_Info_Inst         std::shared_ptr<Type_Info_Inst>
#define S_Type_Info_Num          std::shared_ptr<Type_Info_Num>
#define S_Type_Info_Num          std::shared_ptr<Type_Info_Num>
#define S_Type_Info_Lref         std::shared_ptr<Type_Info_Lref>
#define S_Type_Info_Str          std::shared_ptr<Type_Info_Str>
#define S_Type_Info_Union        std::shared_ptr<Type_Info_Union>
#define S_Type_Info              std::shared_ptr<Type_Info>
#define S_Type                   std::shared_ptr<Type>
#define S_Iffeature              std::shared_ptr<Iffeature>
#define S_Ext_Instance           std::shared_ptr<Ext_Instance>
#define S_Revision               std::shared_ptr<Revision>
#define S_Schema_Node            std::shared_ptr<Schema_Node>
#define S_Schema_Node_Container  std::shared_ptr<Schema_Node_Container>
#define S_Schema_Node_Choice     std::shared_ptr<Schema_Node_Choice>
#define S_Schema_Node_Leaf       std::shared_ptr<Schema_Node_Leaf>
#define S_Schema_Node_Leaflist   std::shared_ptr<Schema_Node_Leaflist>
#define S_Schema_Node_List       std::shared_ptr<Schema_Node_List>
#define S_Schema_Node_Anydata    std::shared_ptr<Schema_Node_Anydata>
#define S_Schema_Node_Uses       std::shared_ptr<Schema_Node_Uses>
#define S_Schema_Node_Grp        std::shared_ptr<Schema_Node_Grp>
#define S_Schema_Node_Case       std::shared_ptr<Schema_Node_Case>
#define S_Schema_Node_Inout      std::shared_ptr<Schema_Node_Inout>
#define S_Schema_Node_Notif      std::shared_ptr<Schema_Node_Notif>
#define S_Schema_Node_Action     std::shared_ptr<Schema_Node_Action>
#define S_Schema_Node_Augment    std::shared_ptr<Schema_Node_Augment>
#define S_When                   std::shared_ptr<When>
#define S_Substmt                std::shared_ptr<Substmt>
#define S_Ext                    std::shared_ptr<Ext>
#define S_Refine_Mod_List        std::shared_ptr<Refine_Mod_List>
#define S_Refine_Mod             std::shared_ptr<Refine_Mod>
#define S_Refine                 std::shared_ptr<Refine>
#define S_Deviate                std::shared_ptr<Deviate>
#define S_Deviation              std::shared_ptr<Deviation>
#define S_Import                 std::shared_ptr<Import>
#define S_Include                std::shared_ptr<Include>
#define S_Tpdf                   std::shared_ptr<Tpdf>
#define S_Unique                 std::shared_ptr<Unique>
#define S_Feature                std::shared_ptr<Feature>
#define S_Restr                  std::shared_ptr<Restr>
#define S_Ident                  std::shared_ptr<Ident>

#define NEW(data, element, class)                                                                                                                    \
    {                                                                                                                                                \
        return data->element ? S_##class(new class(data->element, _deleter)) : NULL;                                                                 \
    };

#define NEW_CASTED(cast, data, element, class)                                                                                                       \
    {                                                                                                                                                \
        cast *node = (struct cast *) data;                                                                                                           \
        return node->element ? S_##class(new class(node->element, _deleter)) : NULL;                                                                 \
    };

#define NEW_LIST(data, element, size, class)                                                                                                         \
    {                                                                                                                                                \
        auto s_vector = new vector<S_##class>;                                                                                                       \
        if (NULL == s_vector) {                                                                                                                      \
            return NULL;                                                                                                                             \
        }                                                                                                                                            \
                                                                                                                                                     \
        for (uint8_t i = 0; i < data->size; i++) {                                                                                                   \
            s_vector->push_back(S_##class(new class(&data->element[i], _deleter)));                                                                  \
        }                                                                                                                                            \
                                                                                                                                                     \
        return s_vector;                                                                                                                             \
    };

#define NEW_LIST_CASTED(cast, data, element, size, class)                                                                                            \
    {                                                                                                                                                \
        struct cast *node = (struct cast *) data;                                                                                                    \
        NEW_LIST(node, element, size, class);                                                                                                        \
    };

#define NEW_P_LIST(data, element, size, class)                                                                                                       \
    {                                                                                                                                                \
        auto s_vector = new vector<S_##class>;                                                                                                       \
        if (NULL == s_vector) {                                                                                                                      \
            return NULL;                                                                                                                             \
        }                                                                                                                                            \
                                                                                                                                                     \
        for (uint8_t i = 0; i < data->size; i++) {                                                                                                   \
            s_vector->push_back(S_##class(new class(data->element[i], _deleter)));                                                                   \
        }                                                                                                                                            \
                                                                                                                                                     \
        return s_vector;                                                                                                                             \
    };

#define NEW_P_LIST_CASTED(cast, data, element, size, class)                                                                                          \
    {                                                                                                                                                \
        struct cast *node = (struct cast *) data;                                                                                                    \
        NEW_P_LIST(node, element, size, class);                                                                                                      \
    };

#define NEW_STRING_LIST(data, element, size)                                                                                                         \
    {                                                                                                                                                \
        auto s_vector = new vector<S_String>;                                                                                                        \
        if (NULL == s_vector) {                                                                                                                      \
            return NULL;                                                                                                                             \
        }                                                                                                                                            \
                                                                                                                                                     \
        for (uint8_t i = 0; i < data->size; i++) {                                                                                                   \
            s_vector->push_back(std::string(data->element[i]));                                                                                      \
        }                                                                                                                                            \
                                                                                                                                                     \
        return s_vector;                                                                                                                             \
    };

#include <iostream>
#include <memory>

extern "C" {
#include "libyang.h"
}

#define typeof(x) __typeof__(x)

using namespace std;

/* defined */
class Deleter;

/* used */
class Context;

typedef enum free_type_e {
    CONTEXT,
    DATA_NODE,
    //TODO DATA_NODE_WITHSIBLINGS,
    SCHEMA_NODE,
    MODULE,
    SUBMODULE,
    XML,
    SET,
    DIFFLIST,
} free_type_t;

typedef union value_e {
    struct ly_ctx *ctx;
    struct lyd_node *data;
    struct lys_node *schema;
    struct lys_module *module;
    struct lys_submodule *submodule;
    struct lyxml_elem *elem;
    struct ly_set *set;
    struct lyd_difflist *diff;
} value_t;

class Deleter
{
public:
    Deleter(ly_ctx *ctx, S_Deleter parent = NULL);
    Deleter(struct lyd_node *data, S_Deleter parent = NULL);
    Deleter(struct lys_node *schema, S_Deleter parent = NULL);
    Deleter(struct lys_module *module, S_Deleter parent = NULL);
    Deleter(struct lys_submodule *submodule, S_Deleter parent = NULL);
    Deleter(S_Context context, struct lyxml_elem *elem, S_Deleter parent = NULL);
    Deleter(struct ly_set *set, S_Deleter parent = NULL);
    Deleter(struct lyd_difflist *diff, S_Deleter parent = NULL);
    ~Deleter();

private:
    S_Context _context;
    value_t _v;
    free_type_t _t;
    S_Deleter _parent;
};

#endif
