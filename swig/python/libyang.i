%module libyang

%{
    extern "C" {
        #include "libyang.h"
        #include "tree_data.h"
    }
%}

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

%extend Data_Node {
    PyObject *subtype() {
        PyObject *casted = 0;

        auto type = self->swig_node()->schema->nodetype;
        if (LYS_LEAF == type || LYS_LEAFLIST == type) {
            S_Data_Node_Leaf_List *node_leaf_list = NULL;
            node_leaf_list = new S_Data_Node_Leaf_List(new Data_Node_Leaf_List(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node_leaf_list), SWIGTYPE_p_std__shared_ptrT_Data_Node_Leaf_List_t, SWIG_POINTER_OWN);
        } else if (LYS_ANYDATA == type || LYS_ANYXML == type) {
            S_Data_Node_Anydata *node_anydata = NULL;
            node_anydata = new S_Data_Node_Anydata(new Data_Node_Anydata(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node_anydata), SWIGTYPE_p_std__shared_ptrT_Data_Node_Anydata_t, SWIG_POINTER_OWN);
        } else {
            S_Data_Node *node = NULL;
            node = new S_Data_Node(new Data_Node(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_Data_Node_t, SWIG_POINTER_OWN);
        }

        return casted;
    }
};

%extend Schema_Node {
    PyObject *subtype() {
        PyObject *casted = 0;

        auto type = self->swig_node()->nodetype;
        if (LYS_CONTAINER == type) {
            S_Schema_Node_Container *node = NULL;
            node = new S_Schema_Node_Container(new Schema_Node_Container(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_Schema_Node_Container_t, SWIG_POINTER_OWN);
        } else if (LYS_CHOICE == type) {
            S_Schema_Node_Choice *node = NULL;
            node = new S_Schema_Node_Choice(new Schema_Node_Choice(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_Schema_Node_Choice_t, SWIG_POINTER_OWN);
        } else if (LYS_LEAF == type) {
            S_Schema_Node_Leaf *node = NULL;
            node = new S_Schema_Node_Leaf(new Schema_Node_Leaf(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_Schema_Node_Leaf_t, SWIG_POINTER_OWN);
        } else if (LYS_LEAFLIST == type) {
            S_Schema_Node_Leaflist *node = NULL;
            node = new S_Schema_Node_Leaflist(new Schema_Node_Leaflist(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_Schema_Node_Leaflist_t, SWIG_POINTER_OWN);
        } else if (LYS_LIST == type) {
            S_Schema_Node_List *node = NULL;
            node = new S_Schema_Node_List(new Schema_Node_List(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_Schema_Node_List_t, SWIG_POINTER_OWN);
        } else if (LYS_ANYDATA == type || LYS_ANYXML == type) {
            S_Schema_Node_Anydata *node = NULL;
            node = new S_Schema_Node_Anydata(new Schema_Node_Anydata(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_Schema_Node_Anydata_t, SWIG_POINTER_OWN);
        } else if (LYS_USES == type) {
            S_Schema_Node_Uses *node = NULL;
            node = new S_Schema_Node_Uses(new Schema_Node_Uses(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_Schema_Node_Uses_t, SWIG_POINTER_OWN);
        } else if (LYS_GROUPING == type || LYS_RPC == type || LYS_ACTION == type) {
            S_Schema_Node_Grp *node = NULL;
            node = new S_Schema_Node_Grp(new Schema_Node_Grp(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_Schema_Node_Grp_t, SWIG_POINTER_OWN);
        } else if (LYS_CASE == type) {
            S_Schema_Node_Case *node = NULL;
            node = new S_Schema_Node_Case(new Schema_Node_Case(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_Schema_Node_Case_t, SWIG_POINTER_OWN);
        } else if (LYS_INPUT == type || LYS_OUTPUT == type) {
            S_Schema_Node_Inout *node = NULL;
            node = new S_Schema_Node_Inout(new Schema_Node_Inout(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_Schema_Node_Inout_t, SWIG_POINTER_OWN);
        } else if (LYS_NOTIF == type) {
            S_Schema_Node_Notif *node = NULL;
            node = new S_Schema_Node_Notif(new Schema_Node_Notif(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_Schema_Node_Notif_t, SWIG_POINTER_OWN);
        } else {
            S_Schema_Node *node = NULL;
            node = new S_Schema_Node(new Schema_Node(self->swig_node(), self->swig_deleter()));
            casted = SWIG_NewPointerObj(SWIG_as_voidptr(node), SWIGTYPE_p_std__shared_ptrT_Schema_Node_t, SWIG_POINTER_OWN);
        }

        return casted;
    }
};


%include "../swig_base/python_base.i"
