/**
 * @file Tree_Schema.cpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Implementation of header Tree_Schema.hpp.
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

#include "Internal.hpp"
#include "Libyang.hpp"
#include "Tree_Schema.hpp"

extern "C" {
#include "../../../src/libyang.h"
#include "../../../src/tree_schema.h"
}

using namespace std;

Module::Module(struct lys_module *module, S_Deleter deleter) {
    _module = module;
    _deleter = deleter;
};
Module::~Module() {};
S_Revision Module::rev() NEW(_module, rev, Revision);
std::vector<S_Deviation> *Module::deviation() NEW_LIST(_module, deviation, deviation_size, Deviation);
Submodule::Submodule(struct lys_submodule *submodule, S_Deleter deleter) {
    _submodule = submodule;
    _deleter = deleter;
};
Submodule::~Submodule() {};
S_Revision Submodule::rev() NEW(_submodule, rev, Revision);
std::vector<S_Deviation> *Submodule::deviation() NEW_LIST(_submodule, deviation, deviation_size, Deviation);

Type_Info_Binary::Type_Info_Binary(struct lys_type_info_binary *info_binary, S_Deleter deleter) {
    _info_binary = info_binary;
    _deleter = deleter;
};
Type_Info_Binary::~Type_Info_Binary() {};
S_Restr Type_Info_Binary::length() {return _info_binary->length ? S_Restr(new Restr(_info_binary->length, _deleter)) : NULL;};

Type_Bit::Type_Bit(struct lys_type_bit *info_bit, S_Deleter deleter) {
    _info_bit = info_bit;
    _deleter = deleter;
};
Type_Bit::~Type_Bit() {};
std::vector<S_Ext_Instance> *Type_Bit::ext() NEW_P_LIST(_info_bit, ext, ext_size, Ext_Instance);
std::vector<S_Iffeature> *Type_Bit::iffeature() NEW_LIST(_info_bit, iffeature, iffeature_size, Iffeature);

Type_Info_Bits::Type_Info_Bits(struct lys_type_info_bits *info_bits, S_Deleter deleter) {
    _info_bits = info_bits;
    _deleter = deleter;
};
Type_Info_Bits::~Type_Info_Bits() {};
S_Type_Bit Type_Info_Bits::bit() {return _info_bits->bit ? S_Type_Bit(new Type_Bit(_info_bits->bit, _deleter)) : NULL;};

Type_Info_Dec64::Type_Info_Dec64(struct lys_type_info_dec64 *info_dec64, S_Deleter deleter) {
    _info_dec64 = info_dec64;
    _deleter = deleter;
};
Type_Info_Dec64::~Type_Info_Dec64() {};
S_Restr Type_Info_Dec64::range() {return _info_dec64->range ? S_Restr(new Restr(_info_dec64->range, _deleter)) : NULL;};

Type_Enum::Type_Enum(struct lys_type_enum *info_enum, S_Deleter deleter) {
    _info_enum = info_enum;
    _deleter = deleter;
};
Type_Enum::~Type_Enum() {};
std::vector<S_Ext_Instance> *Type_Enum::ext() NEW_P_LIST(_info_enum, ext, ext_size, Ext_Instance);
std::vector<S_Iffeature> *Type_Enum::iffeature() NEW_LIST(_info_enum, iffeature, iffeature_size, Iffeature);

Type_Info_Enums::Type_Info_Enums(struct lys_type_info_enums *info_enums, S_Deleter deleter) {
    _info_enums = info_enums;
    _deleter = deleter;
};
Type_Info_Enums::~Type_Info_Enums() {};
S_Type_Enum Type_Info_Enums::enm() {return _info_enums->enm ? S_Type_Enum(new Type_Enum(_info_enums->enm, _deleter)) : NULL;};

Type_Info_Ident::Type_Info_Ident(struct lys_type_info_ident *info_ident, S_Deleter deleter) {
    _info_ident = info_ident;
_deleter = deleter;
};
Type_Info_Ident::~Type_Info_Ident() {};
std::vector<S_Ident> *Type_Info_Ident::ref() NEW_P_LIST(_info_ident, ref, count, Ident);

Type_Info_Inst::Type_Info_Inst(struct lys_type_info_inst *info_inst, S_Deleter deleter) {
    _info_inst = info_inst;
    _deleter = deleter;
};
Type_Info_Inst::~Type_Info_Inst() {};

Type_Info_Num::Type_Info_Num(struct lys_type_info_num *info_num, S_Deleter deleter) {
    _info_num = info_num;
    _deleter = deleter;
};
Type_Info_Num::~Type_Info_Num() {};
S_Restr Type_Info_Num::range() {return _info_num->range ? S_Restr(new Restr(_info_num->range, _deleter)) : NULL;};

Type_Info_Lref::Type_Info_Lref(lys_type_info_lref *info_lref, S_Deleter deleter) {
    _info_lref = info_lref;
    _deleter = deleter;
}
Type_Info_Lref::~Type_Info_Lref() {};
S_Schema_Node_Leaf Type_Info_Lref::target() {return _info_lref->target ? S_Schema_Node_Leaf(new Schema_Node_Leaf((struct lys_node *)_info_lref->target, _deleter)) : NULL;};

Type_Info_Str::Type_Info_Str(lys_type_info_str *info_str, S_Deleter deleter) {
    _info_str = info_str;
    _deleter = deleter;
}
Type_Info_Str::~Type_Info_Str() {};
S_Restr Type_Info_Str::length() {return _info_str->length ? S_Restr(new Restr(_info_str->length, _deleter)) : NULL;};
S_Restr Type_Info_Str::patterns() {return _info_str->patterns ? S_Restr(new Restr(_info_str->patterns, _deleter)) : NULL;};

Type_Info_Union::Type_Info_Union(lys_type_info_union *info_union, S_Deleter deleter) {
    _info_union = info_union;
    _deleter = deleter;
}
Type_Info_Union::~Type_Info_Union() {};
std::vector<S_Type> *Type_Info_Union::types() NEW_LIST(_info_union, types, count, Type);

Type_Info::Type_Info(union lys_type_info info, LY_DATA_TYPE type, S_Deleter deleter) {
    _info = info;
    _type = type;
    _deleter = deleter;
};
Type_Info::~Type_Info() {};
S_Type_Info_Binary Type_Info::binary() {return LY_TYPE_BINARY == _type ? S_Type_Info_Binary(new Type_Info_Binary(&_info.binary, _deleter)) : NULL;};
S_Type_Info_Bits Type_Info::bits() {return LY_TYPE_BITS == _type ? S_Type_Info_Bits(new Type_Info_Bits(&_info.bits, _deleter)) : NULL;};
S_Type_Info_Dec64 Type_Info::dec64() {return LY_TYPE_DEC64 == _type ? S_Type_Info_Dec64(new Type_Info_Dec64(&_info.dec64, _deleter)) : NULL;};
S_Type_Info_Enums Type_Info::enums() {return LY_TYPE_ENUM == _type ? S_Type_Info_Enums(new Type_Info_Enums(&_info.enums, _deleter)) : NULL;};
S_Type_Info_Ident Type_Info::ident() {return LY_TYPE_IDENT == _type ? S_Type_Info_Ident(new Type_Info_Ident(&_info.ident, _deleter)) : NULL;};
S_Type_Info_Inst Type_Info::inst() {return LY_TYPE_INST == _type ? S_Type_Info_Inst(new Type_Info_Inst(&_info.inst, _deleter)) : NULL;};
S_Type_Info_Num Type_Info::num() {
    if (_type >= LY_TYPE_INT8 && _type <= LY_TYPE_UINT64) {
        return S_Type_Info_Num(new Type_Info_Num(&_info.num, _deleter));
    } else {
        return NULL;
    }
};
S_Type_Info_Lref Type_Info::lref() {return LY_TYPE_LEAFREF == _type ? S_Type_Info_Lref(new Type_Info_Lref(&_info.lref, _deleter)) : NULL;};
S_Type_Info_Str Type_Info::str() {return LY_TYPE_STRING == _type ? S_Type_Info_Str(new Type_Info_Str(&_info.str, _deleter)) : NULL;};
S_Type_Info_Union Type_Info::uni() {return LY_TYPE_UNION == _type ? S_Type_Info_Union(new Type_Info_Union(&_info.uni, _deleter)) : NULL;};

Type::Type(struct lys_type *type, S_Deleter deleter) {
    _type = type;
    _deleter = deleter;
};
Type::~Type() {};
std::vector<S_Ext_Instance> *Type::ext() NEW_P_LIST(_type, ext, ext_size, Ext_Instance);
S_Tpdf Type::der() {return _type->der ? S_Tpdf(new Tpdf(_type->der, _deleter)) : NULL;};
S_Tpdf Type::parent() {return _type->parent ? S_Tpdf(new Tpdf(_type->parent, _deleter)) : NULL;};
S_Type_Info Type::info() {return S_Type_Info(new Type_Info(_type->info, _type->base, _deleter));};

Iffeature::Iffeature(struct lys_iffeature *iffeature, S_Deleter deleter) {
    _iffeature = iffeature;
    _deleter = deleter;
};
Iffeature::~Iffeature() {};
std::vector<S_Feature> *Iffeature::features() {
    auto s_vector = new vector<S_Feature>;
    if (NULL == s_vector) {
        return NULL;
    }

    //TODO check if sizeof can be used
    for (size_t i = 0; i < sizeof(*_iffeature->features); i++) {
        s_vector->push_back(S_Feature(new Feature(_iffeature->features[i], _deleter)));
    }

    return s_vector;
};
std::vector<S_Ext_Instance> *Iffeature::ext() NEW_P_LIST(_iffeature, ext, ext_size, Ext_Instance);

Ext_Instance::Ext_Instance(lys_ext_instance *ext_instance, S_Deleter deleter) {
    _ext_instance = ext_instance;
    _deleter = deleter;
};
Ext_Instance::~Ext_Instance() {};
std::vector<S_Ext_Instance> *Ext_Instance::ext() NEW_P_LIST(_ext_instance, ext, ext_size, Ext_Instance);

Revision::Revision(lys_revision *revision, S_Deleter deleter) {
    _revision = revision;
    _deleter = deleter;
};
Revision::~Revision() {};

Schema_Node::Schema_Node(struct lys_node *node, S_Deleter deleter) {
    _node = node;
    _deleter = deleter;
};
Schema_Node::~Schema_Node() {};
std::vector<S_Ext_Instance> *Schema_Node::ext() NEW_P_LIST(_node, ext, ext_size, Ext_Instance);
S_Module Schema_Node::module() NEW(_node, module, Module);
S_Schema_Node Schema_Node::parent() NEW(_node, parent, Schema_Node);
S_Schema_Node Schema_Node::child() NEW(_node, child, Schema_Node);
S_Schema_Node Schema_Node::next() NEW(_node, next, Schema_Node);
S_Schema_Node Schema_Node::prev() NEW(_node, prev, Schema_Node);
S_Set Schema_Node::find_xpath(const char *path) {
    struct ly_set *set = lys_find_path(_node->module, _node, path);
    if (NULL == set) {
        return NULL;
    }

    return S_Set(new Set(set, _deleter));
}

S_Set Schema_Node::xpath_atomize(enum lyxp_node_type ctx_node_type, const char *expr, int options) {
    struct ly_set *set = lys_xpath_atomize(_node, ctx_node_type, expr, options);
    if (NULL == set) {
        return NULL;
    }

    return S_Set(new Set(set, _deleter));
}
S_Set Schema_Node::xpath_atomize(int options) {
    struct ly_set *set = lys_node_xpath_atomize(_node, options);
    if (NULL == set) {
        return NULL;
    }

    return S_Set(new Set(set, _deleter));
}
std::vector<S_Schema_Node> *Schema_Node::tree_for() {
    auto s_vector = new vector<S_Schema_Node>;

    if (NULL == s_vector) {
        return NULL;
    }

    struct lys_node *elem = NULL;
    LY_TREE_FOR(_node, elem) {
        s_vector->push_back(S_Schema_Node(new Schema_Node(elem, _deleter)));
    }

    return s_vector;
}
std::vector<S_Schema_Node> *Schema_Node::tree_dfs() {
    auto s_vector = new vector<S_Schema_Node>;

    if (NULL == s_vector) {
        return NULL;
    }

    struct lys_node *elem = NULL, *next = NULL;
    LY_TREE_DFS_BEGIN(_node, next, elem) {
        s_vector->push_back(S_Schema_Node(new Schema_Node(elem, _deleter)));
        LY_TREE_DFS_END(_node, next, elem)
    }

    return s_vector;
}

Schema_Node_Container::~Schema_Node_Container() {};
S_When Schema_Node_Container::when() NEW_CASTED(lys_node_container, _node, when, When);
S_Restr Schema_Node_Container::must() {
    struct lys_node_container *node = (struct lys_node_container *)_node;
    return node->must ? S_Restr(new Restr(node->must, _deleter)) : NULL;
};
S_Tpdf Schema_Node_Container::ptdf() {
    struct lys_node_container *node = (struct lys_node_container *)_node;
    return node->tpdf ? S_Tpdf(new Tpdf(node->tpdf, _deleter)) : NULL;
};

Schema_Node_Choice::~Schema_Node_Choice() {};
S_When Schema_Node_Choice::when() NEW_CASTED(lys_node_choice, _node, when, When);
S_Schema_Node Schema_Node_Choice::dflt() {
    struct lys_node_choice *node = (struct lys_node_choice *)_node;
    return node->dflt ? S_Schema_Node(new Schema_Node(node->dflt, _deleter)) : NULL;
};

Schema_Node_Leaf::~Schema_Node_Leaf() {};
S_Set Schema_Node_Leaf::backlinks() NEW_CASTED(lys_node_leaf, _node, backlinks, Set);
S_When Schema_Node_Leaf::when() NEW_CASTED(lys_node_leaf, _node, when, When);
S_Type Schema_Node_Leaf::type() {return S_Type(new Type(&((struct lys_node_leaf *)_node)->type, _deleter));}

Schema_Node_Leaflist::~Schema_Node_Leaflist() {};
S_Set Schema_Node_Leaflist::backlinks() NEW_CASTED(lys_node_leaflist, _node, backlinks, Set);
S_When Schema_Node_Leaflist::when() NEW_CASTED(lys_node_leaflist, _node, when, When);
std::vector<S_String> *Schema_Node_Leaflist::dflt() {
    struct lys_node_leaflist *node = (struct lys_node_leaflist *)_node;
    NEW_STRING_LIST(node, dflt, dflt_size);
}
std::vector<S_Restr> *Schema_Node_Leaflist::must() NEW_LIST_CASTED(lys_node_leaflist, _node, must, must_size, Restr);
S_Type Schema_Node_Leaflist::type() {return S_Type(new Type(&((struct lys_node_leaflist *)_node)->type, _deleter));}

Schema_Node_List::~Schema_Node_List() {};
S_When Schema_Node_List::when() NEW_CASTED(lys_node_list, _node, when, When);
std::vector<S_Restr> *Schema_Node_List::must() NEW_LIST_CASTED(lys_node_list, _node, must, must_size, Restr);
std::vector<S_Tpdf> *Schema_Node_List::tpdf() NEW_LIST_CASTED(lys_node_list, _node, tpdf, tpdf_size, Tpdf);
std::vector<S_Schema_Node_Leaf> *Schema_Node_List::keys() {
    auto list = (struct lys_node_list *) _node;

    auto s_vector = new vector<S_Schema_Node_Leaf>;
    if (NULL == s_vector) {
        return NULL;
    }

    for (uint8_t i = 0; i < list->keys_size; i++) {
        s_vector->push_back(S_Schema_Node_Leaf(new Schema_Node_Leaf((struct lys_node *) list->keys[i], _deleter)));
    }

    return s_vector;
}
std::vector<S_Unique> *Schema_Node_List::unique() NEW_LIST_CASTED(lys_node_list, _node, unique, unique_size, Unique);

Schema_Node_Anydata::~Schema_Node_Anydata() {};
S_When Schema_Node_Anydata::when() NEW_CASTED(lys_node_anydata, _node, when, When);
std::vector<S_Restr> *Schema_Node_Anydata::must() NEW_LIST_CASTED(lys_node_anydata, _node, must, must_size, Restr);

Schema_Node_Uses::~Schema_Node_Uses() {};
S_When Schema_Node_Uses::when() NEW_CASTED(lys_node_uses, _node, when, When);
std::vector<S_Refine> *Schema_Node_Uses::refine() NEW_LIST_CASTED(lys_node_uses, _node, refine, refine_size, Refine);
std::vector<S_Schema_Node_Augment> *Schema_Node_Uses::augment() {
    auto uses = (struct lys_node_uses *) _node;

    auto s_vector = new vector<S_Schema_Node_Augment>;
    if (NULL == s_vector) {
        return NULL;
    }

    for (uint8_t i = 0; i < uses->augment_size; i++) {
        s_vector->push_back(S_Schema_Node_Augment(new Schema_Node_Augment((struct lys_node *) &uses->augment[i], _deleter)));
    }

    return s_vector;
}
S_Schema_Node_Grp Schema_Node_Uses::grp() {
    auto uses = (struct lys_node_uses *) _node;
    return uses->grp ? S_Schema_Node_Grp(new Schema_Node_Grp(_node, _deleter)) : NULL;
};

Schema_Node_Grp::~Schema_Node_Grp() {};
std::vector<S_Tpdf> *Schema_Node_Grp::tpdf() NEW_LIST_CASTED(lys_node_grp, _node, tpdf, tpdf_size, Tpdf);

Schema_Node_Case::~Schema_Node_Case() {};
S_When Schema_Node_Case::when() NEW_CASTED(lys_node_case, _node, when, When);

Schema_Node_Inout::~Schema_Node_Inout() {};
std::vector<S_Tpdf> *Schema_Node_Inout::tpdf() NEW_LIST_CASTED(lys_node_inout, _node, tpdf, tpdf_size, Tpdf);
std::vector<S_Restr> *Schema_Node_Inout::must() NEW_LIST_CASTED(lys_node_inout, _node, must, must_size, Restr);

Schema_Node_Notif::~Schema_Node_Notif() {};
std::vector<S_Tpdf> *Schema_Node_Notif::tpdf() NEW_LIST_CASTED(lys_node_notif, _node, tpdf, tpdf_size, Tpdf);
std::vector<S_Restr> *Schema_Node_Notif::must() NEW_LIST_CASTED(lys_node_notif, _node, must, must_size, Restr);

Schema_Node_Rpc_Action::~Schema_Node_Rpc_Action() {};
std::vector<S_Tpdf> *Schema_Node_Rpc_Action::tpdf() NEW_LIST_CASTED(lys_node_rpc_action, _node, tpdf, tpdf_size, Tpdf);

Schema_Node_Augment::~Schema_Node_Augment() {};
S_When Schema_Node_Augment::when() NEW_CASTED(lys_node_augment, _node, when, When);

When::When(struct lys_when *when, S_Deleter deleter) {
    _when = when;
    _deleter = deleter;
}
When::~When() {};
std::vector<S_Ext_Instance> *When::ext() NEW_P_LIST(_when, ext, ext_size, Ext_Instance);

Substmt::Substmt(struct lyext_substmt *substmt, S_Deleter deleter) {
    _substmt = substmt;
    _deleter = deleter;
}
Substmt::~Substmt() {};

Ext::Ext(struct lys_ext *ext, S_Deleter deleter) {
    _ext = ext;
    _deleter = deleter;
}
Ext::~Ext() {};
std::vector<S_Ext_Instance> *Ext::ext() NEW_P_LIST(_ext, ext, ext_size, Ext_Instance);
S_Module Ext::module() NEW(_ext, module, Module);

Refine_Mod_List::Refine_Mod_List(struct lys_refine_mod_list *list, S_Deleter deleter) {
    _list = list;
    _deleter = deleter;
}
Refine_Mod_List::~Refine_Mod_List() {};

Refine_Mod::Refine_Mod(union lys_refine_mod mod, uint16_t target_type, S_Deleter deleter) {
    _mod = mod;;
    _target_type = target_type;
    _deleter = deleter;
}
Refine_Mod::~Refine_Mod() {};
//TODO check which type's to accept
S_Refine_Mod_List Refine_Mod::list() {return _target_type != LYS_CONTAINER ? S_Refine_Mod_List(new Refine_Mod_List(&_mod.list, _deleter)) : NULL;};

Refine::Refine(struct lys_refine *refine, S_Deleter deleter) {
    _refine = refine;
    _deleter = deleter;
}
Refine::~Refine() {};
std::vector<S_Ext_Instance> *Refine::ext() NEW_P_LIST(_refine, ext, ext_size, Ext_Instance);
S_Module Refine::module() NEW(_refine, module, Module);
std::vector<S_Restr> *Refine::must() NEW_LIST(_refine, must, must_size, Restr);
S_Refine_Mod Refine::mod() {S_Refine_Mod(new Refine_Mod(_refine->mod, _refine->target_type, _deleter));};

Deviate::Deviate(struct lys_deviate *deviate, S_Deleter deleter) {
    _deviate = deviate;
    _deleter = deleter;
}
Deviate::~Deviate() {};
std::vector<S_Ext_Instance> *Deviate::ext() NEW_P_LIST(_deviate, ext, ext_size, Ext_Instance);
S_Restr Deviate::must() {return _deviate->must ? S_Restr(new Restr(_deviate->must, _deleter)) : NULL;};
S_Unique Deviate::unique() {return _deviate->unique ? S_Unique(new Unique(_deviate->unique, _deleter)) : NULL;};
S_Type Deviate::type() {return _deviate->type ? S_Type(new Type(_deviate->type, _deleter)) : NULL;}

Deviation::Deviation(struct lys_deviation *deviation, S_Deleter deleter) {
    _deviation = deviation;
    _deleter = deleter;
}
Deviation::~Deviation() {};
S_Schema_Node Deviation::orig_node() NEW(_deviation, orig_node, Schema_Node);
std::vector<S_Deviate> *Deviation::deviate() NEW_LIST(_deviation, deviate, deviate_size, Deviate);
std::vector<S_Ext_Instance> *Deviation::ext() NEW_P_LIST(_deviation, ext, ext_size, Ext_Instance);

Import::Import(struct lys_import *import, S_Deleter deleter) {
    _import = import;
    _deleter = deleter;
}
Import::~Import() {};

Include::Include(struct lys_include *include, S_Deleter deleter) {
    _include = include;
    _deleter = deleter;
}
Include::~Include() {};

Tpdf::Tpdf(struct lys_tpdf *tpdf, S_Deleter deleter) {
    _tpdf = tpdf;
    _deleter = deleter;
}
Tpdf::~Tpdf() {};
S_Type Tpdf::type() {return S_Type(new Type(&_tpdf->type, _deleter));}

Unique::Unique(struct lys_unique *unique, S_Deleter deleter) {
    _unique = unique;
    _deleter = deleter;
}
Unique::~Unique() {};

Feature::Feature(struct lys_feature *feature, S_Deleter deleter) {
    _feature = feature;
    _deleter = deleter;
}
Feature::~Feature() {};

Restr::Restr(struct lys_restr *restr, S_Deleter deleter) {
    _restr = restr;
    _deleter = deleter;
}
Restr::~Restr() {};

Ident::Ident(struct lys_ident *ident, S_Deleter deleter) {
    _ident = ident;
    _deleter = deleter;
}
Ident::~Ident() {};
std::vector<S_Ident> *Ident::base() NEW_P_LIST(_ident, base, base_size, Ident);
