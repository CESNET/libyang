%{
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "context.h"
#include "resolve.h"
#include "common.h"
#include "parser_yang.h"

struct Scheck {
	int check;
	struct Scheck *next;
};
extern int yylineno;
extern int yyleng;
void yyerror(struct lys_module *module, struct unres_schema *unres, struct lys_array_size *size_arrays, int read_all, char *str, ...);   //parameter is in directive parse-param
void yychecked(int value);
int yylex(void);
void free_check();
extern char *yytext;
char *s;
struct Scheck *checked=NULL;
void *actual;
int actual_type;
%}

%parse-param {struct lys_module *module} {struct unres_schema *unres} {struct lys_array_size *size_arrays} {int read_all}

%union {
  int i;
  char *str;
}

%token ANYXML_KEYWORD
%token WHITESPACE
%token ERROR
%token EOL
%token STRING
%token STRINGS
%token IDENTIFIER
%token IDENTIFIERPREFIX
%token REVISION_DATE
%token TAB
%token DOUBLEDOT
%token URI
%token INTEGER
%token NON_NEGATIVE_INTEGER
%token ZERO
%token DECIMAL
%token FRACTION_DIGITS
%token ARGUMENT_KEYWORD
%token AUGMENT_KEYWORD
%token BASE_KEYWORD
%token BELONGS_TO_KEYWORD
%token BIT_KEYWORD
%token CASE_KEYWORD
%token CHOICE_KEYWORD
%token CONFIG_KEYWORD
%token CONTACT_KEYWORD
%token CONTAINER_KEYWORD
%token DEFAULT_KEYWORD
%token DESCRIPTION_KEYWORD
%token ENUM_KEYWORD
%token ERROR_APP_TAG_KEYWORD
%token ERROR_MESSAGE_KEYWORD
%token EXTENSION_KEYWORD
%token DEVIATION_KEYWORD
%token DEVIATE_KEYWORD
%token FEATURE_KEYWORD
%token FRACTION_DIGITS_KEYWORD
%token GROUPING_KEYWORD
%token IDENTITY_KEYWORD
%token IF_FEATURE_KEYWORD
%token IMPORT_KEYWORD
%token INCLUDE_KEYWORD
%token INPUT_KEYWORD
%token KEY_KEYWORD
%token LEAF_KEYWORD
%token LEAF_LIST_KEYWORD
%token LENGTH_KEYWORD
%token LIST_KEYWORD
%token MANDATORY_KEYWORD
%token MAX_ELEMENTS_KEYWORD
%token MIN_ELEMENTS_KEYWORD
%token MODULE_KEYWORD
%token MUST_KEYWORD
%token NAMESPACE_KEYWORD
%token NOTIFICATION_KEYWORD
%token ORDERED_BY_KEYWORD
%token ORGANIZATION_KEYWORD
%token OUTPUT_KEYWORD
%token PATH_KEYWORD
%token PATTERN_KEYWORD
%token POSITION_KEYWORD
%token PREFIX_KEYWORD
%token PRESENCE_KEYWORD
%token RANGE_KEYWORD
%token REFERENCE_KEYWORD
%token REFINE_KEYWORD
%token REQUIRE_INSTANCE_KEYWORD
%token REVISION_KEYWORD
%token REVISION_DATE_KEYWORD
%token RPC_KEYWORD
%token STATUS_KEYWORD
%token SUBMODULE_KEYWORD
%token TYPE_KEYWORD
%token TYPEDEF_KEYWORD
%token UNIQUE_KEYWORD
%token UNITS_KEYWORD
%token USES_KEYWORD
%token VALUE_KEYWORD
%token WHEN_KEYWORD
%token YANG_VERSION_KEYWORD
%token YIN_ELEMENT_KEYWORD
%token ADD_KEYWORD
%token CURRENT_KEYWORD
%token DELETE_KEYWORD
%token DEPRECATED_KEYWORD
%token FALSE_KEYWORD
%token MAX_KEYWORD
%token MIN_KEYWORD
%token NOT_SUPPORTED_KEYWORD
%token OBSOLETE_KEYWORD
%token REPLACE_KEYWORD
%token SYSTEM_KEYWORD
%token TRUE_KEYWORD
%token UNBOUNDED_KEYWORD
%token USER_KEYWORD

%type <i> module_header_stmts
%type <str> tmp_identifier_arg_str
%type <i> status_stmt
%type <i> status_arg_str

%destructor { free($$); } tmp_identifier_arg_str

%%

start: module_stmt 
 |  submodule_stmt 


string_1: STRING  { if (read_all) {
                      s = strdup(yytext);
                      if (!s) {
                        LOGMEM;
                        YYERROR;
                      }
                    }
                  }
optsep string_2


string_2: %empty
  |  string_2 '+' optsep 
     STRING { if (! read_all){
                s = realloc(s,yyleng+strlen(s)+1);
                if (s) {
                  strcat(s,yytext);
                } else {
                  LOGMEM;
                  YYERROR;
                }
              }
            }
     optsep;

start_check: stmtsep {struct Scheck *new; new=malloc(sizeof(struct Scheck)); if (new==NULL) exit(1); new->next=checked; checked=new; 
                      checked->check=0; }

module_stmt: optsep MODULE_KEYWORD sep identifier_arg_str { yang_read_common(module,s,MODULE_KEYWORD,yylineno); s=NULL; }
             '{' stmtsep
                 module_header_stmts { if (read_all && !module->ns) { LOGVAL(LYE_MISSSTMT2,yylineno,"namespace", "module"); YYERROR; }
                                       if (read_all && !module->prefix) { LOGVAL(LYE_MISSSTMT2,yylineno,"prefix", "module"); YYERROR; }
                                     }
                 linkage_stmts
                 meta_stmts
                 revision_stmts
                 body_stmts
             '}' optsep

module_header_stmts: %empty  { $$ = 0; }
  |  module_header_stmts yang_version_stmt { if ($1) { LOGVAL(LYE_TOOMANY, yylineno, "yang version", "module"); YYERROR; } $$ = 1; }
  |  module_header_stmts namespace_stmt { if (read_all && yang_read_common(module,s,NAMESPACE_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  |  module_header_stmts prefix_stmt { if (read_all && yang_read_prefix(module,NULL,s,MODULE_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  ;

submodule_stmt: optsep SUBMODULE_KEYWORD sep identifier_arg_str
                '{' start_check
                    submodule_header_stmts  {  if((checked->check&2)==0) { yyerror(module,unres,size_arrays,read_all,"Belong statement missing."); YYERROR; }
                                               checked->check=0;}
                    linkage_stmts
                    meta_stmts         {free_check();}
                    revision_stmts
                    body_stmts
                '}' optsep

submodule_header_stmts: %empty 
  |  submodule_header_stmts yychecked_1 yang_version_stmt
  |  submodule_header_stmts yychecked_2 belongs_to_stmt
  ;
  
yang_version_stmt: YANG_VERSION_KEYWORD sep yang_version_arg_str stmtend;

yang_version_arg_str: '1' optsep
  | string_1;

namespace_stmt: NAMESPACE_KEYWORD sep uri_str stmtend;

uri_str: string_1
  /*|  URI*/
  ;

linkage_stmts: %empty { if (read_all) {
                          module->imp = calloc(size_arrays->imp, sizeof *module->imp);
                          module->inc = calloc(size_arrays->inc, sizeof *module->inc);
                        }
                      }
 |  linkage_stmts import_stmt 
 |  linkage_stmts include_stmt
 ;

import_stmt: IMPORT_KEYWORD sep tmp_identifier_arg_str {
                 if (!read_all) {
                   size_arrays->imp++;
                 } else {
                   actual = &module->imp[module->imp_size];
                 }
             }
             '{' stmtsep
                 prefix_stmt { if (read_all) {
                                 if (yang_read_prefix(module,actual,s,IMPORT_KEYWORD,yylineno)) {YYERROR;}
                                 s=NULL;
                                 actual_type=IMPORT_KEYWORD;
                               }
                             }
                 revision_date_opt
             '}' stmtsep { if (read_all && yang_fill_import(module,actual,$3,yylineno)) {YYERROR;} }

tmp_identifier_arg_str: identifier_arg_str { $$ = s; s = NULL; }

include_stmt: INCLUDE_KEYWORD sep identifier_arg_str include_end stmtsep;

include_end: ';'
  | '{' stmtsep
     revision_date_opt 
    '}'
  ;  

revision_date_opt: %empty 
  | revision_date_stmt;

revision_date_stmt: REVISION_DATE_KEYWORD sep revision_date optsep stmtend;

revision_date: REVISION_DATE { if (read_all) {
                                 if (actual_type==IMPORT_KEYWORD) {
                                     memcpy(((struct lys_import *)actual)->rev,yytext,LY_REV_SIZE-1);
                                 } else {                              // INCLUDE KEYWORD
                                     memcpy(((struct lys_include *)actual)->rev,yytext,LY_REV_SIZE-1);
                                 }
                               }
                             }

belongs_to_stmt: BELONGS_TO_KEYWORD sep identifier_arg_str
                 '{' stmtsep
                     prefix_stmt 
                 '}' ;

prefix_stmt: PREFIX_KEYWORD sep prefix_arg_str stmtend;

meta_stmts: %empty 
  |  meta_stmts organization_stmt { if (read_all && yang_read_common(module,s,ORGANIZATION_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  |  meta_stmts contact_stmt { if (read_all && yang_read_common(module,s,CONTACT_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  |  meta_stmts description_stmt { if (read_all && yang_read_description(module,NULL,s,MODULE_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  |  meta_stmts reference_stmt { if (read_all && yang_read_reference(module,NULL,s,MODULE_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  ;

organization_stmt: ORGANIZATION_KEYWORD sep string stmtend;

contact_stmt: CONTACT_KEYWORD sep string stmtend;

description_stmt: DESCRIPTION_KEYWORD sep string stmtend;

reference_stmt: REFERENCE_KEYWORD sep string stmtend;

revision_stmts: %empty { if (read_all) {
                           module->rev = calloc(size_arrays->rev, sizeof *module->rev);
                         }
                       }
  | revision_stmts revision_stmt stmtsep;


revision_stmt: REVISION_KEYWORD sep date_arg_str { if (read_all) {
                                                     if(!(actual=yang_read_revision(module,s))) {YYERROR;}
                                                     s=NULL;
                                                   } else {
                                                     size_arrays->rev++;
                                                   }
                                                 }
               revision_end;

revision_end: ';'
  | '{' stmtsep { actual_type = REVISION_KEYWORD; }
        description_reference_stmt
     '}'
  ;

description_reference_stmt: %empty 
  |  description_reference_stmt description_stmt { if (read_all && yang_read_description(module,actual,s,actual_type,yylineno)) {YYERROR;} s=NULL; }
  |  description_reference_stmt reference_stmt { if (read_all && yang_read_reference(module,actual,s,actual_type,yylineno)) {YYERROR;} s=NULL; }
  ;

date_arg_str: REVISION_DATE { if (read_all) {
                                s = strdup(yytext);
                                if (!s) {
                                  LOGMEM;
                                  YYERROR;
                                }
                              }
                            }
              optsep
  | string_1
  ;

body_stmts: %empty { if (read_all) {
                       module->features = calloc(size_arrays->features,sizeof *module->features);
                     }
                   }
  | body_stmts body_stmt stmtsep;


body_stmt: extension_stmt
  | feature_stmt
  | identity_stmt
  | typedef_stmt
  | grouping_stmt
  | data_def_stmt
  | augment_stmt
  | rpc_stmt 
  | notification_stmt 
  | deviation_stmt;

extension_stmt: EXTENSION_KEYWORD sep identifier_arg_str  extension_end;

extension_end: ';' 
  | '{' start_check
        extension_opt_stmt  {free_check();}
    '}'
  ;

extension_opt_stmt: %empty
  |  extension_opt_stmt yychecked_1 argument_stmt
  |  extension_opt_stmt yychecked_2 status_stmt
  |  extension_opt_stmt yychecked_3 description_stmt
  |  extension_opt_stmt yychecked_4 reference_stmt
  ;

argument_stmt: ARGUMENT_KEYWORD sep identifier_arg_str argument_end stmtsep;
                     
argument_end: ';'
  | '{' stmtsep
        yin_element_stmt
    '}'
  ;  

yin_element_stmt: YIN_ELEMENT_KEYWORD sep yin_element_arg_str stmtend;

yin_element_arg_str: TRUE_KEYWORD optsep
  | FALSE_KEYWORD optsep
  | string_1
  ;

status_stmt:  STATUS_KEYWORD sep status_arg_str stmtend { $$ = $3; }

status_arg_str: CURRENT_KEYWORD optsep { $$ = LYS_STATUS_CURR; }
  | OBSOLETE_KEYWORD optsep { $$ = LYS_STATUS_OBSLT; }
  | DEPRECATED_KEYWORD optsep { $$ = LYS_STATUS_DEPRC; }
  | string_1 // not implement
  ;

feature_stmt: FEATURE_KEYWORD sep identifier_arg_str { if (read_all) {
                                                         if (!(actual = yang_read_feature(module,s,yylineno))) {YYERROR;} 
                                                         s=NULL; 
                                                       } else {
                                                         size_arrays->features++;
                                                         if (yang_add_elem(&size_arrays->node, &size_arrays->size)) {
                                                           LOGMEM;
                                                           YYERROR;
                                                         }
                                                       }
                                                     }
              feature_end;

feature_end: ';' 
  | '{' stmtsep
        feature_opt_stmt { if (read_all) { size_arrays->next++; } }
    '}' 
  ;

feature_opt_stmt: %empty { if (read_all) {
                             ((struct lys_feature*)actual)->features = calloc(size_arrays->node[size_arrays->next].if_features, 
                                                                              sizeof *((struct lys_feature*)actual)->features);
                             if (!((struct lys_feature*)actual)->features) {
                               LOGMEM;
                               YYERROR;
                             }
                           } 
                         }
  |  feature_opt_stmt if_feature_stmt { if (read_all) {
                                          if (yang_read_if_feature(module,actual,s,unres,yylineno)) {YYERROR;} 
                                          s=NULL; 
                                        } else {
                                          size_arrays->node[size_arrays->size-1].if_features++;
                                        }
                                      }
  |  feature_opt_stmt status_stmt { if (read_all && yang_read_status(actual,$2,FEATURE_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  |  feature_opt_stmt description_stmt { if (read_all && yang_read_description(module,actual,s,FEATURE_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  |  feature_opt_stmt reference_stmt { if (read_all && yang_read_reference(module,actual,s,FEATURE_KEYWORD,yylineno)) {YYERROR;} s=NULL; }
  ;

if_feature_stmt: IF_FEATURE_KEYWORD sep identifier_ref_arg_str stmtend;

identity_stmt: IDENTITY_KEYWORD sep identifier_arg_str identity_end;

identity_end: ';' 
  |  '{' start_check
         identity_opt_stmt {free_check();}
      '}'
  ;

identity_opt_stmt: %empty 
  |  identity_opt_stmt yychecked_1 base_stmt
  |  identity_opt_stmt yychecked_2 status_stmt
  |  identity_opt_stmt yychecked_3 description_stmt
  |  identity_opt_stmt yychecked_4 reference_stmt
  ;

base_stmt: BASE_KEYWORD sep identifier_ref_arg_str stmtend;

typedef_stmt: TYPEDEF_KEYWORD sep identifier_arg_str 
              '{' start_check
                  type_opt_stmt { if ((checked->check&1)==0) { yyerror(module,unres,size_arrays,read_all,"type statement missing."); YYERROR; }
                                  free_check(); } 
               '}' ;
  
type_stmt: TYPE_KEYWORD sep identifier_ref_arg_str type_end stmtsep;

type_opt_stmt: %empty
  |  type_opt_stmt yychecked_1 type_stmt 
  |  type_opt_stmt yychecked_2 units_stmt
  |  type_opt_stmt yychecked_3 default_stmt
  |  type_opt_stmt yychecked_4 status_stmt
  |  type_opt_stmt yychecked_5 description_stmt
  |  type_opt_stmt yychecked_6 reference_stmt
  ;

type_end: ';' 
  |  '{' start_check
         type_body_stmts /*maybe OK*/  { free_check(); }
      '}'
  ;


type_body_stmts: string_restrictions /* may be finished is OK, but it doesn't test*/
  //| numerical_restrictions
  | decimal64_specification  /*it shuold be semantic control for numerical_restrictions*/
  | enum_specification 
  | path_stmt  /*leafref_specification */
  | base_stmt  /*identityref_specification */
  | require_instance_stmt  /*instance_identifier_specification */
  | bits_specification 
  | union_specification
  ;


string_restrictions: %empty  /*string_restrictions and binary_specification  //ambiguity grammar maybe*/
  |  string_restrictions yychecked_1 length_stmt
  |  string_restrictions pattern_stmt
  ;

//numerical_restrictions: range_stmt stmtsep 

decimal64_specification: fraction_digits_stmt range_stmt_opt;
  |  range_stmt stmtsep fraction_digits_stmt_opt

fraction_digits_stmt_opt: %empty 
  |  fraction_digits_stmt ; 

range_stmt_opt: %empty 
  |  range_stmt stmtsep;

fraction_digits_stmt: FRACTION_DIGITS_KEYWORD sep fraction_digits_arg_str stmtend;

fraction_digits_arg_str: FRACTION_DIGITS optsep
  | string_1
  ;

length_stmt: LENGTH_KEYWORD sep length_arg_str length_end stmtsep;

length_end: ';' 
  |  '{' start_check
         message_opt_stmt  {free_check();}        
      '}'
  ;    

message_opt_stmt: %empty
  |  message_opt_stmt yychecked_1 error_message_stmt
  |  message_opt_stmt yychecked_2 error_app_tag_stmt
  |  message_opt_stmt yychecked_3 description_stmt
  |  message_opt_stmt yychecked_4 reference_stmt
  ;

pattern_stmt: PATTERN_KEYWORD sep string pattern_end stmtsep;

pattern_end: ';'
  |  '{' start_check
         message_opt_stmt  {free_check();}
     '}'
  ;  

enum_specification: enum_stmt enum_stmts;

enum_stmts: %empty
  | enum_stmts enum_stmt;


enum_stmt: ENUM_KEYWORD sep string enum_end stmtsep;

enum_end: ';'
  |  '{' start_check
         enum_opt_stmt  {free_check();}
     '}'
  ;

enum_opt_stmt: %empty 
  |  enum_opt_stmt yychecked_1 value_stmt
  |  enum_opt_stmt yychecked_2 status_stmt
  |  enum_opt_stmt yychecked_3 description_stmt
  |  enum_opt_stmt yychecked_4 reference_stmt
  ;

value_stmt: VALUE_KEYWORD sep integer_value_arg_str stmtend;

integer_value_arg_str: integer_value optsep
  |  string_1
  ;

range_stmt: RANGE_KEYWORD sep range_arg_str range_end; 


range_end: ';'
  |  '{' start_check
         message_opt_stmt  {free_check();}
      '}'
   ;

path_stmt: PATH_KEYWORD sep path_arg_str stmtend; 

require_instance_stmt: REQUIRE_INSTANCE_KEYWORD sep require_instance_arg_str stmtend;

require_instance_arg_str: TRUE_KEYWORD optsep
  |  FALSE_KEYWORD optsep
  |  string_1
  ;

bits_specification: bit_stmt bit_stmts

bit_stmts: %empty 
  | bit_stmts bit_stmt;

bit_stmt: BIT_KEYWORD sep identifier_arg_str bit_end stmtsep;

bit_end: ';'
  |  '{' start_check
         bit_opt_stmt  {free_check();}
     '}'
  ;

bit_opt_stmt: %empty 
  |  bit_opt_stmt yychecked_1 position_stmt
  |  bit_opt_stmt yychecked_2 status_stmt
  |  bit_opt_stmt yychecked_3 description_stmt
  |  bit_opt_stmt yychecked_4 reference_stmt
  ;

position_stmt: POSITION_KEYWORD sep position_value_arg_str stmtend;

position_value_arg_str: non_negative_integer_value optsep
  |  string_1
  ;

union_specification: type_stmt type_stmts

type_stmts: %empty 
  |  type_stmts type_stmt;

error_message_stmt: ERROR_MESSAGE_KEYWORD sep string stmtend;

error_app_tag_stmt: ERROR_APP_TAG_KEYWORD sep string stmtend;

units_stmt: UNITS_KEYWORD sep string stmtend;

default_stmt: DEFAULT_KEYWORD sep string stmtend;

grouping_stmt: GROUPING_KEYWORD sep identifier_arg_str grouping_end;

grouping_end: ';'
  |  '{' start_check
         grouping_opt_stmt  {free_check();}
     '}'
  ;

grouping_opt_stmt: %empty 
  |  grouping_opt_stmt yychecked_1 status_stmt
  |  grouping_opt_stmt yychecked_2 description_stmt
  |  grouping_opt_stmt yychecked_3 reference_stmt
  |  grouping_opt_stmt typedef_grouping_stmt
  |  grouping_opt_stmt data_def_stmt stmtsep
  ;

typedef_grouping_stmt: typedef_stmt stmtsep
  | grouping_stmt stmtsep
  ;


data_def_stmt: container_stmt
  |  leaf_stmt
  |  leaf_list_stmt 
  |  list_stmt 
  |  choice_stmt 
  |  anyxml_stmt
  |  uses_stmt
  ;

container_stmt: CONTAINER_KEYWORD sep identifier_arg_str container_end;

container_end: ';'
  |  '{' start_check
         container_opt_stmt  {free_check();}
      '}'
  ;  

container_opt_stmt: %empty 
  |  container_opt_stmt yychecked_1 when_stmt
  |  container_opt_stmt if_feature_stmt
  |  container_opt_stmt must_stmt
  |  container_opt_stmt yychecked_2 presence_stmt
  |  container_opt_stmt yychecked_3 config_stmt
  |  container_opt_stmt yychecked_4 status_stmt
  |  container_opt_stmt yychecked_5 description_stmt
  |  container_opt_stmt yychecked_6 reference_stmt
  |  container_opt_stmt typedef_grouping_stmt
  |  container_opt_stmt data_def_stmt stmtsep
  ;

leaf_stmt: LEAF_KEYWORD sep identifier_arg_str 
           '{' start_check
               leaf_opt_stmt  { if ((checked->check&2)==0) { yyerror(module,unres,size_arrays,read_all,"type statement missingo."); YYERROR; }
                                free_check(); }
           '}' ;

leaf_opt_stmt: %empty 
  |  leaf_opt_stmt yychecked_1 when_stmt
  |  leaf_opt_stmt if_feature_stmt
  |  leaf_opt_stmt yychecked_2 type_stmt
  |  leaf_opt_stmt yychecked_3 units_stmt
  |  leaf_opt_stmt must_stmt
  |  leaf_opt_stmt yychecked_4 default_stmt
  |  leaf_opt_stmt yychecked_5 config_stmt
  |  leaf_opt_stmt yychecked_6 mandatory_stmt
  |  leaf_opt_stmt yychecked_7 status_stmt
  |  leaf_opt_stmt yychecked_8 description_stmt
  |  leaf_opt_stmt yychecked_9 reference_stmt
  ;

leaf_list_stmt: LEAF_LIST_KEYWORD sep identifier_arg_str 
                '{' start_check
                    leaf_list_opt_stmt { if ((checked->check&2)==0) { yyerror(module,unres,size_arrays,read_all,"type statement missing."); YYERROR; }
                                         free_check(); }
                '}' ;

leaf_list_opt_stmt: %empty 
  |  leaf_list_opt_stmt yychecked_1 when_stmt
  |  leaf_list_opt_stmt if_feature_stmt
  |  leaf_list_opt_stmt yychecked_2 type_stmt
  |  leaf_list_opt_stmt yychecked_3 units_stmt
  |  leaf_list_opt_stmt must_stmt
  |  leaf_list_opt_stmt yychecked_4 config_stmt
  |  leaf_list_opt_stmt yychecked_5 min_elements_stmt
  |  leaf_list_opt_stmt yychecked_6 max_elements_stmt
  |  leaf_list_opt_stmt yychecked_7 ordered_by_stmt
  |  leaf_list_opt_stmt yychecked_8 status_stmt
  |  leaf_list_opt_stmt yychecked_9 description_stmt
  |  leaf_list_opt_stmt yychecked_10 reference_stmt
  ;

list_stmt: LIST_KEYWORD sep identifier_arg_str 
           '{' start_check
               list_opt_stmt { if ((checked->check&1024)==0) { yyerror(module,unres,size_arrays,read_all,"data-def statement missing."); YYERROR; }
                               free_check(); }
            '}' ;

list_opt_stmt: %empty 
  |  list_opt_stmt yychecked_1 when_stmt
  |  list_opt_stmt if_feature_stmt
  |  list_opt_stmt must_stmt
  |  list_opt_stmt yychecked_2 key_stmt
  |  list_opt_stmt unique_stmt
  |  list_opt_stmt yychecked_3 config_stmt
  |  list_opt_stmt yychecked_4 min_elements_stmt
  |  list_opt_stmt yychecked_5 max_elements_stmt
  |  list_opt_stmt yychecked_6 ordered_by_stmt
  |  list_opt_stmt yychecked_7 status_stmt
  |  list_opt_stmt yychecked_8 description_stmt
  |  list_opt_stmt yychecked_9 reference_stmt
  |  list_opt_stmt typedef_grouping_stmt
  |  list_opt_stmt yychecked data_def_stmt stmtsep
  ;

choice_stmt: CHOICE_KEYWORD sep identifier_arg_str choice_end;

choice_end: ';'
  |  '{' start_check
         choice_opt_stmt  {free_check(); }
     '}' ;

choice_opt_stmt: %empty 
  |  choice_opt_stmt yychecked_1 when_stmt 
  |  choice_opt_stmt if_feature_stmt
  |  choice_opt_stmt yychecked_2 default_stmt
  |  choice_opt_stmt yychecked_3 config_stmt
  |  choice_opt_stmt yychecked_4 mandatory_stmt
  |  choice_opt_stmt yychecked_5 status_stmt
  |  choice_opt_stmt yychecked_6 description_stmt
  |  choice_opt_stmt yychecked_7 reference_stmt
  |  choice_opt_stmt  short_case_case_stmt
  ;

short_case_case_stmt:  short_case_stmt stmtsep
  |  case_stmt stmtsep
  ;

short_case_stmt: container_stmt
  |  leaf_stmt
  |  leaf_list_stmt
  |  list_stmt
  |  anyxml_stmt
  ;

case_stmt: CASE_KEYWORD sep identifier_arg_str case_end;

case_end: ';'
  |  '{' start_check
         case_opt_stmt  {free_check();}
      '}' ;

case_opt_stmt: %empty 
  |  case_opt_stmt yychecked_1 when_stmt
  |  case_opt_stmt if_feature_stmt
  |  case_opt_stmt yychecked_2 status_stmt
  |  case_opt_stmt yychecked_3 description_stmt
  |  case_opt_stmt yychecked_4 reference_stmt
  |  case_opt_stmt data_def_stmt stmtsep
  ;

anyxml_stmt: ANYXML_KEYWORD sep identifier_arg_str anyxml_end;

anyxml_end: ';'
  |  '{' start_check
         anyxml_opt_stmt  {free_check();}
     '}' ;

anyxml_opt_stmt: %empty 
  |  anyxml_opt_stmt yychecked_1 when_stmt
  |  anyxml_opt_stmt if_feature_stmt
  |  anyxml_opt_stmt must_stmt
  |  anyxml_opt_stmt yychecked_2 config_stmt
  |  anyxml_opt_stmt yychecked_3 mandatory_stmt
  |  anyxml_opt_stmt yychecked_4 status_stmt
  |  anyxml_opt_stmt yychecked_5 description_stmt
  |  anyxml_opt_stmt yychecked_6 reference_stmt
  ;

uses_stmt: USES_KEYWORD sep identifier_ref_arg_str uses_end;

uses_end: ';'
  |  '{' start_check
         uses_opt_stmt  {free_check();}
     '}' ;

uses_opt_stmt: %empty 
  |  uses_opt_stmt yychecked_1 when_stmt
  |  uses_opt_stmt if_feature_stmt
  |  uses_opt_stmt yychecked_2 status_stmt
  |  uses_opt_stmt yychecked_3 description_stmt
  |  uses_opt_stmt yychecked_4 reference_stmt
  |  uses_opt_stmt refine_stmt stmtsep
  |  uses_opt_stmt uses_augment_stmt stmtsep
  ;

refine_stmt: REFINE_KEYWORD sep refine_arg_str refine_end;

refine_end: ';'
  |  '{' start_check
         refine_body_opt_stmts {free_check();}
     '}' ;


refine_arg_str: descendant_schema_nodeid optsep 
  | string_1
  ;

refine_body_opt_stmts: %empty 
  |  refine_body_opt_stmts must_stmt 
  |  refine_body_opt_stmts yychecked_1 presence_stmt  
  |  refine_body_opt_stmts yychecked_2 default_stmt 
  |  refine_body_opt_stmts yychecked_3 config_stmt 
  |  refine_body_opt_stmts yychecked_4 mandatory_stmt 
  |  refine_body_opt_stmts yychecked_5 min_elements_stmt 
  |  refine_body_opt_stmts yychecked_6 max_elements_stmt 
  |  refine_body_opt_stmts yychecked_7 description_stmt 
  |  refine_body_opt_stmts yychecked_8 reference_stmt
  ;

/*original grammar has rules for every node, but I have 20 reduce conflicts. I think it should be checked semantic analysis*/
/*
refine_stmt1: presence_stmt
  |  default_stmt
  ;
/*
refine_stmt2: mandatory_stmt
  |  min_elements_stmt max_elements_stmt
  ;
*/

uses_augment_stmt: AUGMENT_KEYWORD sep uses_augment_arg_str 
                   '{' start_check
                       augment_opt_stmt { if ((checked->check&1024)==0) { yyerror(module,unres,size_arrays,read_all,"data-def or case statement missing."); YYERROR; }
                                     free_check(); }
                   '}' ;

data_def_case_stmt: data_def_stmt stmtsep
  | case_stmt stmtsep
  ;

uses_augment_arg_str: descendant_schema_nodeid optsep 
  |  string_1 
  ; 

augment_stmt: AUGMENT_KEYWORD sep augment_arg_str 
              '{' start_check
                  augment_opt_stmt { if ((checked->check&1024)==0) { yyerror(module,unres,size_arrays,read_all,"data-def or case statement missing."); YYERROR; }
                                     free_check(); }
               '}' ;

augment_opt_stmt: %empty 
  |  augment_opt_stmt yychecked_1 when_stmt
  |  augment_opt_stmt if_feature_stmt
  |  augment_opt_stmt yychecked_2 status_stmt
  |  augment_opt_stmt yychecked_3 description_stmt
  |  augment_opt_stmt yychecked_4 reference_stmt
  |  augment_opt_stmt yychecked data_def_case_stmt
  ;

augment_arg_str: absolute_schema_nodeids optsep
  |  string_1
  ; 

rpc_stmt: RPC_KEYWORD sep identifier_arg_str rpc_end;

rpc_end: ';'
  |  '{' start_check
         rpc_opt_stmt  {free_check();}
      '}' ;

rpc_opt_stmt: %empty 
  |  rpc_opt_stmt if_feature_stmt
  |  rpc_opt_stmt yychecked_1 status_stmt
  |  rpc_opt_stmt yychecked_2 description_stmt
  |  rpc_opt_stmt yychecked_3 reference_stmt
  |  rpc_opt_stmt typedef_grouping_stmt
  |  rpc_opt_stmt yychecked_4 input_stmt
  |  rpc_opt_stmt yychecked_5 output_stmt
  ;

input_stmt: INPUT_KEYWORD optsep
            '{' start_check
                input_output_opt_stmt  { if ((checked->check&1024)==0) { yyerror(module,unres,size_arrays,read_all,"data-def or case statement missing."); YYERROR; }
                                         free_check(); }
            '}' stmtsep;

input_output_opt_stmt: %empty 
  |  input_output_opt_stmt typedef_grouping_stmt
  |  input_output_opt_stmt yychecked data_def_stmt stmtsep
  ;

output_stmt: OUTPUT_KEYWORD optsep
                     '{' start_check
                         input_output_opt_stmt  { if ((checked->check&1024)==0) { yyerror(module,unres,size_arrays,read_all,"data-def or case statement missing."); YYERROR; }
                                                  free_check(); }
                     '}' stmtsep;

notification_stmt: NOTIFICATION_KEYWORD sep identifier_arg_str notification_end;

notification_end: ';'
  |  '{' start_check
         notification_opt_stmt  {free_check();}
      '}' ;

notification_opt_stmt: %empty 
  |  notification_opt_stmt if_feature_stmt 
  |  notification_opt_stmt yychecked_1 status_stmt
  |  notification_opt_stmt yychecked_2 description_stmt
  |  notification_opt_stmt yychecked_3 reference_stmt
  |  notification_opt_stmt typedef_grouping_stmt
  |  notification_opt_stmt data_def_stmt stmtsep
  ;

deviation_stmt: DEVIATION_KEYWORD sep deviation_arg_str 
                '{' start_check
                    deviation_opt_stmt  {  if ((checked->check&4)==0) { yyerror(module,unres,size_arrays,read_all,"type statement missingo."); YYERROR; }
                                           free_check(); }
                '}' ;

deviation_opt_stmt: %empty 
  |  deviation_opt_stmt yychecked_1 description_stmt
  |  deviation_opt_stmt yychecked_2 reference_stmt
  |  deviation_opt_stmt yychecked_3 DEVIATE_KEYWORD sep deviate_body_stmt
  ;

deviation_arg_str: absolute_schema_nodeids optsep
  | string_1
; 

deviate_body_stmt: deviate_not_supported_stmt
  |  deviate_stmt_opt deviate_stmts;


deviate_stmt_opt: %empty 
  |  deviate_stmt_opt DEVIATE_KEYWORD sep deviate_stmts DEVIATE_KEYWORD sep;

deviate_stmts: deviate_add_stmt
  |  deviate_replace_stmt
  |  deviate_delete_stmt
  ;

deviate_not_supported_stmt: NOT_SUPPORTED_KEYWORD optsep stmtend;

deviate_add_stmt: ADD_KEYWORD optsep deviate_add_end;

deviate_add_end: ';'
  |  '{' start_check
         deviate_add_opt_stmt  {free_check();}
     '}' ;

deviate_add_opt_stmt: %empty 
  |  deviate_add_opt_stmt yychecked_1 units_stmt
  |  deviate_add_opt_stmt must_stmt
  |  deviate_add_opt_stmt unique_stmt
  |  deviate_add_opt_stmt yychecked_2 default_stmt
  |  deviate_add_opt_stmt yychecked_3 config_stmt
  |  deviate_add_opt_stmt yychecked_4 mandatory_stmt
  |  deviate_add_opt_stmt yychecked_5 min_elements_stmt
  |  deviate_add_opt_stmt yychecked_6 max_elements_stmt
  ;

deviate_delete_stmt: DELETE_KEYWORD optsep deviate_delete_end;

deviate_delete_end: ';'
  |  '{' start_check
         deviate_delete_opt_stmt  {free_check();}
      '}' ;

deviate_delete_opt_stmt: %empty 
  |  deviate_delete_opt_stmt yychecked_1 units_stmt
  |  deviate_delete_opt_stmt must_stmt
  |  deviate_delete_opt_stmt unique_stmt
  |  deviate_delete_opt_stmt yychecked_2 default_stmt
  ;

deviate_replace_stmt: REPLACE_KEYWORD optsep deviate_replace_end;

deviate_replace_end: ';'
  |  '{' start_check
         deviate_replace_opt_stmt {free_check();}
     '}' ;

deviate_replace_opt_stmt: %empty 
  |  deviate_replace_opt_stmt yychecked_1 type_stmt
  |  deviate_replace_opt_stmt yychecked_2 units_stmt
  |  deviate_replace_opt_stmt yychecked_3 default_stmt
  |  deviate_replace_opt_stmt yychecked_4 config_stmt
  |  deviate_replace_opt_stmt yychecked_5 mandatory_stmt
  |  deviate_replace_opt_stmt yychecked_6 min_elements_stmt
  |  deviate_replace_opt_stmt yychecked_7 max_elements_stmt
  ;

when_stmt: WHEN_KEYWORD sep string when_end stmtsep;

when_end: ';'
  |  '{' start_check
         description_reference_stmt  {free_check();}
     '}'
  ;

config_stmt: CONFIG_KEYWORD sep config_arg_str stmtend;

config_arg_str: TRUE_KEYWORD optsep
  |  FALSE_KEYWORD optsep
  |  string_1
  ;

mandatory_stmt: MANDATORY_KEYWORD sep mandatory_arg_str stmtend;

mandatory_arg_str: TRUE_KEYWORD optsep
  |  FALSE_KEYWORD optsep
  |  string_1
  ;

presence_stmt: PRESENCE_KEYWORD sep string stmtend;

min_elements_stmt: MIN_ELEMENTS_KEYWORD sep min_value_arg_str stmtend;

min_value_arg_str: non_negative_integer_value optsep
  |  string_1 
  ;

max_elements_stmt: MAX_ELEMENTS_KEYWORD sep max_value_arg_str stmtend;

max_value_arg_str: UNBOUNDED_KEYWORD optsep
  |  NON_NEGATIVE_INTEGER optsep 
  |  string_1
  ;

ordered_by_stmt: ORDERED_BY_KEYWORD sep ordered_by_arg_str stmtend;

ordered_by_arg_str: USER_KEYWORD optsep
  |  SYSTEM_KEYWORD optsep
  |  string_1
  ;

must_stmt: MUST_KEYWORD sep string must_end stmtsep;

must_end: ';'
  |  '{' start_check
         message_opt_stmt  {free_check();}
     '}'
  ;

unique_stmt: UNIQUE_KEYWORD sep unique_arg_str; 

unique_arg_str: descendant_schema_nodeid unique_arg 
  |  string_1 stmtend;

unique_arg: sep descendant_schema_nodeid unique_arg
  |  stmtend;

key_stmt: KEY_KEYWORD sep key_arg_str; 

key_arg_str: node_identifier key_opt
  |  string_1 stmtend
  ;

key_opt: sep node_identifier key_opt
  | stmtend 
  ;

range_arg_str: range_part1 range_part_opt;
  |  string_1
  ;

range_part_opt: %empty 
  | range_part_opt '|' optsep range_part1 ;

range_part1: range_boundary range_part2;

range_part2: %empty 
  | DOUBLEDOT optsep range_boundary;

range_boundary: MIN_KEYWORD optsep
  | MAX_KEYWORD optsep
  | integer_value optsep
  | DECIMAL optsep
  ;

length_arg_str: length_part1 length_part_opt;
  |  string_1
  ;

length_part_opt: %empty 
  | length_part_opt '|' optsep length_part1;

length_part1: length_boundary length_part2;

length_part2: %empty 
  |  DOUBLEDOT optsep length_boundary;

length_boundary: MIN_KEYWORD optsep
  |  MAX_KEYWORD optsep
  |  non_negative_integer_value optsep
  ;

absolute_schema_nodeid: "/" node_identifier;

absolute_schema_nodeids: absolute_schema_nodeid absolute_schema_nodeid_opt;

absolute_schema_nodeid_opt: %empty 
  |  absolute_schema_nodeid_opt absolute_schema_nodeid
  ;

descendant_schema_nodeid: node_identifier absolute_schema_nodeid_opt;

path_arg_str: absolute_paths
  |  relative_path
  |  string_1
  ;

absolute_path: "/" node_identifier path_predicate

absolute_paths: absolute_path absolute_path_opt

absolute_path_opt: %empty 
  |  absolute_path_opt absolute_path;

relative_path: relative_path_part1 relative_path_part1_opt descendant_path

relative_path_part1: ".." "/" ;

relative_path_part1_opt: %empty 
  |  relative_path_part1_opt relative_path_part1;

descendant_path: node_identifier descendant_path_opt

descendant_path_opt: %empty 
  |  path_predicate absolute_paths;

path_predicate: %empty 
  | path_predicate "[" whitespace_opt path_equality_expr whitespace_opt "]"

path_equality_expr: node_identifier whitespace_opt "=" whitespace_opt path_key_expr

path_key_expr: current_function_invocation whitespace_opt "/" whitespace_opt
                     rel_path_keyexpr

rel_path_keyexpr: rel_path_keyexpr_part1 rel_path_keyexpr_part1_opt
                    node_identifier rel_path_keyexpr_part2
                     node_identifier

rel_path_keyexpr_part1: ".." whitespace_opt "/" whitespace_opt;

rel_path_keyexpr_part1_opt: %empty 
  |  rel_path_keyexpr_part1_opt rel_path_keyexpr_part1;

rel_path_keyexpr_part2: %empty 
  | rel_path_keyexpr_part2 whitespace_opt "/" whitespace_opt node_identifier;

current_function_invocation: CURRENT_KEYWORD whitespace_opt "(" whitespace_opt ")"

non_negative_integer_value: ZERO
  |  NON_NEGATIVE_INTEGER
  ;

integer_value: ZERO
  |  INTEGER
  |  NON_NEGATIVE_INTEGER
  ;

prefix_arg_str: string_1
  |  identifiers optsep;

identifier_arg_str: identifiers optsep 
  |  string_1 
  ;

node_identifier: identifiers 
  | IDENTIFIERPREFIX 
  ; 

identifier_ref_arg_str: identifiers optsep
  | IDENTIFIERPREFIX optsep
  | string_1
  ; 

stmtend: ';' stmtsep
  | '{' stmtsep '}' stmtsep
  ;

stmtsep: %empty 
  | stmtsep sep_stmt 
  | stmtsep unknown_statement
  ;

unknown_statement: IDENTIFIERPREFIX string_opt unknown_statement_end 

string_opt: %empty //Soptsep
  |  sep string_1;

unknown_statement_end: ';' 
  |  '{' optsep unknown_statement2_opt '}'

unknown_statement2_opt: %empty 
  |  node_identifier string_opt unknown_statement2_end;

unknown_statement2_end: ';' optsep
  |  '{' optsep unknown_statement2_opt '}' optsep

sep_stmt: WHITESPACE
  | EOL
  ;

optsep: %empty 
  | optsep sep_stmt
  ; 

sep: sep_stmt optsep;

whitespace_opt: %empty 
  | WHITESPACE
  ;


string: STRINGS optsep
  | REVISION_DATE optsep
  | identifiers optsep
  | string_1
  ;

identifier: IDENTIFIER
  |  ANYXML_KEYWORD
  |  ARGUMENT_KEYWORD
  |  AUGMENT_KEYWORD
  |  BASE_KEYWORD
  |  BELONGS_TO_KEYWORD
  |  BIT_KEYWORD
  |  CASE_KEYWORD
  |  CHOICE_KEYWORD
  |  CONFIG_KEYWORD
  |  CONTACT_KEYWORD
  |  CONTAINER_KEYWORD
  |  DEFAULT_KEYWORD
  |  DESCRIPTION_KEYWORD
  |  ENUM_KEYWORD
  |  ERROR_APP_TAG_KEYWORD
  |  ERROR_MESSAGE_KEYWORD
  |  EXTENSION_KEYWORD
  |  DEVIATION_KEYWORD
  |  DEVIATE_KEYWORD
  |  FEATURE_KEYWORD
  |  FRACTION_DIGITS_KEYWORD
  |  GROUPING_KEYWORD
  |  IDENTITY_KEYWORD
  |  IF_FEATURE_KEYWORD
  |  IMPORT_KEYWORD
  |  INCLUDE_KEYWORD
  |  INPUT_KEYWORD
  |  KEY_KEYWORD
  |  LEAF_KEYWORD
  |  LEAF_LIST_KEYWORD
  |  LENGTH_KEYWORD
  |  LIST_KEYWORD
  |  MANDATORY_KEYWORD
  |  MAX_ELEMENTS_KEYWORD
  |  MIN_ELEMENTS_KEYWORD
  |  MODULE_KEYWORD
  |  MUST_KEYWORD
  |  NAMESPACE_KEYWORD
  |  NOTIFICATION_KEYWORD
  |  ORDERED_BY_KEYWORD
  |  ORGANIZATION_KEYWORD
  |  OUTPUT_KEYWORD
  |  PATH_KEYWORD
  |  PATTERN_KEYWORD
  |  POSITION_KEYWORD
  |  PREFIX_KEYWORD
  |  PRESENCE_KEYWORD
  |  RANGE_KEYWORD
  |  REFERENCE_KEYWORD
  |  REFINE_KEYWORD
  |  REQUIRE_INSTANCE_KEYWORD
  |  REVISION_KEYWORD
  |  REVISION_DATE_KEYWORD
  |  RPC_KEYWORD
  |  STATUS_KEYWORD
  |  SUBMODULE_KEYWORD
  |  TYPE_KEYWORD
  |  TYPEDEF_KEYWORD
  |  UNIQUE_KEYWORD
  |  UNITS_KEYWORD
  |  USES_KEYWORD
  |  VALUE_KEYWORD
  |  WHEN_KEYWORD
  |  YANG_VERSION_KEYWORD
  |  YIN_ELEMENT_KEYWORD
  |  ADD_KEYWORD
  |  CURRENT_KEYWORD
  |  DELETE_KEYWORD
  |  DEPRECATED_KEYWORD
  |  FALSE_KEYWORD
  |  MAX_KEYWORD
  |  MIN_KEYWORD
  |  NOT_SUPPORTED_KEYWORD
  |  OBSOLETE_KEYWORD
  |  REPLACE_KEYWORD
  |  SYSTEM_KEYWORD
  |  TRUE_KEYWORD
  |  UNBOUNDED_KEYWORD
  |  USER_KEYWORD
  ;

yychecked: %empty {checked->check|=1024;}
yychecked_1: %empty { if ((checked->check&1)==0) checked->check|=1; else { yyerror(module,unres,size_arrays,read_all,"syntax error!"); YYERROR; }	}
yychecked_2: %empty { if ((checked->check&2)==0) checked->check|=2; else { yyerror(module,unres,size_arrays,read_all,"syntax error!"); YYERROR; }	}
yychecked_3: %empty { if ((checked->check&4)==0) checked->check|=4; else { yyerror(module,unres,size_arrays,read_all,"syntax error!"); YYERROR; }	}
yychecked_4: %empty { if ((checked->check&8)==0) checked->check|=8; else { yyerror(module,unres,size_arrays,read_all,"syntax error!"); YYERROR; }	}
yychecked_5: %empty { if ((checked->check&16)==0) checked->check|=16; else { yyerror(module,unres,size_arrays,read_all,"syntax error!"); YYERROR; }	}
yychecked_6: %empty { if ((checked->check&32)==0) checked->check|=32; else { yyerror(module,unres,size_arrays,read_all,"syntax error!"); YYERROR; }	}
yychecked_7: %empty { if ((checked->check&64)==0) checked->check|=64; else { yyerror(module,unres,size_arrays,read_all,"syntax error!"); YYERROR; }	}
yychecked_8: %empty { if ((checked->check&128)==0) checked->check|=128; else { yyerror(module,unres,size_arrays,read_all,"syntax error!"); YYERROR; }	}
yychecked_9: %empty { if ((checked->check&256)==0) checked->check|=256; else { yyerror(module,unres,size_arrays,read_all,"syntax error!"); YYERROR; }	}
yychecked_10: %empty { if ((checked->check&512)==0) checked->check|=512; else { yyerror(module,unres,size_arrays,read_all,"syntax error!"); YYERROR; }	}

identifiers: identifier { if (read_all) {
                            s = strdup(yytext);
                            if (!s) {
                              LOGMEM;
                              YYERROR;
                            }
                          }
                        }

%%

void yyerror(struct lys_module *module, struct unres_schema *unres, struct lys_array_size *size_arrays, int read_all, char *str, ...){
  va_list ap;
  va_start(ap,str);

  fprintf(stderr,"%d: error: ", yylineno);
  vfprintf(stderr,str,ap);
  //fprintf(stderr," Given %s.",yytext);
  fprintf(stderr,"\n");
  if (s) {
    free(s);
  }
}

void free_check(){

	struct Scheck *old=checked;
	checked=old->next;
	free(old);
}
