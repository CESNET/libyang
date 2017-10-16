%module libyangEnums

/* %rename("$ignore", "not" %$isenum, "not" %$isconstant, "not" %$isenumitem, regextarget=1, fullname=1) ""; */
%rename("$ignore", "not lyd_node", "not" %$isenum, "not" %$isconstant, "not" %$isenumitem, regextarget=1, fullname=1) "";

%{
#include "./src/libyang.h"
#include "./src/tree_schema.h"
#include "./src/tree_data.h"
#include "./src/extensions.h"
#include "./src/xml.h"
%}

%include "./src/libyang.h"
%include "./src/tree_schema.h"
%include "./src/tree_data.h"
%include "./src/extensions.h"
%include "./src/xml.h"
