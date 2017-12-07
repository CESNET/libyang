/**
 * @file Tree_Schema.hpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Class implementation for libyang C header tree_schema.h
 *
 * Copyright (c) 2017 Deutsche Telekom AG.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef TREE_SCHEMA_H
#define TREE_SCHEMA_H

#include <iostream>
#include <memory>
#include <exception>
#include <vector>

#include "Internal.hpp"
#include "Libyang.hpp"

extern "C" {
#include "libyang.h"
#include "tree_schema.h"
}

/* defined */
class Module;
class Submodule;
class Type_Info_Binary;
class Type_Bit;
class Type_Info_Bits;
class Type_Info_Dec64;
class Type_Info_Enum;
class Type_Info_Enums;
class Type_Info_Ident;
class Type_Info_Inst;
class Type_Info_Num;
class Type_Info_Lref;
class Type_Info_Str;
class Type_Info_Union;
class Type_Info;
class Type;
class Iffeature;
class Ext_Instance;
class Schema_Node;
class Schema_Node_Container;
class Schema_Node_Choice;
class Schema_Node_Leaf;
class Schema_Node_Leaflist;
class Schema_Node_List;
class Schema_Node_Anydata;
class Schema_Node_Uses;
class Schema_Node_Grp;
class Schema_Node_Case;
class Schema_Node_Inout;
class Schema_Node_Notif;
class Schema_Node_Action;
class Schema_Node_Augment;
class Substmt;
class Ext;
class Refine_Mod_List;
class Refine_Mod;
class Refine;
class Deviate;
class Deviation;
class Import;
class Include;
class Revision;
class Tpdf;
class Unique;
class Feature;
class Restr;
class When;
class Ident;

class Module
{
public:
    Module(struct lys_module *module, S_Deleter deleter);
    ~Module();
    const char *name() {return module->name;};
    const char *prefix() {return module->prefix;};
    const char *dsc() {return module->dsc;};
    const char *ref() {return module->ref;};
    const char *org() {return module->org;};
    const char *contact() {return module->contact;};
    const char *filepath() {return module->filepath;};
    uint8_t type() {return module->type;};
    uint8_t version() {return module->version;};
    uint8_t deviated() {return module->deviated;};
    uint8_t disabled() {return module->disabled;};
    uint8_t rev_size() {return module->rev_size;};
    uint8_t imp_size() {return module->imp_size;};
    uint8_t inc_size() {return module->inc_size;};
    uint8_t ident_size() {return module->ident_size;};
    uint8_t tpdf_size() {return module->tpdf_size;};
    uint8_t features_size() {return module->features_size;};
    uint8_t augment_size() {return module->augment_size;};
    uint8_t devaiation_size() {return module->deviation_size;};
    uint8_t extensions_size() {return module->extensions_size;};
    uint8_t ext_size() {return module->ext_size;};
    const char *ns() {return module->ns;};
    S_Revision rev();
    std::vector<S_Deviation> *deviation();

    friend class Context;
    friend class Data_Node;

private:
    struct lys_module *module;
    S_Deleter deleter;
};

class Submodule
{
public:
    Submodule(struct lys_submodule *submodule, S_Deleter deleter);
    ~Submodule();
    S_Context ctx() LY_NEW(submodule, ctx, Context);
    const char *name() {return submodule->name;};
    const char *prefix() {return submodule->prefix;};
    const char *dsc() {return submodule->dsc;};
    const char *ref() {return submodule->ref;};
    const char *org() {return submodule->org;};
    const char *contact() {return submodule->contact;};
    const char *filepath() {return submodule->filepath;};
    uint8_t type() {return submodule->type;};
    uint8_t version() {return submodule->version;};
    uint8_t deviated() {return submodule->deviated;};
    uint8_t disabled() {return submodule->disabled;};
    uint8_t implemented() {return submodule->implemented;};
    uint8_t rev_size() {return submodule->rev_size;};
    uint8_t imp_size() {return submodule->imp_size;};
    uint8_t inc_size() {return submodule->inc_size;};
    uint8_t ident_size() {return submodule->ident_size;};
    uint8_t tpdf_size() {return submodule->tpdf_size;};
    uint8_t features_size() {return submodule->features_size;};
    uint8_t augment_size() {return submodule->augment_size;};
    uint8_t devaiation_size() {return submodule->deviation_size;};
    uint8_t extensions_size() {return submodule->extensions_size;};
    uint8_t ext_size() {return submodule->ext_size;};
    S_Revision rev();
    std::vector<S_Deviation> *deviation();

    S_Module belongsto() LY_NEW(submodule, belongsto, Module);

private:
    struct lys_submodule *submodule;
    S_Deleter deleter;
};

class Type_Info_Binary
{
public:
    Type_Info_Binary(struct lys_type_info_binary *info_binary, S_Deleter deleter);
    ~Type_Info_Binary();
    S_Restr length();

private:
    lys_type_info_binary *info_binary;
    S_Deleter deleter;
};

class Type_Bit
{
public:
    Type_Bit(struct lys_type_bit *info_bit, S_Deleter deleter);
    ~Type_Bit();
    const char *name() {return info_bit->name;};
    const char *dsc() {return info_bit->dsc;};
    const char *ref() {return info_bit->ref;};
    uint16_t flags() {return info_bit->flags;};
    uint8_t ext_size() {return info_bit->ext_size;};
    uint8_t iffeature_size() {return info_bit->iffeature_size;};
    uint32_t pos() {return info_bit->pos;};
    std::vector<S_Ext_Instance> *ext();
    std::vector<S_Iffeature> *iffeature();

private:
    lys_type_bit *info_bit;
    S_Deleter deleter;
};

class Type_Info_Bits
{
public:
    Type_Info_Bits(struct lys_type_info_bits *info_bits, S_Deleter deleter);
    ~Type_Info_Bits();
    S_Type_Bit bit();
    int count() {return info_bits->count;};

private:
    lys_type_info_bits *info_bits;
    S_Deleter deleter;
};

class Type_Info_Dec64
{
public:
    Type_Info_Dec64(struct lys_type_info_dec64 *info_dec64, S_Deleter deleter);
    ~Type_Info_Dec64();
    S_Restr range();
    uint8_t dig() {return info_dec64->dig;}
    uint8_t div() {return info_dec64->div;}

private:
    lys_type_info_dec64 *info_dec64;
    S_Deleter deleter;
};

class Type_Enum
{
public:
    Type_Enum(struct lys_type_enum *info_enum, S_Deleter deleter);
    ~Type_Enum();
    const char *name() {return info_enum->name;};
    const char *dsc() {return info_enum->dsc;};
    const char *ref() {return info_enum->ref;};
    uint16_t flags() {return info_enum->flags;};
    uint8_t ext_size() {return info_enum->ext_size;};
    uint8_t iffeature_size() {return info_enum->iffeature_size;};
    int32_t value() {return info_enum->value;};
    std::vector<S_Ext_Instance> *ext();
    std::vector<S_Iffeature> *iffeature();

private:
    lys_type_enum *info_enum;
    S_Deleter deleter;
};

class Type_Info_Enums
{
public:
    Type_Info_Enums(struct lys_type_info_enums *info_enums, S_Deleter deleter);
    ~Type_Info_Enums();
    S_Type_Enum enm();
    int count() {return info_enums->count;};

private:
    lys_type_info_enums *info_enums;
    S_Deleter deleter;
};

class Type_Info_Ident
{
public:
    Type_Info_Ident(struct lys_type_info_ident *info_ident, S_Deleter deleter);
    ~Type_Info_Ident();
    std::vector<S_Ident> *ref();
    int count() {return info_ident->count;};

private:
    lys_type_info_ident *info_ident;
    S_Deleter deleter;
};

class Type_Info_Inst
{
public:
    Type_Info_Inst(struct lys_type_info_inst *info_inst, S_Deleter deleter);
    ~Type_Info_Inst();
    int8_t req() {return info_inst->req;};

private:
    lys_type_info_inst *info_inst;
    S_Deleter deleter;
};

class Type_Info_Num
{
public:
    Type_Info_Num(struct lys_type_info_num *info_num, S_Deleter deleter);
    ~Type_Info_Num();
    S_Restr range();

private:
    lys_type_info_num *info_num;
    S_Deleter deleter;
};

class Type_Info_Lref
{
public:
    Type_Info_Lref(struct lys_type_info_lref *info_lref, S_Deleter deleter);
    ~Type_Info_Lref();
    const char *path() {return info_lref->path;};
    S_Schema_Node_Leaf target();
    int8_t req() {return info_lref->req;};

private:
    lys_type_info_lref *info_lref;
    S_Deleter deleter;
};

class Type_Info_Str
{
public:
    Type_Info_Str(struct lys_type_info_str *info_str, S_Deleter deleter);
    ~Type_Info_Str();
    S_Restr length();
    S_Restr patterns();
    int pat_count() {return info_str->pat_count;};

private:
    lys_type_info_str *info_str;
    S_Deleter deleter;
};

class Type_Info_Union
{
public:
    Type_Info_Union(struct lys_type_info_union *info_union, S_Deleter deleter);
    ~Type_Info_Union();
    std::vector<S_Type> *types();
    int count() {return info_union->count;};
    int has_ptr_type() {return info_union->has_ptr_type;};

private:
    lys_type_info_union *info_union;
    S_Deleter deleter;
};

class Type_Info
{
public:
    Type_Info(union lys_type_info info, LY_DATA_TYPE type, S_Deleter deleter);
    ~Type_Info();
    S_Type_Info_Binary binary();
    S_Type_Info_Bits bits();
    S_Type_Info_Dec64 dec64();
    S_Type_Info_Enums enums();
    S_Type_Info_Ident ident();
    S_Type_Info_Inst inst();
    S_Type_Info_Num num();
    S_Type_Info_Lref lref();
    S_Type_Info_Str str();
    S_Type_Info_Union uni();

private:
    union lys_type_info info;
    LY_DATA_TYPE type;
    S_Deleter deleter;
};

class Type
{
public:
    Type(struct lys_type *type, S_Deleter deleter);
    ~Type();
    const char *module_name() {return type->module_name;};
    LY_DATA_TYPE base() {return type->base;};
    uint8_t ext_size() {return type->ext_size;};
    std::vector<S_Ext_Instance> *ext();
    S_Tpdf der();
    S_Tpdf parent();
    S_Type_Info info();

private:
    struct lys_type *type;
    S_Deleter deleter;
};

class Iffeature {
public:
    Iffeature(struct lys_iffeature *iffeature, S_Deleter deleter);
    ~Iffeature();
    uint8_t *expr() {return iffeature->expr;};
    uint8_t ext_size() {return iffeature->ext_size;};
    std::vector<S_Feature> *features();
    std::vector<S_Ext_Instance> *ext();

private:
    struct lys_iffeature *iffeature;
    S_Deleter deleter;
};

class Ext_Instance
{
public:
    Ext_Instance(lys_ext_instance *ext_instance, S_Deleter deleter);
    ~Ext_Instance();
    //TODO void *parent();
    const char *arg_value() {return ext_instance->arg_value;};
    uint16_t flags() {return ext_instance->flags;};
    uint8_t ext_size() {return ext_instance->ext_size;};
    uint8_t insubstmt_index() {return ext_instance->insubstmt_index;};
    uint8_t insubstmt() {return ext_instance->insubstmt;};
    uint8_t parent_type() {return ext_instance->parent_type;};
    uint8_t ext_type() {return ext_instance->ext_type;};
    std::vector<S_Ext_Instance> *ext();
    void *priv() {return ext_instance->priv;};
    S_Module module() LY_NEW(ext_instance, module, Module);
    LYS_NODE nodetype() {return ext_instance->nodetype;};
private:
    struct lys_ext_instance *ext_instance;
    S_Deleter deleter;
};

class Schema_Node
{
public:
    Schema_Node(lys_node *node, S_Deleter deleter);
    ~Schema_Node();
    const char *name() {return node->name;};
    const char *dsc() {return node->dsc;};
    const char *ref() {return node->ref;};
    uint16_t flags() {return node->flags;};
    uint8_t ext_size() {return node->ext_size;};
    uint8_t iffeature_size() {return node->iffeature_size;};
    std::vector<S_Ext_Instance> *ext();
    std::vector<S_Iffeature> *iffeature() LY_NEW_LIST(node, iffeature, iffeature_size, Iffeature);
    S_Module module();
    LYS_NODE nodetype() {return node->nodetype;};
    virtual S_Schema_Node parent();
    virtual S_Schema_Node child();
    virtual S_Schema_Node next();
    virtual S_Schema_Node prev();
    S_Set find_xpath(const char *path);
    S_Set xpath_atomize(enum lyxp_node_type ctx_node_type, const char *expr, int options);
    S_Set xpath_atomize(int options);
    // void *priv;

    /* emulate TREE macro's */
    std::vector<S_Schema_Node> *tree_for();
    std::vector<S_Schema_Node> *tree_dfs();

    /* SWIG can not access private variables so it needs public getters */
    struct lys_node *swig_node() {return node;};
    S_Deleter swig_deleter() {return deleter;};

    friend Set;
    friend Data_Node;
    friend Context;

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Container : public Schema_Node
{
public:
    Schema_Node_Container(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Container();
    S_When when();
    S_Restr must();
    S_Tpdf ptdf();
    const char *presence() {return ((struct lys_node_container *) node)->presence;};

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Choice : public Schema_Node
{
public:
    Schema_Node_Choice(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Choice();
    S_When when();
    S_Schema_Node dflt();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Leaf : public Schema_Node
{
public:
    Schema_Node_Leaf(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Leaf();
    S_Set backlinks();
    S_When when();
    S_Type type();
    const char *units() {return ((struct lys_node_leaf *)node)->units;};
    const char *dflt() {return ((struct lys_node_leaf *)node)->dflt;};
    S_Schema_Node child() {return nullptr;};

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Leaflist : public Schema_Node
{
public:
    Schema_Node_Leaflist(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Leaflist();
    uint8_t dflt_size() {return ((struct lys_node_leaflist *)node)->dflt_size;};
    uint8_t must_size() {return ((struct lys_node_leaflist *)node)->must_size;};
    S_When when();
    S_Set backlinks();
    std::vector<S_Restr> *must();
    S_Type type();
    const char *units() {return ((struct lys_node_leaflist *)node)->units;};
    std::vector<std::string> *dflt();
    uint32_t min() {return ((struct lys_node_leaflist *)node)->min;};
    uint32_t max() {return ((struct lys_node_leaflist *)node)->max;};
    S_Schema_Node child() {return nullptr;};

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_List : public Schema_Node
{
public:
    Schema_Node_List(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_List();
    uint8_t must_size() {return ((struct lys_node_list *)node)->must_size;};
    uint8_t tpdf_size() {return ((struct lys_node_list *)node)->tpdf_size;};
    uint8_t keys_size() {return ((struct lys_node_list *)node)->keys_size;};
    uint8_t unique_size() {return ((struct lys_node_list *)node)->unique_size;};
    S_When when();
    std::vector<S_Restr> *must();
    std::vector<S_Tpdf> *tpdf();
    std::vector<S_Schema_Node_Leaf> *keys();
    std::vector<S_Unique> *unique();
    uint32_t min() {return ((struct lys_node_list *)node)->min;};
    uint32_t max() {return ((struct lys_node_list *)node)->max;};
    const char *keys_str() {return ((struct lys_node_list *)node)->keys_str;};

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Anydata : public Schema_Node
{
public:
    Schema_Node_Anydata(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Anydata();
    uint8_t must_size() {return ((struct lys_node_list *)node)->must_size;};
    S_When when();
    std::vector<S_Restr> *must();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Uses : public Schema_Node
{
public:
    Schema_Node_Uses(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Uses();
    uint8_t augment_size() {return ((struct lys_node_uses *)node)->augment_size;};
    S_When when();
    std::vector<S_Refine> *refine();
    std::vector<S_Schema_Node_Augment> *augment();
    S_Schema_Node_Grp grp();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Grp : public Schema_Node
{
public:
    Schema_Node_Grp(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Grp();
    uint8_t tpdf_size() {return ((struct lys_node_grp *)node)->tpdf_size;};
    std::vector<S_Tpdf> *tpdf();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Case : public Schema_Node
{
public:
    Schema_Node_Case(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Case();
    S_When when();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Inout : public Schema_Node
{
public:
    Schema_Node_Inout(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Inout();
    uint8_t tpdf_size() {return ((struct lys_node_inout *)node)->tpdf_size;};
    uint8_t must_size() {return ((struct lys_node_inout *)node)->must_size;};
    std::vector<S_Tpdf> *tpdf();
    std::vector<S_Restr> *must();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Notif : public Schema_Node
{
public:
    Schema_Node_Notif(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Notif();
    uint8_t tpdf_size() {return ((struct lys_node_notif *)node)->tpdf_size;};
    uint8_t must_size() {return ((struct lys_node_notif *)node)->must_size;};
    std::vector<S_Tpdf> *tpdf();
    std::vector<S_Restr> *must();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Rpc_Action : public Schema_Node
{
public:
    Schema_Node_Rpc_Action(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Rpc_Action();
    uint8_t tpdf_size() {return ((struct lys_node_rpc_action *)node)->tpdf_size;};
    std::vector<S_Tpdf> *tpdf();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Augment : public Schema_Node
{
public:
    Schema_Node_Augment(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Augment();
    S_When when();
    S_Schema_Node target() LY_NEW_CASTED(lys_node_augment, node, target, Schema_Node);

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Substmt
{
public:
    Substmt(struct lyext_substmt *substmt, S_Deleter deleter);
    ~Substmt();
    LY_STMT stmt() {return substmt->stmt;};
    size_t offest() {return substmt->offset;};
    LY_STMT_CARD cardinality() {return substmt->cardinality;};
private:
    struct lyext_substmt *substmt;
    S_Deleter deleter;
};

class Ext
{
public:
    Ext(struct lys_ext *ext, S_Deleter deleter);
    ~Ext();
    const char *name() {return ext->name;};
    const char *dsc() {return ext->dsc;};
    const char *ref() {return ext->ref;};
    uint16_t flags() {return ext->flags;};
    uint8_t ext_size() {return ext->ext_size;};
    std::vector<S_Ext_Instance> *ext_instance();
    const char *argument() {return ext->argument;};
    S_Module module();
    //struct lyext_plugin *plugin;
private:
    struct lys_ext *ext;
    S_Deleter deleter;
};

class Refine_Mod_List
{
public:
    Refine_Mod_List(struct lys_refine_mod_list *list, S_Deleter deleter);
    ~Refine_Mod_List();
    uint32_t min() {return list->min;};
    uint32_t max() {return list->max;};

private:
    struct lys_refine_mod_list *list;
    S_Deleter deleter;
};

class Refine_Mod
{
public:
    Refine_Mod(union lys_refine_mod mod, uint16_t target_type, S_Deleter deleter);
    ~Refine_Mod();
    const char *presence() {return target_type == LYS_CONTAINER ? mod.presence : nullptr;};
    S_Refine_Mod_List list();

private:
    union lys_refine_mod mod;
    uint16_t target_type;
    S_Deleter deleter;
};

class Refine
{
public:
    Refine(struct lys_refine *refine, S_Deleter deleter);
    ~Refine();
    const char *target_name() {return refine->target_name;};
    const char *dsc() {return refine->dsc;};
    const char *ref() {return refine->ref;};
    uint16_t flags() {return refine->flags;};
    uint8_t ext_size() {return refine->ext_size;};
    uint8_t iffeature_size() {return refine->iffeature_size;};
    uint16_t target_type() {return refine->target_type;};
    uint8_t must_size() {return refine->must_size;};
    uint8_t dflt_size() {return refine->dflt_size;};
    std::vector<S_Ext_Instance> *ext();
    std::vector<S_Iffeature> *iffeature() LY_NEW_LIST(refine, iffeature, iffeature_size, Iffeature);
    S_Module module();
    std::vector<S_Restr> *must();
    std::vector<std::string> *dflt() LY_NEW_STRING_LIST(refine, dflt, dflt_size);
    S_Refine_Mod mod();

private:
    struct lys_refine *refine;
    S_Deleter deleter;
};

class Deviate
{
public:
    Deviate(struct lys_deviate *deviate, S_Deleter deleter);
    ~Deviate();
    LYS_DEVIATE_TYPE mod() {return deviate->mod;};
    uint8_t flags() {return deviate->flags;};
    uint8_t dflt_size() {return deviate->dflt_size;};
    uint8_t ext_size() {return deviate->ext_size;};
    uint8_t min_set() {return deviate->min_set;};
    uint8_t max_set() {return deviate->max_set;};
    uint8_t must_size() {return deviate->must_size;};
    uint8_t unique_size() {return deviate->unique_size;};
    uint32_t min() {return deviate->min;};
    uint32_t max() {return deviate->max;};
    S_Restr must();
    S_Unique unique();
    S_Type type();
    const char *units() {return deviate->units;};
    std::vector<std::string> *dflt() LY_NEW_STRING_LIST(deviate, dflt, dflt_size);
    std::vector<S_Ext_Instance> *ext();

private:
    struct lys_deviate *deviate;
    S_Deleter deleter;
};

class Deviation
{
public:
    Deviation(struct lys_deviation *deviation, S_Deleter deleter);
    ~Deviation();
    const char *target_name() {return deviation->target_name;};
    const char *dsc() {return deviation->dsc;};
    const char *ref() {return deviation->ref;};
    S_Schema_Node orig_node();
    uint8_t deviate_size() {return deviation->deviate_size;};
    uint8_t ext_size() {return deviation->ext_size;};
    std::vector<S_Deviate> *deviate();
    std::vector<S_Ext_Instance> *ext();

private:
    struct lys_deviation *deviation;
    S_Deleter deleter;
};

class Import
{
public:
    Import(struct lys_import *import, S_Deleter deleter);
    ~Import();
    S_Module module() LY_NEW(import, module, Module);
    const char *prefix() {return import->prefix;};
    char *rev() {return &import->rev[0];};
    uint8_t ext_size() {return import->ext_size;};
    std::vector<S_Ext_Instance> *ext() LY_NEW_P_LIST(import, ext, ext_size, Ext_Instance);
    const char *dsc() {return import->dsc;};
    const char *ref() {return import->ref;};

private:
    struct lys_import *import;
    S_Deleter deleter;
};

class Include
{
public:
    Include(struct lys_include *include, S_Deleter deleter);
    ~Include();
    S_Submodule submodule() LY_NEW(include, submodule, Submodule);
    char *rev() {return &include->rev[0];};
    uint8_t ext_size() {return include->ext_size;};
    std::vector<S_Ext_Instance> *ext() LY_NEW_P_LIST(include, ext, ext_size, Ext_Instance);
    const char *dsc() {return include->dsc;};
    const char *ref() {return include->ref;};

private:
    struct lys_include *include;
    S_Deleter deleter;
};

class Revision
{
public:
    Revision(lys_revision *revision, S_Deleter deleter);
    ~Revision();
    char *date() {return &revision->date[0];};
    uint8_t ext_size() {return revision->ext_size;};
    const char *dsc() {return revision->dsc;};
    const char *ref() {return revision->ref;};

private:
    struct lys_revision *revision;
    S_Deleter deleter;
};

class Tpdf
{
public:
    Tpdf(struct lys_tpdf *tpdf, S_Deleter deleter);
    ~Tpdf();
    const char *name() {return tpdf->name;};
    const char *dsc() {return tpdf->dsc;};
    const char *ref() {return tpdf->ref;};
    uint16_t flags() {return tpdf->flags;};
    uint8_t ext_size() {return tpdf->ext_size;};
    uint8_t padding_iffsize() {return tpdf->padding_iffsize;};
    uint8_t has_union_leafref() {return tpdf->has_union_leafref;};
    std::vector<S_Ext_Instance> *ext() LY_NEW_P_LIST(tpdf, ext, ext_size, Ext_Instance);
    const char *units() {return tpdf->units;};
    S_Module module() LY_NEW(tpdf, module, Module);
    S_Type type();
    const char *dflt() {return tpdf->dflt;};

private:
    struct lys_tpdf *tpdf;
    S_Deleter deleter;
};

class Unique
{
public:
    Unique(struct lys_unique *unique, S_Deleter deleter);
    ~Unique();
    std::vector<std::string> *expr() LY_NEW_STRING_LIST(unique, expr, expr_size);
    uint8_t expr_size() {return unique->expr_size;};
    uint8_t trg_type() {return unique->trg_type;};

private:
    struct lys_unique *unique;
    S_Deleter deleter;
};

class Feature
{
public:
    Feature(struct lys_feature *feature, S_Deleter);
    ~Feature();
    const char *name() {return feature->name;};
    const char *dsc() {return feature->dsc;};
    const char *ref() {return feature->ref;};
    uint16_t flags() {return feature->flags;};
    uint8_t ext_size() {return feature->ext_size;};
    uint8_t iffeature_size() {return feature->iffeature_size;};
    std::vector<S_Ext_Instance> *ext() LY_NEW_P_LIST(feature, ext, ext_size, Ext_Instance);
    std::vector<S_Iffeature> *iffeature() LY_NEW_LIST(feature, iffeature, iffeature_size, Iffeature);
    S_Module module() LY_NEW(feature, module, Module);
    S_Set depfeatures() LY_NEW(feature, depfeatures, Set);

private:
    struct lys_feature *feature;
    S_Deleter deleter;
};

class Restr
{
public:
    Restr(struct lys_restr *restr, S_Deleter deleter);
    ~Restr();
    const char *expr() {return restr->expr;};
    const char *dsc() {return restr->dsc;};
    const char *ref() {return restr->ref;};
    const char *eapptag() {return restr->eapptag;};
    const char *emsg() {return restr->emsg;};
    std::vector<S_Ext_Instance> *ext() LY_NEW_P_LIST(restr, ext, ext_size, Ext_Instance);
    uint8_t ext_size() {return restr->ext_size;};

private:
    struct lys_restr *restr;
    S_Deleter deleter;
};

class When
{
public:
    When(struct lys_when *when, S_Deleter deleter = nullptr);
    ~When();
    const char *cond() {return when->cond;};
    const char *dsc() {return when->dsc;};
    const char *ref() {return when->ref;};
    std::vector<S_Ext_Instance> *ext();
    uint8_t ext_size() {return when->ext_size;};

private:
    struct lys_when *when;
    S_Deleter deleter;
};

class Ident
{
public:
    Ident(struct lys_ident *ident, S_Deleter deleter);
    ~Ident();
    const char *name() {return ident->name;};
    const char *dsc() {return ident->dsc;};
    const char *ref() {return ident->ref;};
    uint16_t flags() {return ident->flags;};
    uint8_t ext_size() {return ident->ext_size;};
    uint8_t iffeature_size() {return ident->iffeature_size;};
    uint8_t base_size() {return ident->base_size;};
    std::vector<S_Ext_Instance> *ext() LY_NEW_P_LIST(ident, ext, ext_size, Ext_Instance);
    std::vector<S_Iffeature> *iffeature() LY_NEW_LIST(ident, iffeature, iffeature_size, Iffeature);
    S_Module module() LY_NEW(ident, module, Module);
    std::vector<S_Ident> *base();
    S_Set der() LY_NEW(ident, der, Set);

private:
   struct lys_ident *ident;
    S_Deleter deleter;
};

#endif
