%module libyang_javascript
%{

#include "libyang.h"
#include "tree_schema.h"
#include "tree_data.h"
#include "xml.h"
#include "dict.h"

%}

%include "libyang.h"
%include "tree_schema.h"
%include "tree_data.h"
%include "xml.h"
%include "dict.h"
