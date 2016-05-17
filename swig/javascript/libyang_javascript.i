%module libyang_javascript

%{

#include "libyang.h"
#include "tree_schema.h"
#include "tree_data.h"
#include "xml.h"
#include "dict.h"


void ly_ctx_destroy(struct ly_ctx *ctx) {
        ly_ctx_destroy(ctx, NULL);
}

%}
%include "typemaps.i"

%feature("immutable","1") name;
%feature("immutable","1") dsc;
%feature("immutable","1") ref;
%feature("immutable","1") cond;
%feature("immutable","1") emsg;
%feature("immutable","1") eapptag;
%feature("immutable","1") expr;
%feature("immutable","1") dflt;
%feature("immutable","1") units;
%feature("immutable","1") prefix;
%feature("immutable","1") target_name;
%feature("immutable","1") presence;
%feature("immutable","1") module_name;
%feature("immutable","1") filepath;
%feature("immutable","1") contact;
%feature("immutable","1") org;
%feature("immutable","1") ns;


%feature("immutable","1") value;
%feature("immutable","1") binary;
%feature("immutable","1") string;
%feature("immutable","1") value_str;
%feature("immutable","1") content;

%feature("immutable","1") path;

%include "libyang.h"
%include "tree_schema.h"
%include "tree_data.h"
%include "xml.h"
%include "dict.h"

void ly_ctx_destroy(struct ly_ctx *ctx);
