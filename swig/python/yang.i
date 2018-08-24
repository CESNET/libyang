%module yang

%{
    extern "C" {
        #include "libyang.h"
        #include "tree_data.h"
    }
%}

%include exception.i
%include <std_except.i>
%catches(std::runtime_error, std::exception, std::string);

%inline %{
#include <unistd.h>
#include "libyang.h"
#include <signal.h>
#include <vector>
#include <memory>

#include "Libyang.hpp"
#include "Tree_Data.hpp"

%}

%extend libyang::Data_Node {
    PyObject *subtype() {
        PyObject *casted = 0;

        auto type = self->swig_node()->schema->nodetype;
        if (LYS_LEAF == type || LYS_LEAFLIST == type) {
            auto node_leaf_list = new std::shared_ptr<libyang::Data_Node_Leaf_List>(new libyang::Data_Node_Leaf_List(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node_leaf_list), SWIGTYPE_p_std__shared_ptrT_libyang__Data_Node_Leaf_List_t, SWIG_POINTER_OWN);
        } else if (LYS_ANYDATA == type || LYS_ANYXML == type) {
            auto node_anydata = new std::shared_ptr<libyang::Data_Node_Anydata>(new libyang::Data_Node_Anydata(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node_anydata), SWIGTYPE_p_std__shared_ptrT_libyang__Data_Node_Anydata_t, SWIG_POINTER_OWN);
        } else {
            auto node = new std::shared_ptr<libyang::Data_Node>(new libyang::Data_Node(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_libyang__Data_Node_t, SWIG_POINTER_OWN);
        }

        return casted;
    }
};

%extend libyang::Schema_Node {
    PyObject *subtype() {
        PyObject *casted = 0;

        auto type = self->swig_node()->nodetype;
        if (LYS_CONTAINER == type) {
            auto node = new std::shared_ptr<libyang::Schema_Node_Container>(new libyang::Schema_Node_Container(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_libyang__Schema_Node_Container_t, SWIG_POINTER_OWN);
        } else if (LYS_CHOICE == type) {
            auto node = new std::shared_ptr<libyang::Schema_Node_Choice>(new libyang::Schema_Node_Choice(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_libyang__Schema_Node_Choice_t, SWIG_POINTER_OWN);
        } else if (LYS_LEAF == type) {
            auto node = new std::shared_ptr<libyang::Schema_Node_Leaf>(new libyang::Schema_Node_Leaf(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_libyang__Schema_Node_Leaf_t, SWIG_POINTER_OWN);
        } else if (LYS_LEAFLIST == type) {
            auto node = new std::shared_ptr<libyang::Schema_Node_Leaflist>(new libyang::Schema_Node_Leaflist(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_libyang__Schema_Node_Leaflist_t, SWIG_POINTER_OWN);
        } else if (LYS_LIST == type) {
            auto node = new std::shared_ptr<libyang::Schema_Node_List>(new libyang::Schema_Node_List(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_libyang__Schema_Node_List_t, SWIG_POINTER_OWN);
        } else if (LYS_ANYDATA == type || LYS_ANYXML == type) {
            auto node = new std::shared_ptr<libyang::Schema_Node_Anydata>(new libyang::Schema_Node_Anydata(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_libyang__Schema_Node_Anydata_t, SWIG_POINTER_OWN);
        } else if (LYS_USES == type) {
            auto node = new std::shared_ptr<libyang::Schema_Node_Uses>(new libyang::Schema_Node_Uses(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_libyang__Schema_Node_Uses_t, SWIG_POINTER_OWN);
        } else if (LYS_GROUPING == type || LYS_RPC == type || LYS_ACTION == type) {
            auto node = new std::shared_ptr<libyang::Schema_Node_Grp>(new libyang::Schema_Node_Grp(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_libyang__Schema_Node_Grp_t, SWIG_POINTER_OWN);
        } else if (LYS_CASE == type) {
            auto node = new std::shared_ptr<libyang::Schema_Node_Case>(new libyang::Schema_Node_Case(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_libyang__Schema_Node_Case_t, SWIG_POINTER_OWN);
        } else if (LYS_INPUT == type || LYS_OUTPUT == type) {
            auto node = new std::shared_ptr<libyang::Schema_Node_Inout>(new libyang::Schema_Node_Inout(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_libyang__Schema_Node_Inout_t, SWIG_POINTER_OWN);
        } else if (LYS_NOTIF == type) {
            auto node = new std::shared_ptr<libyang::Schema_Node_Notif>(new libyang::Schema_Node_Notif(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_libyang__Schema_Node_Notif_t, SWIG_POINTER_OWN);
        } else {
            auto node = new std::shared_ptr<libyang::Schema_Node>(new libyang::Schema_Node(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_libyang__Schema_Node_t, SWIG_POINTER_OWN);
        }

        return casted;
    }
};

%include "../swig_base/python_base.i"
